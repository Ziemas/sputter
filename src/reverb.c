#include "sputter.h"

static int channel = 0;
static int voice = 1;

static const u32 SPU_DST_ADDR = (0x2800 << 1);

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

    sceSdSetSwitch(channel | SD_SWITCH_VMIXL, (1 << (voice)));
    sceSdSetSwitch(channel | SD_SWITCH_VMIXR, (1 << (voice)));
    sceSdSetSwitch(channel | SD_SWITCH_VMIXL, 0);
    sceSdSetSwitch(channel | SD_SWITCH_VMIXR, 0);
    sceSdSetSwitch(channel | SD_SWITCH_VMIXEL, (1 << (voice)));
    sceSdSetSwitch(channel | SD_SWITCH_VMIXER, (1 << (voice)));
    sceSdSetParam(channel | SD_PARAM_MMIX, 0xffff);
}


void reverbregtest() {
    u16 ESA = 0, EEA = 0;
    *SD_A_EEA_HI(0) = 0xffff;
    *SD_A_ESA_HI(0) = 0xffff;

    DelayThread(2000000);

    EEA = *SD_A_EEA_HI(0);
    ESA = *SD_A_EEA_HI(0);

    printf("ESA %x, EEA %x", EEA, ESA);
}

void reverbtest() {
    initRegs();

    memfill(0xff);

    sceSdSetAddr(SD_ADDR_EEA, 0x1ffff << 1);
    sceSdEffectAttr attr = {};
    attr.core = 0;
    attr.mode = 5 | 0x100; // hall and clear wa with chan 0
    // attr.mode = 0x100;
    attr.depth_L = 0x7fff;
    attr.depth_R = 0x7fff;
    sceSdSetEffectAttr(0, &attr);
    sceSdSetCoreAttr(channel | SD_CORE_EFFECT_ENABLE, 0);

    memfill(0xff);

    sceSdSetAddr(SD_VOICE(channel, voice) | SD_VADDR_SSA, SPU_DST_ADDR);
    loadSound("host0:sine.adp", 0, (u32 *)SPU_DST_ADDR);
    sceSdSetSwitch(channel | SD_SWITCH_KON, (1 << voice));

    sceSdSetCoreAttr(channel | SD_CORE_EFFECT_ENABLE, 0);
    DelayThread(200000);
    //memdump("host0:spumem-0.bin");

    //sceSdSetAddr(SD_ADDR_ESA, 0xfff8 << 1);
    //sceSdSetAddr(SD_ADDR_ESA, 0x9910 << 1);
    sceSdSetAddr(SD_ADDR_ESA, 0x1b910 << 1);
    sceSdSetCoreAttr(channel | SD_CORE_EFFECT_ENABLE, 1);

    DelayThread(2000000);
    DelayThread(2000000);
    DelayThread(2000000);

    sceSdSetCoreAttr(channel | SD_CORE_EFFECT_ENABLE, 0);
    memdump("host0:spumem-1.bin");

    //sceSdSetAddr(SD_ADDR_EEA, 0xeffff << 1);

    return;

    //char fname[20];
    //for (int i = 0; i < 30; i++) {
    //    sprintf(fname, "host0:spumem-%d.bin", i + 1);
    //    sceSdSetCoreAttr(channel | SD_CORE_EFFECT_ENABLE, 0);
    //    DelayThread(200000);
    //    sceSdSetAddr(SD_ADDR_ESA, 0xfff8 << 1);
    //    sceSdSetCoreAttr(channel | SD_CORE_EFFECT_ENABLE, 1);
    //    memdump(fname);
    //}
}
