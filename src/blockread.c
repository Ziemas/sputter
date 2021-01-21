#include "sputter.h"

//static u8 *captureBuf;

static int channel = 0;

typedef struct {
    u8 *buffers[2];
    u8 scratchpad[1024];
    u8 spubuf;
    u32 pos;
    u8 activeBuf;
    u32 sema;
} transData;

static transData data = {};
static u32 outFd;
static u32 outFd2;

u8 *scratchPad;

int transHandler(u32 channel, void *data, void **addr, int *size) {
    transData *d = (transData *)data;
    *size = 1024;
    *addr = d->scratchpad;

    memcpy(d->buffers[d->activeBuf] + (d->pos * 512), d->scratchpad + (d->spubuf * 512), 512);

    d->spubuf = 1 - d->spubuf;
    d->pos++;

    if (d->pos > 10) {
        d->activeBuf = 1 - d->activeBuf;
        //*addr = d->buffers[d->activeBuf];
        d->pos = 0;

        iSignalSema(d->sema);
    }

    return 0;
}

void blockRead() {
    playSound();

    scratchPad = AllocSysMemory(ALLOC_FIRST, 10 * 1024, NULL);
    if (scratchPad == NULL) {
        printf("blockread: Alloc failed\n");
        return;
    }

    for (int i = 0; i < 2; i++) {
        if (data.buffers[i] == NULL) {
            data.buffers[i] = AllocSysMemory(ALLOC_FIRST, 10 * 1024, NULL);
            if (data.buffers[i] == NULL) {
                printf("blockread: Alloc failed\n");
                return;
            }
        }
    }

#define SD_BLOCK_C0_VOICE1 (0x0 << 8)
#define SD_BLOCK_C0_VOICE3 (0x1 << 8)
#define SD_BLOCK_C1_SINL (0x2 << 8)
#define SD_BLOCK_C1_SINR (0x3 << 8)
#define SD_BLOCK_C1_VOICE1 (0x4 << 8)
#define SD_BLOCK_C1_VOICE3 (0x5 << 8)
#define SD_BLOCK_C0_MEMOUTL (0x6 << 8)
#define SD_BLOCK_C0_MEMOUTR (0x7 << 8)
#define SD_BLOCK_C0_MEMOUTEL (0x8 << 8)
#define SD_BLOCK_C0_MEMOUTER (0x9 << 8)
#define SD_BLOCK_C1_MEMOUTL (0xa << 8)
#define SD_BLOCK_C1_MEMOUTR (0xb << 8)
#define SD_BLOCK_C1_MEMOUTEL (0xc << 8)
#define SD_BLOCK_C1_MEMOUTER (0xd << 8)

    outFd = open("host:outstream", O_CREAT | O_RDWR);
    outFd2 = open("host:outstream2", O_CREAT | O_RDWR);
    if (!outFd) {
        printf("Failed to open output file\n");
        return;
    }

#define SD_BLOCK_COUNT(x) ((x) << 12)

    printf("-----------\n");
    printf("adma_count %02x\n", *U16_REGISTER(0x1AE + (channel * 1024)));
    printf("irqa %04x\n", *SD_CORE_IRQA(channel));
    printf("-----------\n");
    printf("starting\n");
    printf("-----------\n");

#define SD_BLOCK_HANDLER (1 << 7)
    sceSdBlockTrans(channel, SD_TRANS_READ | SD_BLOCK_HANDLER | SD_BLOCK_C0_VOICE1 | SD_BLOCK_COUNT(1),
                    data.scratchpad, 1024, &transHandler, &data);

    printf("-----------\n");
    printf("adma_count %02x\n", *U16_REGISTER(0x1AE + (channel * 1024)));
    printf("irqa %04x\n", *SD_CORE_IRQA(0));
    printf("irqa %04x\n", *SD_CORE_IRQA(1));
    printf("-----------\n");

    printf("BlockTrans called\n");

    while (1) {
        WaitSema(data.sema);
        //write(outFd, data.buffers[1 - data.activeBuf], 10 * 1024);
        //printf("wrote %d bytes from buffer %d\n", 10 * 1024, 1 - data.activeBuf);
        //printf("irqa %04x\n", *SD_CORE_IRQA(0));
        //printf("irqa %04x\n", *SD_CORE_IRQA(1));

        //printf("tsa %04x\n", *SD_A_TSA_LO(0) | (*SD_A_TSA_HI(0) << 16));
        //printf("spdif %04x\n", *SD_C_IRQINFO);

        //printf("tsa %04x\n", *SD_A_TSA_LO(0) | (*SD_A_TSA_HI(0) << 16));
        //printf("tsa %04x\n", *SD_A_TSA_LO(1) | (*SD_A_TSA_HI(1) << 16));

        write(outFd, data.buffers[1 - data.activeBuf], 10 * 512);

        //for (int i = 0; i < 2; i++) {
        //    u8 *dst = scratchPad;
        //    u8 *src = data.buffers[1 - data.activeBuf] + (i * 512);

        //    for (int j = 0; j < 5; j++) {
        //        memcpy(dst, src, 512);

        //        src += 1024;
        //        dst += 512;
        //    }

        //    if (i == 0)
        //        write(outFd, scratchPad, 5 * 512);
        //    else
        //        write(outFd2, scratchPad, 5 * 512);
        //}
    }
}
