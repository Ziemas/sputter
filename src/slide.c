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

#define SAMPLES_MAX 4096

typedef struct {
    s16 envx;
    u32 cycle;
} envxSample;

typedef struct {
    envxSample samples[SAMPLES_MAX];
    u32 sampleIdx;
    u32 startCycle;
    int timer;
    volatile int flag;
} sampleData;

static sampleData data = {};

static int voice = 1;
static int channel = 0;

// #define NAXTEST_PITCH 0x0010
// #define NAXTEST_PITCH 0x500
#define NAXTEST_PITCH 0x3FFF
#define SPU_DST_ADDR (0x2800 << 1)

static void initRegs() {
    sceSdSetParam(SD_VOICE(channel, voice) | SD_VPARAM_VOLR, 0x3fff);
    sceSdSetParam(SD_VOICE(channel, voice) | SD_VPARAM_VOLL, 0x3fff);
    sceSdSetParam(SD_VOICE(channel, voice) | SD_VPARAM_PITCH, NAXTEST_PITCH);
    sceSdSetParam(SD_VOICE(channel, voice) | SD_VPARAM_ADSR1, SD_SET_ADSR1(SD_ADSR_AR_EXPi, 0x3f, 4, 0xa));
    sceSdSetParam(SD_VOICE(channel, voice) | SD_VPARAM_ADSR2, SD_SET_ADSR2(SD_ADSR_SR_EXPd, 0x3a, SD_ADSR_RR_EXPd, 0));

    sceSdSetParam(0 | SD_PARAM_MVOLL, 0x3fff);
    sceSdSetParam(0 | SD_PARAM_MVOLR, 0x3fff);
    sceSdSetParam(1 | SD_PARAM_AVOLL, 0x3fff);
    sceSdSetParam(1 | SD_PARAM_AVOLR, 0x3fff);
    sceSdSetParam(1 | SD_PARAM_MVOLL, 0x3fff);
    sceSdSetParam(1 | SD_PARAM_MVOLR, 0x3fff);

    sceSdSetParam(SD_VOICE(channel, voice) | SD_VPARAM_VOLL, 0x4000);
    nopdelay();
}

static unsigned int timerCb(void *common) {
    sampleData *d = (sampleData *)common;

    if (d->sampleIdx >= SAMPLES_MAX) {
        d->flag = 1;
        return 0;
    }

    d->samples[d->sampleIdx].envx = read16(SD_VP_VOLXL(channel, voice));
    d->samples[d->sampleIdx].cycle = GetTimerCounter(d->timer) - d->startCycle;

    // printf("Saw envx %04x at cycle %lu\n", d->samples[d->sampleIdx].envx, d->samples[d->sampleIdx].cycle);
    d->sampleIdx++;

    iop_sys_clock_t clock = {0};
    USec2SysClock(5000, &clock);

    return clock.lo;
}

void slide() {
    initRegs();

    int trans = sceSdVoiceTrans(channel, SD_TRANS_WRITE | SD_TRANS_MODE_DMA, (u8 *)adpcm_silence, (u32 *)SPU_DST_ADDR, sizeof(adpcm_silence));
    if (trans < 0) {
        printf("Bad transfer\n");
        return;
    }

    sceSdVoiceTransStatus(channel, SPU_WAIT_FOR_TRANSFER);
    printf("Voice transfer complete\n");

    sceSdSetAddr(SD_VOICE(channel, voice) | SD_VADDR_SSA, SPU_DST_ADDR);

#define TC_SYSCLOCK 1
    data.timer = AllocHardTimer(TC_SYSCLOCK, 32, 1);

    iop_sys_clock_t clock = {0};
    USec2SysClock(10, &clock);

    printf("Keying on!\n");
    sceSdSetSwitch(channel | SD_SWITCH_KON, (1 << voice));

#define ENABLE BIT(15)
#define DECREMENT BIT(13)
#define POLARITY BIT(12)
#define VALUE GENMASK(6, 0)

    u16 val = 0;
    val |= FIELD_PREP(VALUE, 0x3f);
    val |= FIELD_PREP(POLARITY, 1);
    val |= FIELD_PREP(DECREMENT, 1);
    val |= FIELD_PREP(ENABLE, 1);

    sceSdSetParam(SD_VOICE(channel, voice) | SD_VPARAM_VOLL, val);

    data.startCycle = GetTimerCounter(data.timer);
    timerCb((void *)&data);

    SetAlarm(&clock, &timerCb, &data);

    while (data.flag == 0)
        ;

    printf("Test concluded\n");

    u32 fd = open("host0:slide1", O_RDWR | O_CREAT);
    if (!fd) {
        printf("Failed to open file\n");
        return;
    }

    for (int i = 0; i < SAMPLES_MAX; i++) {
        fdprintf(fd, "%lu, %d\n", data.samples[i].cycle, data.samples[i].envx);
        // printf("%lu, %u\n", data.samples[i].cycle, data.samples[i].envx);
    }

    while (1)
        ;
}
