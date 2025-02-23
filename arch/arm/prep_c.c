

#define VECTOR_ADDRESS ((uintptr_t)_vector_start)

static inline void relocate_vector_table(void)
{
	// SCB->VTOR = VECTOR_ADDRESS & SCB_VTOR_TBLOFF_Msk;
	// barrier_dsync_fence_full();
	// barrier_isync_fence_full();
}

extern void w_cstart(void);

/**
 *
 * @brief Prepare to and run C code
 *
 * This routine prepares for the execution of and runs C code.
 *
 */
void w_prep_c(void)
{
#if defined(CONFIG_SOC_PREP_HOOK)
	soc_prep_hook();
#endif

	relocate_vector_table();

// #if defined(CONFIG_CPU_HAS_FPU)
// 	z_arm_floating_point_init();
// #endif

// 	z_bss_zero();
// 	z_data_copy();

// #if defined(CONFIG_ARM_CUSTOM_INTERRUPT_CONTROLLER)
// 	/* Invoke SoC-specific interrupt controller initialization */
// 	z_soc_irq_init();
// #else
// 	z_arm_interrupt_init();
// #endif /* CONFIG_ARM_CUSTOM_INTERRUPT_CONTROLLER */

// #if CONFIG_ARCH_CACHE
// 	arch_cache_init();
// #endif

// #ifdef CONFIG_NULL_POINTER_EXCEPTION_DETECTION_DWT
// 	z_arm_debug_enable_null_pointer_detection();
// #endif

	w_cstart();
}