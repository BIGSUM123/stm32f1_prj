#include "vector_table.h"

    .syntax unified
    .cpu cortex-m3
    .fpu softvfp
    .thumb

.section .isr_vector,"a",%progbits
    .type _vector_table, %object
    .size _vector_table, .-_vector_table

_vector_table:
    .word _estack
    .word w_arm_reset
    .word w_arm_nmi
    .word w_arm_hard_fault
    .word w_arm_mpu_fault
    .word w_arm_bus_fault
    .word w_arm_usage_fault
    .word 0
    .word 0
    .word 0
    .word 0
    .word w_arm_svc
    .word w_arm_debug_monitor
    .word 0
    .word w_arm_pendsv
    .word sys_clock_isr

