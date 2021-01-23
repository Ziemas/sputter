#include "sputter.h"

IRX_ID("sputter", 1, 1);

int thid = 0;

void sputterThread(void *param) {
    printf("sputter alive\n");
    //naxTest();
    //noiseTest();
    playSound();
    //envx();
    //blockRead();
    while(1)
        ;
}

s32 _start() {
    if (sceSdInit(0) < 0) {
        printf("SD INIT FAILED");
        return -1;
    }

    iop_thread_t thread = {};
    thread.attr = TH_C;
    thread.thread = &sputterThread;
    thread.priority = 40;
    thread.stacksize = 0x800;

    thid = CreateThread(&thread);
    if (thid > 0) {
        StartThread(thid, NULL);
    }

    return 0;
}
