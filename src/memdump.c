#include "sputter.h"

u8 buffer[0x2000];

void memdump(char *filename) {
    int fd;
    memset(buffer, 0, 0x2000);

    fd = open(filename, O_CREAT | O_RDWR | O_TRUNC);
    if (!fd) {
        printf("Failed to open output file\n");
        return;
    }

    printf("dumping memory");

    s32 left = 0x200000;
    u32 pos = 0;
    while (left > 0) {
        sceSdVoiceTrans(0, SD_TRANS_READ, buffer, (u32 *)pos, 0x2000);
        sceSdVoiceTransStatus(0, SPU_WAIT_FOR_TRANSFER);

        write(fd, buffer, 0x2000);
        memset(buffer, 0, 0x2000);

        //printf("%08x left, at %08x\n", left, pos);
        printf(".");

        pos += 0x2000;
        left -= 0x2000;
    }

    close(fd);
    printf(" done\n");
}

void memfill(u8 value) {
    memset(buffer, value, 0x2000);

    printf("filling memory");

    s32 left = 0x200000;
    u32 pos = 0;
    while (left > 0) {
        sceSdVoiceTrans(0, SD_TRANS_WRITE, buffer, (u32 *)pos, 0x2000);
        sceSdVoiceTransStatus(0, SPU_WAIT_FOR_TRANSFER);

        //printf("%08x left, at %08x\n", left, pos);
        printf(".");

        pos += 0x2000;
        left -= 0x2000;
    }

    printf(" done\n");
}

// ????
void memfill16(u16 value) {
    for (int i = 0; i < 0x2000; i += 2) {
        *(u16*)(&buffer[i]) = value;
    }

    s32 left = 0x200000;
    u32 pos = 0;
    while (left > 0) {
        sceSdVoiceTrans(0, SD_TRANS_WRITE, buffer, (u32 *)pos, 0x2000);
        sceSdVoiceTransStatus(0, SPU_WAIT_FOR_TRANSFER);

        //printf("%08x left, at %08x\n", left, pos);
        printf(".");

        pos += 0x2000;
        left -= 0x2000;
    }
}
