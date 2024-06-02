#define ASM     1

# EXCEPTION(name, id)
# Inputs: name, id
# Return Value: void
# Function: Handles any program exceptions by displaying exception message
#define EXCEPTION(name, id)         \
    .globl name                   ; \
    name:                         ; \
        pushal                    ; \
        pushl   $-1               ; \
        pushl   $id               ; \
        call    handle_exception  ; \
        addl    $8, %esp          ; \
        popal                     ; \
        iret                      ; \

# EXCEPTION_ERROR_CODE(name, id, err_code)
# Inputs: name, id, err_code
# Return Value: void
# Function: Handles any program exceptions that are associated with an error code by displaying exception message
#define EXCEPTION_ERROR_CODE(name, id, err_code)    \
    .globl name                                   ; \
    name:                                         ; \
        pushal                                    ; \
        pushl   $err_code                         ; \
        pushl   $id                               ; \
        call    handle_exception                  ; \
        addl    $8, %esp                          ; \
        popal                                     ; \
        iret                                      ; \

# Calling EXCEPTION and EXCEPTION_ERROR_CODE to set the exception handlers 
# with the correct IDT indexes and potential error codes
EXCEPTION(divide_error, 0x0);
EXCEPTION(single_step, 0x1);
EXCEPTION(nmi, 0x2);
EXCEPTION(breakpoint, 0x3);
EXCEPTION(overflow, 0x4);
EXCEPTION(bound_range_exceeded, 0x5);
EXCEPTION(invalid_opcode, 0x6);
EXCEPTION(dna, 0x7);
EXCEPTION_ERROR_CODE(double_fault, 0x8, 0);
EXCEPTION(coproc, 0x9);
EXCEPTION_ERROR_CODE(invalid_tss, 0xA, 0);
EXCEPTION_ERROR_CODE(snp, 0xB, 0);
EXCEPTION_ERROR_CODE(ssf, 0xC, 0);
EXCEPTION_ERROR_CODE(gen_prot, 0xD, 0);
EXCEPTION_ERROR_CODE(page_fault, 0xE, 0);
EXCEPTION(math_fault, 0x10);
EXCEPTION_ERROR_CODE(align_check, 0x11, 0);
EXCEPTION(machine_check, 0x12);
EXCEPTION(floatp_excep, 0x13);
