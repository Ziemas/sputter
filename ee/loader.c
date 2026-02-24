#include <delaythread.h>
#include <iopcontrol.h>
#include <kernel.h>
#include <loadfile.h>
#include <sifrpc.h>
#include <string.h>

int main(int argc, char *argv[]) {
    SifInitRpc(0);

    SifLoadModule("host0:libsd.irx", 0, NULL);
    SifLoadModule("host0:sputter.irx", 0, NULL);
    // SifLoadModule("host0:sputter.irx", 0, NULL);
    // SifLoadModule("host:ix.irx", 0, NULL);

    // printf("%p 0x%x\n", SPUTTER_irx, sizeof(SPUTTER_irx));
    // printf("---------\n");
    // sceSifWriteBackDCache(SPUTTER_irx, sizeof(SPUTTER_irx));

    while (1) {
        DelayThread(10000);
    }

    return 0;
}
