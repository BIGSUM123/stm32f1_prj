#ifndef __TOOLCHAIN_GCC_H__
#define __TOOLCHAIN_GCC_H__

#define PERFOPT_ALIGN .balign  4

#define FUNC_CODE() .thumb;
#define FUNC_INSTR(a)

#define GTEXT(sym) .global sym; .type sym, %function
#define GDATA(sym) .global sym; .type sym, %object
#define WTEXT(sym) .weak sym; .type sym, %function
#define WDATA(sym) .weak sym; .type sym, %object

#define SECTION_VAR(sect, sym)  .section .sect.sym; sym:
#define SECTION_FUNC(sect, sym)						\
	.section .sect.sym, "ax";					\
				FUNC_CODE()				\
				PERFOPT_ALIGN; sym :		\
							FUNC_INSTR(sym)
#define SECTION_SUBSEC_FUNC(sect, subsec, sym)				\
		.section .sect.subsec, "ax"; PERFOPT_ALIGN; sym :

#endif // __TOOLCHAIN_GCC_H__