#include "sputter.h"
#include "types.h"
#include "stdio.h"

IRX_ID("sputter", 1, 1);
extern struct irx_export_table _exp_hello;

u32 _start() {
    printf("hello world\n");
    return 0;
}
