/* 
 * exception.h - Defines code exception handlers
 */

#ifndef _EXCEPTION_MACRO
#define _EXCEPTION_MACRO

/* We create the necessary exception handler functions */
extern void divide_error(void);
extern void single_step(void);
extern void nmi(void);
extern void breakpoint(void);
extern void overflow(void);
extern void bound_range_exceeded(void);
extern void invalid_opcode(void);
extern void dna(void);
extern void double_fault(void);
extern void coproc(void);
extern void invalid_tss(void);
extern void snp(void);
extern void ssf(void);
extern void gen_prot(void);
extern void page_fault(void);
// INTEL RESERVED
extern void math_fault(void);
extern void align_check(void);
extern void machine_check(void);
extern void floatp_excep(void);

#endif
