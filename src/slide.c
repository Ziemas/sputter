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

//#define SAMPLES_MAX 4096
#define SAMPLES_MAX 2048

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

    // sceSdSetParam(SD_VOICE(channel, voice) | SD_VPARAM_VOLL, 0x4000);
    sceSdSetParam(SD_VOICE(channel, voice) | SD_VPARAM_VOLL, 0x0);
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
    //USec2SysClock(5000, &clock);
    USec2SysClock(10, &clock);

    return clock.lo;
}

static char *phase_str[] = {
    "pos",
    "neg",
};

static char *mode_str[] = {
    "lin",
    "exp",
};

static char *dir_str[] = {
    "inc",
    "dec",
};

#define ENABLE BIT(15)
#define MODE BIT(14)
#define DECREMENT BIT(13)
#define POLARITY BIT(12)
#define VALUE GENMASK(6, 0)

int test_slide(int rate, int polarity, int decrement, int mode) {
    char filename[128];
    char *system = "pcsx2";
    int startvol = 0;

    if (decrement) {
        startvol = 0x7fff >> 1;
    }

    if (decrement && polarity) {
        startvol = 0x8000 >> 1;
    }

    memset(filename, 0, sizeof(filename));
    sprintf(filename, "host:slide/%s/slide-%s-%s-%s-%d", system, phase_str[polarity], dir_str[decrement], mode_str[mode], rate);
    printf("Collecting data for %s\n", filename);

    sceSdSetParam(SD_VOICE(channel, voice) | SD_VPARAM_VOLL, startvol);
    DelayThread(1000);

    memset(data.samples, 0, sizeof(data.samples));
    data.sampleIdx = 0;
    data.startCycle = 0;
    data.flag = 0;

    iop_sys_clock_t clock = {0};
    USec2SysClock(10, &clock);

    sceSdSetSwitch(channel | SD_SWITCH_KON, (1 << voice));

    u16 val = 0;
    val |= FIELD_PREP(VALUE, rate);
    val |= FIELD_PREP(POLARITY, polarity);
    val |= FIELD_PREP(DECREMENT, decrement);
    val |= FIELD_PREP(MODE, mode);
    val |= FIELD_PREP(ENABLE, 1);

    sceSdSetParam(SD_VOICE(channel, voice) | SD_VPARAM_VOLL, val);

    data.startCycle = GetTimerCounter(data.timer);
    timerCb((void *)&data);

    SetAlarm(&clock, &timerCb, &data);

    while (data.flag == 0)
        ;

    u32 fd = open(filename, O_RDWR | O_CREAT | O_TRUNC);
    if (!fd) {
        printf("Failed to open file %s\n", filename);
        return 1;
    }

    for (int i = 0; i < SAMPLES_MAX; i++) {
        fdprintf(fd, "%lu, %d\n", data.samples[i].cycle, data.samples[i].envx);
        // printf("%lu, %u\n", data.samples[i].cycle, data.samples[i].envx);
    }

    close(fd);

    return 0;
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

    test_slide(0x27, 0, 0, 1);
    test_slide(0x2b, 0, 0, 1);
    test_slide(0x3c, 0, 0, 1);

    //for (int phase = 0; phase < 2; phase++) {
    //    for (int dir = 0; dir < 2; dir++) {
    //        for (int mode = 0; mode < 2; mode++) {
    //            for (int rate = 0; rate < 0x7f; rate++) {
    //                test_slide(rate, phase, dir, mode);
    //            }
    //        }
    //    }
    //}

    printf("done\n");
    while (1)
        ;
}
