#include "toolchain/gcc.h"

    .syntax unified
    .cpu cortex-m3
    .fpu softvfp
    .thumb

GDATA(pxCurrentTCB)
// GTEXT(start_first_thread)
GTEXT(w_arm_pendsv)
GTEXT(w_arm_svc)

SECTION_FUNC(text, w_arm_svc)
// SECTION_FUNC(text, start_first_thread)
    cpsid i
    ldr r0, =pxCurrentTCB
    ldr r0, [r0]
    ldr r1, [r0]
    msr psp, r1
    mov r2, #0x02
    msr control, r2
    isb
    mov lr, #0xFFFFFFFD
    cpsie i
    bx lr

SECTION_FUNC(text, w_arm_pendsv)
    cpsid i                 // 关中断
    push {lr}               // 保存原始EXC_RETURN到堆栈

    mrs r0, psp             // 获取线程栈指针
    stmdb r0!, {r4-r11}     // 保存用户态寄存器
    ldr r1, =pxCurrentTCB
    ldr r2, [r1]
    str r0, [r2]            // 更新任务控制块的栈顶指针

    bl w_scheduler          // 调用调度器（会覆盖LR）

    ldr r1, =pxCurrentTCB
    ldr r2, [r1]
    ldr r0, [r2]            // 获取新任务的栈指针
    ldmia r0!, {r4-r11}     // 恢复新任务的寄存器
    msr psp, r0             // 更新线程栈指针

    pop {lr}                // 恢复EXC_RETURN到LR
    cpsie i                 // 开中断
    bx lr                   // 正确退出异常
