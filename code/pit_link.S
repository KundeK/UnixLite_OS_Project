#define ASM     1

# PIT(name, id)
# Inputs: name, id
# Return Value: void
# Function: Handles any PIT interrupts
#define PIT(name)      \
    .globl name                ; \
    name:                      ; \
        pushal                 ; \
        pushfl                 ; \
        call pit_handler       ; \
        popfl                  ; \
        popal                  ; \
        iret                   ; \

# Calling PIT to set the PIT handlers with the correct
# IDT indexes and potential error codes
PIT(pit);

