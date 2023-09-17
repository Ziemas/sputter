#ifndef __SPUTTER_H_
#define __SPUTTER_H_
#include "irx_imports.h"
#include "spu2regs.h"

#define USEC_SECOND 1000000
#define USEC_MS 1000

#define SPU_WAIT_FOR_TRANSFER 1

#define SD_BLOCK_C0_VOICE1 (0x0 << 8)
#define SD_BLOCK_C0_VOICE3 (0x1 << 8)
#define SD_BLOCK_C1_SINL (0x2 << 8)
#define SD_BLOCK_C1_SINR (0x3 << 8)
#define SD_BLOCK_C1_VOICE1 (0x4 << 8)
#define SD_BLOCK_C1_VOICE3 (0x5 << 8)
#define SD_BLOCK_C0_MEMOUTL (0x6 << 8)
#define SD_BLOCK_C0_MEMOUTR (0x7 << 8)
#define SD_BLOCK_C0_MEMOUTEL (0x8 << 8)
#define SD_BLOCK_C0_MEMOUTER (0x9 << 8)
#define SD_BLOCK_C1_MEMOUTL (0xa << 8)
#define SD_BLOCK_C1_MEMOUTR (0xb << 8)
#define SD_BLOCK_C1_MEMOUTEL (0xc << 8)
#define SD_BLOCK_C1_MEMOUTER (0xd << 8)

#define SD_BLOCK_HANDLER (1 << 7)
#define SD_BLOCK_COUNT(x) ((x) << 12)

#define SD_MMIX_SINER  (1 <<  0)
#define SD_MMIX_SINEL  (1 <<  1)
#define SD_MMIX_SINR   (1 <<  2)
#define SD_MMIX_SINL   (1 <<  3)
#define SD_MMIX_MINER  (1 <<  4)
#define SD_MMIX_MINEL  (1 <<  5)
#define SD_MMIX_MINR   (1 <<  6)
#define SD_MMIX_MINL   (1 <<  7)
#define SD_MMIX_MSNDER (1 <<  8)
#define SD_MMIX_MSNDEL (1 <<  9)
#define SD_MMIX_MSNDR  (1 << 10)
#define SD_MMIX_MSNDL  (1 << 11)

#define TC_SYSCLOCK 1

void naxTest();
void envx();
void noiseTest();
void playSound();
void blockRead();
void bufdetect();
void reverbtest();
void reverbregtest();
void dmatest();
void memdump(char* filename);
void memfill(u8 value);

u32 loadSound(const char *filename, u32 channel, u32 *spudst);
#endif // __SPUTTER_H_
