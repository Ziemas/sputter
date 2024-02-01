#include "spu2regs.h"
#include "sputter.h"

static u16 data[64];

static u16 data_readback[512];

static void do_manual_transfer(u32 dest, u8 *data, u32 size) {
    write16(SD_A_TSA_HI(0), dest >> 16);
    write16(SD_A_TSA_LO(0), dest & 0xffff);

    u16 *ptr = (u16 *)data;

    while (size > 0) {
        // Fill fifo
        for (int i = 0; i < 32; i++) {
            write16(SD_A_FIFO(0), *ptr);
            ptr++;
        }

        // Start transfer?
        write16(SD_CORE_ATTR(0), (read16(SD_CORE_ATTR(0)) & 0xffcf) | 0x10);

        // Wait for completion
        while ((read16(SD_C_STATX(0)) & 0x400) != 0)
            ;

        size -= 64;
    }

    write16(SD_CORE_ATTR(0), read16(SD_CORE_ATTR(0)) & 0xffcf);
}

void tsatest() {
    write16(U16_REGISTER(0x1ae), 2);

    memset(data, 0xffff, sizeof(data));
    printf("starting transfer to 0x3000\n");

    do_manual_transfer(0x3000, (u8 *)&data, sizeof(data));
    // sceSdVoiceTrans(0, SD_TRANS_WRITE | SD_TRANS_MODE_IO, (u8 *)&data, (u32 *)0x3000, sizeof(data));

    memset(data, 0xf0f0, sizeof(data));
    do_manual_transfer(0x4000, (u8 *)&data, sizeof(data));

    write16(SD_A_TSA_HI(0), 0x3000 >> 16);
    write16(SD_A_TSA_LO(0), 0x3000 & 0xffff);

    // Fill fifo
    for (int i = 0; i < 38; i++) {
        write16(SD_A_FIFO(0), i);
    }

    // Start transfer?
    write16(SD_CORE_ATTR(0), (read16(SD_CORE_ATTR(0)) & 0xffcf) | 0x10);

    // Wait for completion
    while ((read16(SD_C_STATX(0)) & 0x400) != 0)
        ;

    write16(SD_CORE_ATTR(0), read16(SD_CORE_ATTR(0)) & 0xffcf);

    sceSdVoiceTrans(0, SD_TRANS_READ, (u8 *)&data_readback, (u32 *)(0x3000 << 1), sizeof(data_readback));
    sceSdVoiceTransStatus(0, SPU_WAIT_FOR_TRANSFER);

    for (int i = 0; i < 512;) {
        printf("0x%04x ", data_readback[i]);

        i++;

        if ((i % 8) == 0)
            printf("\n");
    }
}
