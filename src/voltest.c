#include "sputter.h"

static int channel = 0;

#define ENABLE BIT(15)
#define DECREMENT BIT(13)
#define POLARITY BIT(12)
#define VALUE GENMASK(6, 0)

static void initRegs() {
    sceSdSetParam(1 | SD_PARAM_MVOLL, 0x3fff);
    sceSdSetParam(1 | SD_PARAM_MVOLR, 0x3fff);

    sceSdSetParam(0 | SD_PARAM_MVOLL, 0x3fff);
    sceSdSetParam(0 | SD_PARAM_MVOLR, 0x3fff);


    sceSdSetParam(0 | SD_PARAM_BVOLL, 0);
    sceSdSetParam(0 | SD_PARAM_BVOLR, 0);
    sceSdSetParam(0 | SD_PARAM_AVOLL, 0);
    sceSdSetParam(0 | SD_PARAM_AVOLR, 0);

    sceSdSetParam(channel | SD_PARAM_MMIX, 0xffff);
}

static volatile int flip = 0;

static unsigned int timerCb(void *common) {
    iop_sys_clock_t clock = {0};

    u16 val = 0;
    val |= FIELD_PREP(VALUE, 0x4f);
    val |= FIELD_PREP(POLARITY, 0);
    //val |= FIELD_PREP(DECREMENT, 1);
    val |= FIELD_PREP(ENABLE, 1);

    if (read16(SD_P_MVOLXL(channel)) == 0) {
        val |= FIELD_PREP(DECREMENT, 0);
        flip = 1;

        sceSdSetParam(channel | SD_PARAM_MVOLL, val);
    }

    if (read16(SD_P_MVOLXL(channel)) >= 0x7fff) {
        val |= FIELD_PREP(DECREMENT, 1);
        flip = 2;

        sceSdSetParam(channel | SD_PARAM_MVOLL, val);
    }

    USec2SysClock(5000, &clock);
    return clock.lo;
}

void voltest() {
    printf("starting test\n");
    sceSdSetAddr(SD_ADDR_EEA, 0x1ffff << 1);
    sceSdEffectAttr attr = {};
    attr.core = 0;
    attr.mode = 5 | 0x100; // hall and clear wa with chan 0
    // attr.mode = 0 | 0x100; // hall and clear wa with chan 0
    //  attr.mode = 0x100;
    attr.depth_L = 0x7fff;
    attr.depth_R = 0x7fff;
    sceSdSetEffectAttr(0, &attr);
    //sceSdSetCoreAttr(channel | SD_CORE_EFFECT_ENABLE, 1);

    memfill16(0x7fff);

    sceSdSetParam(0 | SD_PARAM_MVOLL, 0);
    sceSdSetParam(0 | SD_PARAM_MVOLR, 0);

    iop_sys_clock_t clock = {0};
    USec2SysClock(10, &clock);
    SetAlarm(&clock, &timerCb, NULL);

    sceSdSetParam(0 | SD_PARAM_EVOLL, 0x7fff);
    sceSdSetParam(0 | SD_PARAM_EVOLR, 0x7fff);

    while (1) {
        int f = flip;

        if (f != 0) {
            printf("flip %d : %x\n", f, read16(SD_P_MVOLXL(channel)));
        }

        flip = 0;
    }
}
