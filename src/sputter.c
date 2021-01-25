#include "sputter.h"

IRX_ID("sputter", 1, 1);

//#define RECORD

int sputterThid = 0;
int recThid = 0;

void sputterThread(void *param) {
    printf("sputter alive\n");

#ifdef RECORD
    iop_thread_t streamThread = {};
    streamThread.attr = TH_C;
    streamThread.thread = &blockRead;
    streamThread.priority = 30;
    streamThread.stacksize = 0x800;

    recThid = CreateThread(&streamThread);
    if (recThid > 0) {
        StartThread(recThid, NULL);
    }
#endif

    //naxTest();
    //noiseTest();
    //playSound();
    //envx();
    //blockRead();
    bufdetect();


    SleepThread();
}

s32 _start() {
    if (sceSdInit(0) < 0) {
        printf("SD INIT FAILED");
        return -1;
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

    return 0;
}
