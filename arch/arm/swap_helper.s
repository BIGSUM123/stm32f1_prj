#include "toolchain/gcc.h"

    .syntax unified
    .cpu cortex-m3
    .fpu softvfp
    .thumb

GDATA(pxCurrentTCB)
GTEXT(w_arm_pendsv)

SECTION_FUNC(text, w_arm_pendsv)
    CPSID I
    MRS R0, PSP

    STMDB R0!, {R4-R11}

    LDR R1, =pxCurrentTCB
    LDR R2, [R1]
    STR R0, [R2]

    LDR R1, =pxCurrentTCB
    LDR R2, [R1]
    LDR R0, [R2]

    LDMIA R0!, {R4-R11}

    MSR PSP, R0

    CPSIE I
    BX LR
