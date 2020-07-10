#include "sputter.h"
#include "spu2regs.h"
#include <ioman.h>
#include <libsd.h>
#include <modload.h>
#include <stdio.h>
#include <sysmem.h>
#include <thbase.h>
#include <types.h>

IRX_ID("sputter", 1, 1);

// clang-format off
unsigned char adpcm_silence[] __attribute__((aligned(16))) = {
    /* 0x00 */ 0x0c, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0x10 */ 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0x20 */ 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0x30 */ 0x0c, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0x40 */ 0x0c, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};
// clang-format on

#define SPU_DST_ADDR (0x2800<<1)

//#define TESTFILE "host:sine.adp"
//const int size = 5888;

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

    //u32 fd = open(TESTFILE, O_RDONLY);
    //if (fd < 0) {
    //    printf("failed top open audio\n");
    //    return;
    //}

    //lseek(fd, 16, SEEK_SET);

    //u32 size = QueryMaxFreeMemSize();

    //u8* buffer = AllocSysMemory(ALLOC_FIRST, size, NULL);
    //if (buffer == NULL) {
    //    printf("alloc failed\n");
    //    return;
    //}

    //read(fd, buffer, size);

    int trans = sceSdVoiceTrans(channel, SD_TRANS_WRITE | SD_TRANS_MODE_DMA, (u8*)adpcm_silence, (u32*)SPU_DST_ADDR, sizeof(adpcm_silence));
    if (trans < 0) {
        printf("Bad transfer\n");
        return;
    }

    sceSdVoiceTransStatus(channel, SPU_WAIT_FOR_TRANSFER);
    printf("Voice transfer complete\n");

    sceSdSetAddr(SD_VOICE(channel, voice) | SD_VADDR_SSA, SPU_DST_ADDR);
    sceSdSetSwitch(channel | SD_SWITCH_KON, (1 << voice));

    iop_sys_clock_t clock = { 0 };
    GetSystemTime(&clock);

    u32 sec_prev, usec_prev;
    SysClock2USec(&clock, &sec_prev, &usec_prev);

    u32 nax = 0;
    while (1) {
        u32 newNax = sceSdGetAddr(SD_VOICE(channel, voice) | SD_VADDR_NAX);
        if (newNax != nax) {
            GetSystemTime(&clock);
            u32 sec_new, usec_new;
            SysClock2USec(&clock, &sec_new, &usec_new);

            printf("New NAX 0x%x, %uus since last change\n", newNax >> 1, (usec_new - usec_prev));

            usec_prev = usec_new;
            sec_prev = sec_new;
            nax = newNax;
            DelayThread(1000);
        }
    }
}

s32 _start()
{
    if (sceSdInit(0) < 0) {
        while (1) {
            printf("SD INIT FAILED");
        }
        return -1;
    }

    runTest();

    return 0;
}
