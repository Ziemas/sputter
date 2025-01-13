#include <kernel.h>
#include <sbv_patches.h>
#include <loadfile.h>
#include <sifrpc.h>

extern unsigned char sputter[];
extern unsigned int size_sputter;

extern unsigned char libsd[];
extern unsigned int size_libsd;

int main(int argc, char *argv[]) {
    int res;

    sbv_patch_enable_lmb();

    SifInitRpc(0);

    SifExecModuleBuffer(libsd, size_libsd, 0, NULL, &res);
    SifExecModuleBuffer(sputter, size_sputter, 0, NULL, &res);

    while (1) {
    }

    return 0;
}
