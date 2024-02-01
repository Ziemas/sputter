#include "sputter.h"

IRX_ID("sputter", 1, 1);

int sputterThid = 0;

void sputterThread(void *param) {
    printf("sputter alive\n");

    SleepThread();
}

s32 _start(int argc, char *argv[]) {
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
