#ifndef __ARCH_ARM_VECTOR_TABLE_H__
#define __ARCH_ARM_VECTOR_TABLE_H__

#include "toolchain/gcc.h"

GTEXT(__start)
GDATA(_vector_table)

GTEXT(w_arm_reset)
GTEXT(w_arm_nmi)
GTEXT(w_arm_hard_fault)
GTEXT(w_arm_mpu_fault)
GTEXT(w_arm_bus_fault)
GTEXT(w_arm_usage_fault)
GTEXT(w_arm_svc)
GTEXT(w_arm_debug_monitor)
GTEXT(w_arm_pendsv)
GTEXT(sys_clock_isr)

GTEXT(z_arm_exc_spurious)

#endif // __ARCH_ARM_VECTOR_TABLE_H__