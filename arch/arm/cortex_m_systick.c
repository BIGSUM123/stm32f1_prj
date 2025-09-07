#include "drivers/timer/system_timer.h"

__attribute__((interrupt("IRQ"))) void sys_clock_isr(void)
{
    sys_clock_announce(1);
}