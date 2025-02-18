#include "toolchain/gcc.h"

    .syntax unified
    .cpu cortex-m3
    .fpu softvfp
    .thumb

GTEXT(w_arm_fault)

GTEXT(w_arm_hard_fault)
GTEXT(w_arm_mpu_fault)
GTEXT(w_arm_bus_fault)
GTEXT(w_arm_usage_fault)
WTEXT(w_arm_debug_monitor)
GTEXT(w_arm_exc_spurious)

SECTION_SUBSEC_FUNC(text,__fault,w_arm_hard_fault)
SECTION_SUBSEC_FUNC(text,__fault,w_arm_mpu_fault)
SECTION_SUBSEC_FUNC(text,__fault,w_arm_bus_fault)
SECTION_SUBSEC_FUNC(text,__fault,w_arm_usage_fault)
SECTION_SUBSEC_FUNC(text,__fault,w_arm_debug_monitor)
SECTION_SUBSEC_FUNC(text,__fault,w_arm_exc_spurious)
    b .

/**
 * @brief 这些先临时定义在这里
 * 
 */
GTEXT(w_arm_nmi)
GTEXT(sys_clock_isr)

SECTION_SUBSEC_FUNC(text,__fault,w_arm_nmi)
SECTION_SUBSEC_FUNC(text,__fault,sys_clock_isr)
    bx lr
