#include "sputter.h"

static int channel = 0;

typedef struct {
    u8 *buffers[2];
    u8 scratchpad[2 * 1024];
    u8 spubuf;
    u32 pos;
    u8 activeBuf;
    u32 sema;
    u32 clock;
} transData;

static transData data = {};
static u32 outFd;

int transHandler(u32 channel, void *data, void **addr, int *size) {
    transData *d = (transData *)data;
    *addr = d->scratchpad;
    *size = 1024;

    d->spubuf = 1 - ((*SD_C_IRQINFO >> 4) & 1);

    memcpy(d->buffers[d->activeBuf] + (d->pos * 512), d->scratchpad + (d->spubuf * 512), 512);

    d->pos++;
    if (d->pos >= 20) {

        d->activeBuf = 1 - d->activeBuf;
        d->pos = 0;
        iSignalSema(d->sema);
    }

    return 0;
}

void blockRead(void *param) {
    printf("sputter host streaming starting\n");

    for (int i = 0; i < 2; i++) {
        if (data.buffers[i] == NULL) {
            data.buffers[i] = AllocSysMemory(ALLOC_FIRST, 10 * 1024, NULL);
            if (data.buffers[i] == NULL) {
                printf("blockread: Alloc failed\n");
                return;
            }
        }

        memset(data.buffers[i], 0, 10 * 1024);
    }

    outFd = open("host:outstream", O_CREAT | O_RDWR);
    if (!outFd) {
        printf("Failed to open output file\n");
        return;
    }

    iop_sema_t sema = {};
    sema.attr = SA_THFIFO;
    sema.initial = 0;
    sema.max = 2;

    data.sema = CreateSema(&sema);

    sceSdBlockTrans(channel, SD_TRANS_READ | SD_BLOCK_HANDLER | SD_BLOCK_C1_SINL | SD_BLOCK_COUNT(1),
                    data.scratchpad, 1 * 1024, &transHandler, &data);

    printf("-----------\n");
    printf("adma_count %04x\n", *U16_REGISTER(0x1AE + (channel * 1024)));
    printf("tsa %08x\n", *SD_A_TSA_LO(0) | (*SD_A_TSA_HI(0) << 16));
    printf("-----------\n");

    while (1) {
        int err = WaitSema(data.sema);
        if (err) {
            printf("sema error %d", err);
            sceSdStopTrans(0);
            return;
        }

        write(outFd, data.buffers[1 - data.activeBuf], 20 * 512);
    }
}
