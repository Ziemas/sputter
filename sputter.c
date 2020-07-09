#include "sputter.h"
#include <ioman.h>
#include <libsd.h>
#include <modload.h>
#include <stdio.h>
#include <sysmem.h>
#include <types.h>

IRX_ID("sputter", 1, 1);

#define TESTFILE "host:sine.adp"
const int size = 5888;

//#define TESTFILE "host:never.adp"
//const int size = 461680;

const int channel = 0;
const int voice = 0;

void initRegs()
{
    sceSdSetParam(SD_VOICE(channel, voice) | SD_VPARAM_VOLR, 0x3fff);
    sceSdSetParam(SD_VOICE(channel, voice) | SD_VPARAM_VOLL, 0x3fff);
    sceSdSetParam(SD_VOICE(channel, voice) | SD_VPARAM_PITCH, 0x0010);
    sceSdSetParam(SD_VOICE(channel, voice) | SD_VPARAM_ADSR1, SD_SET_ADSR1(SD_ADSR_AR_EXPi, 0, 0xf, 0xf));
    sceSdSetParam(SD_VOICE(channel, voice) | SD_VPARAM_ADSR2, SD_SET_ADSR2(SD_ADSR_SR_EXPd, 127, SD_ADSR_RR_EXPd, 0));

    sceSdSetParam(0 | SD_PARAM_MVOLL, 0x3fff);
    sceSdSetParam(0 | SD_PARAM_MVOLR, 0x3fff);
    sceSdSetParam(1 | SD_PARAM_AVOLL, 0x3fff);
    sceSdSetParam(1 | SD_PARAM_AVOLR, 0x3fff);
    sceSdSetParam(1 | SD_PARAM_MVOLL, 0x3fff);
    sceSdSetParam(1 | SD_PARAM_MVOLR, 0x3fff);
}

void runTest()
{
    initRegs();

    u32 fd = open(TESTFILE, O_RDONLY);
    if (fd < 0) {
        printf("failed top open audio\n");
        return;
    }

    lseek(fd, 16, SEEK_SET);

    u32 size = QueryMaxFreeMemSize();
    printf("available space: %d\n", size);

    u8* buffer = AllocSysMemory(ALLOC_FIRST, size, NULL);
    if (buffer == NULL) {
        printf("alloc failed\n");
    }

    printf("1\n");
    u32 spuaddr = 0x5000;
    read(fd, buffer, size);

    printf("2\n");
    int trans = sceSdVoiceTrans(channel, SD_TRANS_WRITE | SD_TRANS_MODE_DMA, buffer, (u32*)spuaddr, size);
    if (trans < 0) {
        printf("Bad transfer\n");
    }
    printf("3\n");
    sceSdVoiceTransStatus(channel, voice);
    printf("4\n");
    sceSdSetAddr(SD_VOICE(channel, voice) | SD_VADDR_SSA, 0x5000);

    sceSdSetSwitch(channel | SD_SWITCH_KON, (1 << voice));

    u32 nax = 0;
    while (1) {
        u32 newNax = sceSdGetAddr(SD_VOICE(channel, voice) | SD_VADDR_NAX);
        if (newNax != nax) {
            printf("Seen NAX %x\n", newNax >> 1);
            nax = newNax;
        }
    }
}

s32 _start()
{
    printf("hello world\n");

    int s = sceSdInit(0);
    if (s < 0) {
        while (1) {
            printf("SD INIT FAILED");
        }

        return -1;
    }

    int wait = 3000000;
    while (wait) {
        wait--;
    }

    runTest();

    return 0;
}
