#include "sputter.h"

struct sample {
    iop_sys_clock_t start, end;
    u32 size;
};

struct sample samples[10];

static inline u64 as_u64(u32 a1, u32 a2) {
    return (u64)a1 << 32 | a2;
}

static int dst = 0x10000;

void dmaspeed() {
    memset(samples, 0, sizeof(samples));
    int size = 64;

    for (int i = 0; i < 10; i++) {
        GetSystemTime(&samples[i].start);

        sceSdVoiceTrans(0, SD_TRANS_WRITE | SD_TRANS_MODE_DMA, (u8 *)0, (u32 *)dst, size);
        sceSdVoiceTransStatus(0, SPU_WAIT_FOR_TRANSFER);

        GetSystemTime(&samples[i].end);

        samples[i].size = size;

        size *= 2;
    }

    for (int i = 0; i < 10; i++) {
        u64 start = as_u64(samples[i].start.hi, samples[i].start.lo);
        u64 end = as_u64(samples[i].end.hi, samples[i].end.lo);

        u64 time = end - start;
        printf("DMA transfer [%d] of %d bytes to %x took %llu cycles\n", i, samples[i].size, dst, time);
    }
}

static u8 autodmabuf[4096];

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
