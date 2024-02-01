#include "limits.h"
#include "sputter.h"

// clang-format off
static unsigned char adpcm_silence[] __attribute__((aligned(16))) = {
/* 0x00 | 0x00 */ 0x0c, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0x10 | 0x08 */ 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0x20 | 0x10 */ 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0x30 | 0x18 */ 0x0c, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0x40 | 0x20 */ 0x0c, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};
// clang-format on

enum naxMark {
    NAX_KEY_ON,
    NAX_SAMPLE,
    NAX_LSA,
    NAX_SWITCH_PITCH,
    NAX_TIMEOUT,
    NAX_UPLOAD,
};

typedef struct {
    enum naxMark mark;
    u16 lsa;
    u16 nax;
    iop_sys_clock_t clock;
} naxSample;

static naxSample gSamples[512] = {};

static int voice = 1;
static s16 channel = 0;

#define NAXTEST_PITCH 0x100
// #define NAXTEST_PITCH 0x500
// #define NAXTEST_PITCH 0x3FFF
#define SPU_DST_ADDR (0x4000 << 1)

static void initRegs() {
    sceSdSetParam(SD_VOICE(channel, voice) | SD_VPARAM_VOLR, 0x3fff);
    sceSdSetParam(SD_VOICE(channel, voice) | SD_VPARAM_VOLL, 0x3fff);
    sceSdSetParam(SD_VOICE(channel, voice) | SD_VPARAM_PITCH, NAXTEST_PITCH);
    sceSdSetParam(SD_VOICE(channel, voice) | SD_VPARAM_ADSR1, SD_SET_ADSR1(SD_ADSR_AR_EXPi, 0, 0xf, 0xf));
    sceSdSetParam(SD_VOICE(channel, voice) | SD_VPARAM_ADSR2, SD_SET_ADSR2(SD_ADSR_SR_EXPd, 127, SD_ADSR_RR_EXPd, 0));

    sceSdSetParam(0 | SD_PARAM_MVOLL, 0x3fff);
    sceSdSetParam(0 | SD_PARAM_MVOLR, 0x3fff);
    sceSdSetParam(1 | SD_PARAM_AVOLL, 0x3fff);
    sceSdSetParam(1 | SD_PARAM_AVOLR, 0x3fff);
    sceSdSetParam(1 | SD_PARAM_MVOLL, 0x3fff);
    sceSdSetParam(1 | SD_PARAM_MVOLR, 0x3fff);
}

void naxTest() {
    initRegs();

    u8 *buffer = AllocSysMemory(ALLOC_FIRST, 0x2000, NULL);
    memset(buffer, 0, 0x2000);
    sceSdVoiceTrans(0, SD_TRANS_WRITE, buffer, (u32 *)SPU_DST_ADDR, 0x2000);
    sceSdVoiceTransStatus(channel, SPU_WAIT_FOR_TRANSFER);
    printf("Memory zereod\n");
    // memdump("host:mem-pre-test");

    // int trans = sceSdVoiceTrans(channel, SD_TRANS_WRITE | SD_TRANS_MODE_DMA, (u8 *)adpcm_silence, (u32 *)SPU_DST_ADDR, sizeof(adpcm_silence));
    // if (trans < 0) {
    //     printf("Bad transfer\n");
    //     return;
    // }

    // sceSdVoiceTransStatus(channel, SPU_WAIT_FOR_TRANSFER);
    // printf("Voice transfer complete\n");

    sceSdSetSwitch(channel | SD_SWITCH_KON, (1 << voice));
    sceSdSetAddr(SD_VOICE(channel, voice) | SD_VADDR_SSA, SPU_DST_ADDR);

    printf("Pitch %04x\n", NAXTEST_PITCH);
    // printf("Keying on!\n");

    u32 nax = (read16(SD_VA_NAX(channel, voice)) << 16) | read16(SD_VA_NAX(channel, voice) + 2);
    u32 lsa = (read16(SD_VA_LSAX(channel, voice)) << 16) | read16(SD_VA_LSAX(channel, voice) + 2);
    // TODO run test with first seen nax included

    u32 timeout = 0;
    for (int i = 0; i < 512; i++) {
        timeout = 0;
        while (1) {
            if (i == 16) {
                gSamples[i].mark = NAX_SWITCH_PITCH;
                GetSystemTime(&gSamples[i].clock);
                gSamples[i].nax = (read16(SD_VA_NAX(channel, voice)) << 16) | read16(SD_VA_NAX(channel, voice) + 2);
                gSamples[i].lsa = (read16(SD_VA_LSAX(channel, voice)) << 16) | read16(SD_VA_LSAX(channel, voice) + 2);
                sceSdSetParam(SD_VOICE(channel, voice) | SD_VPARAM_PITCH, 0);

                break;
            }

            if (i == 32) {
                gSamples[i].mark = NAX_KEY_ON;
                GetSystemTime(&gSamples[i].clock);
                gSamples[i].nax = (read16(SD_VA_NAX(channel, voice)) << 16) | read16(SD_VA_NAX(channel, voice) + 2);
                gSamples[i].lsa = (read16(SD_VA_LSAX(channel, voice)) << 16) | read16(SD_VA_LSAX(channel, voice) + 2);
                sceSdSetSwitch(channel | SD_SWITCH_KON, (1 << voice));

                break;
            }

            if (i == 64) {
                gSamples[i].mark = NAX_UPLOAD;
                GetSystemTime(&gSamples[i].clock);
                gSamples[i].nax = (read16(SD_VA_NAX(channel, voice)) << 16) | read16(SD_VA_NAX(channel, voice) + 2);
                gSamples[i].lsa = (read16(SD_VA_LSAX(channel, voice)) << 16) | read16(SD_VA_LSAX(channel, voice) + 2);

                int trans = sceSdVoiceTrans(channel, SD_TRANS_WRITE | SD_TRANS_MODE_DMA, (u8 *)adpcm_silence, (u32 *)SPU_DST_ADDR, sizeof(adpcm_silence));
                if (trans < 0) {
                    printf("Bad transfer\n");
                    return;
                }

                sceSdVoiceTransStatus(channel, SPU_WAIT_FOR_TRANSFER);
                sceSdSetParam(SD_VOICE(channel, voice) | SD_VPARAM_PITCH, NAXTEST_PITCH);

                break;
            }

            u32 newNax = (read16(SD_VA_NAX(channel, voice)) << 16) | read16(SD_VA_NAX(channel, voice) + 2);
            u32 newlsa = (read16(SD_VA_LSAX(channel, voice)) << 16) | read16(SD_VA_LSAX(channel, voice) + 2);

            if (newlsa != lsa) {
                GetSystemTime(&gSamples[i].clock);

                gSamples[i].mark = NAX_LSA;
                gSamples[i].nax = newNax;
                gSamples[i].lsa = (read16(SD_VA_LSAX(channel, voice)) << 16) | read16(SD_VA_LSAX(channel, voice) + 2);
                GetSystemTime(&gSamples[i].clock);

                lsa = newlsa;

                break;
            }

            if (newNax != nax) {
                GetSystemTime(&gSamples[i].clock);

                gSamples[i].mark = NAX_SAMPLE;
                gSamples[i].nax = newNax;
                gSamples[i].lsa = (read16(SD_VA_LSAX(channel, voice)) << 16) | read16(SD_VA_LSAX(channel, voice) + 2);
                GetSystemTime(&gSamples[i].clock);

                nax = newNax;

                break;
            }

            if (timeout >= 1000) {
                gSamples[i].mark = NAX_TIMEOUT;
                gSamples[i].nax = (read16(SD_VA_NAX(channel, voice)) << 16) | read16(SD_VA_NAX(channel, voice) + 2);
                gSamples[i].lsa = (read16(SD_VA_LSAX(channel, voice)) << 16) | read16(SD_VA_LSAX(channel, voice) + 2);
                GetSystemTime(&gSamples[i].clock);

                break;
            }

            timeout++;
            DelayThread(10);
        }
    }

    printf("Test concluded:\n");

    u32 psec, pusec;
    SysClock2USec(&gSamples[0].clock, &psec, &pusec);

    for (int i = 0; i < 512; i++) {
        u32 sec, usec;
        SysClock2USec(&gSamples[i].clock, &sec, &usec);

        s32 delta = usec - pusec;
        if (delta < 0)
            delta += 1000000;

        pusec = usec;
        psec = sec;

        switch (gSamples[i].mark) {
        case NAX_KEY_ON:
            printf("%d.%d: [+%d] Keyed on at [%04x] lsa[0%4x]\n", sec, usec, delta, gSamples[i].nax, gSamples[i].lsa);
            break;
        case NAX_SAMPLE:
            printf("%d.%d: [+%d] NAX switched to new sample [%04x] lsa[0%4x]\n", sec, usec, delta, gSamples[i].nax, gSamples[i].lsa);
            break;
        case NAX_TIMEOUT:
            printf("%d.%d: [+%d] TIMEOUT [%04x] lsa[0%4x]\n", sec, usec, delta, gSamples[i].nax, gSamples[i].lsa);
            break;
        case NAX_SWITCH_PITCH:
            printf("%d.%d: [+%d] PITCH SET TO 0 [%04x] lsa[0%4x]\n", sec, usec, delta, gSamples[i].nax, gSamples[i].lsa);
            break;
        case NAX_UPLOAD:
            printf("%d.%d: [+%d] Transferring voice data and setting pitch nax[%04x] lsa[0%4x]\n", sec, usec, delta, gSamples[i].nax, gSamples[i].lsa);
            break;
        case NAX_LSA:
            printf("%d.%d: [+%d] New LSA Address nax[%04x] lsa[0%4x]\n", sec, usec, delta, gSamples[i].nax, gSamples[i].lsa);
            break;
        }
    }

    // memdump("host:mem-post-test");
}
