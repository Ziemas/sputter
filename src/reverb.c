#include "sputter.h"

static int channel = 0;
static int voice = 1;

static const u32 SPU_DST_ADDR = (0x2800 << 1);

static s16 samples[200];

#define ENABLE BIT(15)
#define DECREMENT BIT(13)
#define POLARITY BIT(12)
#define VALUE GENMASK(6, 0)

static void initRegs() {
    sceSdSetParam(SD_VOICE(channel, voice) | SD_VPARAM_VOLR, 0x3fff);
    sceSdSetParam(SD_VOICE(channel, voice) | SD_VPARAM_VOLL, 0x3fff);
    sceSdSetParam(SD_VOICE(channel, voice) | SD_VPARAM_PITCH, 0x1000);
    sceSdSetParam(SD_VOICE(channel, voice) | SD_VPARAM_ADSR1, SD_SET_ADSR1(SD_ADSR_AR_EXPi, 0, 0x7f, 0xf));
    sceSdSetParam(SD_VOICE(channel, voice) | SD_VPARAM_ADSR2, SD_SET_ADSR2(SD_ADSR_SR_EXPd, 127, SD_ADSR_RR_LINEARd, 0x10));

    sceSdSetParam(0 | SD_PARAM_MVOLL, 0x3fff);
    sceSdSetParam(0 | SD_PARAM_MVOLR, 0x3fff);

    sceSdSetParam(0 | SD_PARAM_EVOLL, 0x7fff);
    sceSdSetParam(0 | SD_PARAM_EVOLR, 0x7fff);

    sceSdSetParam(1 | SD_PARAM_AVOLL, 0x7fff);
    sceSdSetParam(1 | SD_PARAM_AVOLR, 0x7fff);

    sceSdSetParam(1 | SD_PARAM_MVOLL, 0x3fff);
    sceSdSetParam(1 | SD_PARAM_MVOLR, 0x3fff);

    u16 val = 0;
    val |= FIELD_PREP(VALUE, 0x0);
    val |= FIELD_PREP(POLARITY, 0);
    val |= FIELD_PREP(DECREMENT, 0);
    val |= FIELD_PREP(ENABLE, 1);

    sceSdSetParam(0 | SD_PARAM_MVOLL, val);
    sceSdSetParam(0 | SD_PARAM_MVOLR, val);
    sceSdSetParam(1 | SD_PARAM_MVOLL, val);
    sceSdSetParam(1 | SD_PARAM_MVOLR, val);

    // sceSdSetSwitch(channel | SD_SWITCH_VMIXL, (1 << (voice)));
    // sceSdSetSwitch(channel | SD_SWITCH_VMIXR, 0);

    // sceSdSetSwitch(channel | SD_SWITCH_VMIXEL, 0);
    // sceSdSetSwitch(channel | SD_SWITCH_VMIXER, (1 << (voice)));

    // sceSdSetParam(channel | SD_PARAM_MMIX, 0b0000000000);
    sceSdSetParam(channel | SD_PARAM_MMIX, 0xffff);
}

// void reverbregtest() {
//     u16 ESA = 0, EEA = 0;
//     *SD_A_EEA_HI(0) = 0xffff;
//     *SD_A_ESA_HI(0) = 0xffff;
//
//     DelayThread(2000000);
//
//     EEA = *SD_A_EEA_HI(0);
//     ESA = *SD_A_EEA_HI(0);
//
//     printf("ESA %x, EEA %x", EEA, ESA);
// }

void reverb_upsample() {
    initRegs();

    memset(samples, 0, sizeof(samples));
    samples[100] = 0x7fff;
    samples[0] = -0x8000;
    // for (int i = 0; i < 100; i++) {
    //     samples[i] = 0x7fff;
    // }

    sceSdSetAddr(SD_ADDR_EEA, 0xffff << 1);
    sceSdEffectAttr attr = {};
    attr.core = channel;
    attr.mode = 5 | 0x100; // hall and clear wa with chan 0
    // attr.mode = 0 | 0x100; // hall and clear wa with chan 0
    //  attr.mode = 0x100;
    attr.depth_L = 0x7fff;
    attr.depth_R = 0x7fff;
    sceSdSetEffectAttr(0, &attr);

    //*SD_R_FB_X(0) = 0;

    sceSdSetCoreAttr(channel | SD_CORE_EFFECT_ENABLE, 0);
    sceSdCleanEffectWorkArea(0, channel, 5);
    sceSdVoiceTransStatus(0, SPU_WAIT_FOR_TRANSFER);

    u32 start = sceSdGetAddr(channel | SD_ADDR_ESA);
    u32 end = sceSdGetAddr(channel | SD_ADDR_EEA);
    printf("uploading to %x (end %x)\n", start, end);
    sceSdVoiceTrans(channel, SD_TRANS_WRITE, (u8 *)samples, (u32 *)start, sizeof(samples));

    // sceSdSetAddr(SD_VOICE(channel, voice) | SD_VADDR_SSA, SPU_DST_ADDR);
    // loadSound("host0:crash.adp", 0, (u32 *)SPU_DST_ADDR);
    // sceSdSetSwitch(channel | SD_SWITCH_KON, (1 << voice));
}

static volatile int test;

int spu2irq(int c, void *) {
    // printf("[%d] got irq\n", c);

    test = c + 1;
    return 0;
}

void reverb_irq() {
    test = 0;

    sceSdSetAddr(SD_ADDR_EEA, 0xffff << 1);

    sceSdEffectAttr attr = {};
    attr.core = channel;
    attr.mode = 5 | 0x100; // hall and clear wa with chan 0
                           // attr.mode = 0 | 0x100; // hall and clear wa with chan 0
                           //  attr.mode = 0x100;
    attr.depth_L = 0x7fff;
    attr.depth_R = 0x7fff;
    sceSdSetEffectAttr(0, &attr);

    sceSdSetCoreAttr(channel | SD_CORE_EFFECT_ENABLE, 0);
    // sceSdCleanEffectWorkArea(0, channel, 5);

    sceSdSetSpu2IntrHandler(spu2irq, NULL);
    sceSdSetAddr(SD_ADDR_IRQA, 0xf000 << 1);
    sceSdSetCoreAttr(SD_CORE_IRQ_ENABLE, 1);

    int t = 0;
    int i = 0;
    for (;;) {
        if (test) {
            printf("[%d] got irq\n", test - 1);

            test = 0;
            sceSdSetCoreAttr(SD_CORE_IRQ_ENABLE, 1);
        }
        DelayThread(100000);

        i++;
        if (i == 30) {
            printf("toggling fxenable\n");
            t ^= 1;
            i = 0;
            sceSdSetCoreAttr(channel | SD_CORE_EFFECT_ENABLE, t);
        }
    }
}
