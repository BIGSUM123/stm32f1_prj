#include "toolchain/gcc.h"

    .syntax unified
    .cpu cortex-m3
    .fpu softvfp
    .thumb

GDATA(pxCurrentTCB)
GTEXT(start_first_thread)
GTEXT(w_arm_pendsv)
GTEXT(w_arm_svc)

SECTION_FUNC(text, w_arm_svc)
    cpsid i
    // 弹出r4-r11设置psp
    ldr r1, =pxCurrentTCB
    ldr r1, [r1]
    ldr r0, [r1]
    ldmia r0!, {r4-r11}
    msr psp, r0
    isb

    // 设置了lr就行
    // 不需要显性设置control
    mov r0, #0
    msr basepri, r0
    orr r14, #0xd
    
    cpsie i
    bx r14

SECTION_FUNC(text, start_first_thread)
    // 把msp重置
    ldr r0, =0xE000ED08
    ldr r0, [r0]
    ldr r0, [r0]
    msr msp, r0
    // 触发scv中断
    cpsie i
    cpsie f
    dsb
    isb
    svc 0
    nop
    .ltorg

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
