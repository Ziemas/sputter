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

typedef struct {
    u16 envx;
    u32 cycle;
} envxSample;

typedef struct {
    envxSample samples[512];
    u32 sampleIdx;
    u32 cycleAcc;
    int timer;
    volatile int flag;
} sampleData;

static sampleData data = {};

static int voice = 1;
static int channel = 0;

//#define NAXTEST_PITCH 0x0010
//#define NAXTEST_PITCH 0x500
#define NAXTEST_PITCH 0x3FFF
#define SPU_DST_ADDR (0x2800 << 1)

static void initRegs() {
    sceSdSetParam(SD_VOICE(channel, voice) | SD_VPARAM_VOLR, 0x3fff);
    sceSdSetParam(SD_VOICE(channel, voice) | SD_VPARAM_VOLL, 0x3fff);
    sceSdSetParam(SD_VOICE(channel, voice) | SD_VPARAM_PITCH, NAXTEST_PITCH);
    sceSdSetParam(SD_VOICE(channel, voice) | SD_VPARAM_ADSR1, SD_SET_ADSR1(SD_ADSR_AR_EXPi, 0, 0xf, 0xa));
    sceSdSetParam(SD_VOICE(channel, voice) | SD_VPARAM_ADSR2, SD_SET_ADSR2(SD_ADSR_SR_EXPd, 127, SD_ADSR_RR_EXPd, 0));

    sceSdSetParam(0 | SD_PARAM_MVOLL, 0x3fff);
    sceSdSetParam(0 | SD_PARAM_MVOLR, 0x3fff);
    sceSdSetParam(1 | SD_PARAM_AVOLL, 0x3fff);
    sceSdSetParam(1 | SD_PARAM_AVOLR, 0x3fff);
    sceSdSetParam(1 | SD_PARAM_MVOLL, 0x3fff);
    sceSdSetParam(1 | SD_PARAM_MVOLR, 0x3fff);
}

unsigned int timerCb(void *common) {
    sampleData *d = (sampleData *)common;

    if (d->sampleIdx >= 512) {
        d->flag = 1;
        return 0;
    }

    d->samples[d->sampleIdx].envx = *SD_VP_ENVX(channel, voice);
    d->samples[d->sampleIdx].cycle = GetTimerCounter(d->timer);
    d->sampleIdx++;

    iop_sys_clock_t clock = {0};
    USec2SysClock(500000, &clock);

    return clock.lo;
}

void envx() {
    initRegs();

    int trans = sceSdVoiceTrans(channel, SD_TRANS_WRITE | SD_TRANS_MODE_DMA, (u8 *)adpcm_silence, (u32 *)SPU_DST_ADDR, sizeof(adpcm_silence));
    if (trans < 0) {
        printf("Bad transfer\n");
        return;
    }

    sceSdVoiceTransStatus(channel, SPU_WAIT_FOR_TRANSFER);
    printf("Voice transfer complete\n");

    sceSdSetAddr(SD_VOICE(channel, voice) | SD_VADDR_SSA, SPU_DST_ADDR);

    printf("Keying on!\n");
    sceSdSetSwitch(channel | SD_SWITCH_KON, (1 << voice));

    #define TC_SYSCLOCK 1
    data.timer =  AllocHardTimer(TC_SYSCLOCK, 32, 1);

    iop_sys_clock_t clock = {0};
    USec2SysClock(500000, &clock);
    SetAlarm(&clock, &timerCb, &data);

    while (data.flag == 0)
        ;

    printf("Test concluded:\n");

    for (int i = 0; i < 512; i++) {
        printf("Saw envx %04x at cycle %ld\n", data.samples[i].envx, data.samples[i].cycle);
    }

    while (1)
        ;
}
