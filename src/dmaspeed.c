#include "sputter.h"

static int dst = 0x10000;
static int storage[0x8000];

static void SetDmaWrite(s32 chan) {
    volatile u32 *reg = (volatile u32 *)U32_REGISTER(0x1014 + (chan << 10));
    *reg = (*reg & 0xF0FFFFFF);
}

static void SetDmaRead(s32 chan) {
    volatile u32 *reg = (volatile u32 *)U32_REGISTER(0x1014 + (chan << 10));
    *reg = (*reg & 0xF0FFFFFF) | 0x2000000;
}

static s32 SetupDma(s16 chan, u16 mode, u8 *iop_addr, u32 *spu_addr, u32 size) {
    u32 direction;

    if (read16(SD_CORE_ATTR(chan)) & SD_DMA_IN_PROCESS)
        return -1;

    if (read16(SD_DMA_CHCR(chan)) & SD_DMA_START)
        return -2;

    write16(SD_A_TSA_HI(chan), (u32)spu_addr >> 17);
    write16(SD_A_TSA_LO(chan), (u32)spu_addr >> 1);

    if (mode == SD_TRANS_WRITE) {
        SetDmaWrite(chan);

        direction = SD_DMA_DIR_IOP2SPU;
        mask16(SD_CORE_ATTR(chan), SD_CORE_DMA, SD_DMA_WRITE);
    } else if (mode == SD_TRANS_READ) {
        SetDmaRead(chan);

        direction = SD_DMA_DIR_SPU2IOP;
        mask16(SD_CORE_ATTR(chan), SD_CORE_DMA, SD_DMA_READ);
    } else {
        return -3;
    }

    write32(SD_DMA_ADDR(chan), (u32)iop_addr);
    write32(SD_DMA_MSIZE(chan), (((size + 63) / 64) << 16) | 0x10);
    write32(SD_DMA_CHCR(chan), SD_DMA_CS | direction);

    return size;
}

static void StartDma(int chan) {
    set32(SD_DMA_CHCR(chan), SD_DMA_START);
}

static void WaitDma(int chan) {
    // Wait for dma completion
    while (read32(SD_DMA_CHCR(chan)) & SD_DMA_START) {
    }

    // Wait for SPU FIFO
    while ((read16(SD_C_STATX(chan)) & 0x80) == 0) {
    }
}

static void EndDma(int chan) {
    // Clear DMA mode and wait for it to take
    clear16(SD_CORE_ATTR(chan), SD_CORE_DMA);
    while ((read16(SD_CORE_ATTR(chan)) & 0x30) != 0) {
    }
}

static u32 time_transfer(int size, int channel, int direction) {
    u32 start, end;
    int res;

    res = SetupDma(channel, direction, (u8 *)storage, (u32 *)dst, size);
    if (res < 0) {
        printf("setup failed %d\n", res);
    }

    // read sysclock timer directly
    start = _lw(0xBF801480);

    StartDma(channel);
    WaitDma(channel);
    EndDma(channel);

    end = _lw(0xBF801480);

    return (int)(end - start);
}

static void test_transfer(int size, int channel, int direction) {
    u32 res, avg = 0, min = -1, max = 0;
    char *dir = direction ? "Read" : "Write";
    int dma = channel ? 7 : 4;

    for (int i = 0; i < 200; i++) {
        // Delay to make sure we don't trigger the deckard hacks
        DelayThread(40000);

        res = time_transfer(size, channel, direction);
        min = min(res, min);
        max = max(res, max);
        avg = ((avg * i) + res) / (i + 1);
    }

    printf("DMA[%d], %1s,  %5d,  %6d, %6d, %6d\n", dma, dir, size, avg, min, max);
}

void dmaspeed() {
    printf("Channel, Access, Size, Avg Cycles, Min Cycles, Max Cycles\n");

    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            int size = 64;

            for (int k = 0; k < 10; k++) {
                test_transfer(size, i, j);

                size *= 2;
            }
        }
    }
}

/*
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
*/
