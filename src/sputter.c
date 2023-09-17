#include "sputter.h"

IRX_ID("sputter", 1, 1);

// #define RECORD

int sputterThid = 0;
int recThid = 0;

void sputterThread(void *param) {
    printf("sputter alive\n");

#ifdef RECORD
    iop_thread_t streamThread = {};
    streamThread.attr = TH_C;
    streamThread.thread = &blockRead;
    streamThread.priority = 20;
    streamThread.stacksize = 0x800;

    recThid = CreateThread(&streamThread);
    if (recThid > 0) {
        StartThread(recThid, NULL);
    }
#endif

    // iop_sys_clock_t clock;
    // USec2SysClock(1000000, &clock);
    // printf("%d %d\n", clock.hi, clock.lo / 0xf0);
    // clock.lo /= 0xf0;
    // u32 sec, usec;
    // SysClock2USec(&clock, &sec, &usec);
    // printf("%d\n", USEC_SECOND / usec);

    // memdump();
    // naxTest();
    // noiseTest();
    // playSound();
    // envx();
    // blockRead();
    // bufdetect();
    // dmatest();
    reverbtest();
    // reverbregtest();

    // SleepThread();
}

s32 _start(int argc, char *argv[]) {
    if (sceSdInit(0) < 0) {
        printf("SD INIT FAILED");
        return MODULE_NO_RESIDENT_END;
    }

    iop_thread_t thread = {};
    thread.attr = TH_C;
    thread.thread = &sputterThread;
    thread.priority = 50;
    thread.stacksize = 0x800;

    sputterThid = CreateThread(&thread);
    if (sputterThid > 0) {
        StartThread(sputterThid, NULL);
    }

    return MODULE_RESIDENT_END;
}

u32 loadSound(const char *filename, u32 channel, u32 *spudst) {
    io_stat_t stat;
    getstat(filename, &stat);
    printf("size %d\n", stat.size);

    u8 *buffer = AllocSysMemory(ALLOC_FIRST, stat.size, NULL);
    if (buffer == NULL) {
        printf("Alloc failed\n");
        return 1;
    }

    u32 fd = open(filename, O_RDONLY);
    if (fd < 0) {
        printf("Opening testfile failed\n");
        return 1;
    }

    // skip header data
    lseek(fd, 16, SEEK_SET);
    read(fd, buffer, stat.size);
    close(fd);

    int trans = sceSdVoiceTrans(channel, SD_TRANS_WRITE | SD_TRANS_MODE_DMA, buffer, spudst, stat.size);
    if (trans < 0) {
        printf("Bad transfer\n");
        return 1;
    }

    int err = sceSdVoiceTransStatus(channel, SPU_WAIT_FOR_TRANSFER);
    if (err < 0) {
        printf("failed to wait for transfer %d", err);
    }

    FreeSysMemory(buffer);

    printf("Loaded sound %s size %lu to %08lx\n", filename, stat.size, ((u32)spudst) >> 1);

    return stat.size;
}
