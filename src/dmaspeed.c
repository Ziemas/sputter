#include "sputter.h"

struct sample {
    u32 start, end;
    u32 size;
    int direction, channel;
};

struct sample samples[10 * 2 * 2];

static int dst = 0x10000;

static int storage[0x8000];

static void do_transfer(struct sample *s, int size, int channel, int direction) {
    u32 start, end;

    // read sysclock timer directly
    start = _lw(0xBF801480);
    sceSdVoiceTrans(channel, direction | SD_TRANS_MODE_DMA, (u8 *)storage, (u32 *)dst, size);
    sceSdVoiceTransStatus(channel, SPU_WAIT_FOR_TRANSFER);
    end = _lw(0xBF801480);

    s->size = size;
    s->start = start;
    s->end = end;
    s->channel = channel;
    s->direction = direction;
}

void dmaspeed() {
    struct sample *s;

    memset(samples, 0, sizeof(samples));

    s = samples;

    DelayThread(40000);

    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            int size = 64;

            for (int k = 0; k < 10; k++) {
                do_transfer(s, size, i, j);
                s++;

                size *= 2;
                DelayThread(40000);
            }
        }
    }

    for (int i = 0; i < (10 * 2 * 2); i++) {
        u64 time = (int)(samples[i].end - samples[i].start);
        char *dir = samples[i].direction ? "read" : "write";
        int dma = samples[i].channel ? 7 : 4;
        printf("DMA[%d] %s of %d bytes took %llu cycles\n", dma, dir, samples[i].size, time);
    }
}

static u8 autodmabuf[4096];

/*
void deckard_hack() {
    memset(autodmabuf, 0, sizeof(autodmabuf));
    memset(samples, 0, sizeof(samples));

    for (int i = 0; i < 10; i++) {
        GetSystemTime(&samples[i].start);

        sceSdSetCoreAttr(SD_CORE_NOISE_CLK, 20);

        GetSystemTime(&samples[i].end);
    }

    for (int i = 0; i < 10; i++) {
        u64 start = as_u64(samples[i].start.hi, samples[i].start.lo);
        u64 end = as_u64(samples[i].end.hi, samples[i].end.lo);

        u64 time = end - start;
        printf("DMA transfer [%d] of %d bytes to %x took %llu cycles\n", i, samples[i].size, dst, time);
        printf("%u cycles\n", (u32)time);
    }

    memset(samples, 0, sizeof(samples));
    sceSdVoiceTrans(0, SD_TRANS_WRITE | SD_TRANS_MODE_DMA, (u8 *)0, (u32 *)dst, 0x20000);

    for (int i = 0; i < 10; i++) {
        GetSystemTime(&samples[i].start);

        sceSdSetCoreAttr(SD_CORE_NOISE_CLK, 20);

        GetSystemTime(&samples[i].end);
    }

    for (int i = 0; i < 10; i++) {
        u64 start = as_u64(samples[i].start.hi, samples[i].start.lo);
        u64 end = as_u64(samples[i].end.hi, samples[i].end.lo);

        u64 time = end - start;
        printf("DMA transfer [%d] of %d bytes to %x took %llu cycles\n", i, samples[i].size, dst, time);
        printf("%u cycles\n", (u32)time);
    }
}
*/
