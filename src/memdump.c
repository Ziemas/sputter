#include "sputter.h"

void memdump(char* filename) {
    int fd;
    u8 *buffer = AllocSysMemory(ALLOC_FIRST, 0x2000, NULL);
    memset(buffer, 0, 0x2000);

    fd = open(filename, O_CREAT | O_RDWR);
    if (!fd) {
        printf("Failed to open output file\n");
        return;
    }

    s32 left = 0x200000;
    u32 pos = 0;
    while (left > 0) {
        sceSdVoiceTrans(0, SD_TRANS_READ, buffer, (u32*)pos, 0x2000);
        sceSdVoiceTransStatus(0, SPU_WAIT_FOR_TRANSFER);

        write(fd, buffer, 0x2000);
        memset(buffer, 0, 0x2000);

        printf("%08x left, at %08x\n", left, pos);

        pos += 0x2000;
        left -= 0x2000;
    }

    printf("done\n");
}
