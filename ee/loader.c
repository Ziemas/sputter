#include <kernel.h>
#include <sifrpc.h>
#include <loadfile.h>

extern unsigned char SPUTTER_irx[];
extern unsigned int size_SPUTTER_irx;

int main(int argc, char *argv[]) {
    SifInitRpc(0);

    SifLoadModule("host0:sputter.irx", 0, NULL);

    while (1) {
    }

    return 0;
}
