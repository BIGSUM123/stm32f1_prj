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
    CPSID   I                   // 关中断
    LDR     R0, =pxCurrentTCB   // 加载 pxCurrentTCB 地址到 R0
    LDR     R0, [R0]            // 获取当前 TCB 指针（如 &tcb1）
    LDR     R1, [R0]            // 获取栈顶指针 → R1
    MSR     PSP, R1             // 设置 PSP 为栈顶指针
    MOV     R2, #0x02           // CONTROL[1] = 1 (使用 PSP)
    MSR     CONTROL, R2
    ISB                         // 必须的指令同步
    MOV     LR, #0xFFFFFFFD     // EXC_RETURN: 使用 PSP + 线程模式
    CPSIE   I                   // 开中断
    BX      LR                  // 触发上下文切换

SECTION_FUNC(text, w_arm_pendsv)
    CPSID I

    MRS R0, PSP
    STMDB R0!, {R4-R11}
    LDR R1, =pxCurrentTCB
    LDR R2, [R1]
    STR R0, [R2]

    BL w_scheduler

    LDR R1, =pxCurrentTCB
    LDR R2, [R1]
    LDR R0, [R2]
    LDMIA R0!, {R4-R11}
    MSR PSP, R0

    CPSIE I
    BX LR
