#define ASM     1

# KEYBOARD(name, id)
# Inputs: name, id
# Return Value: void
# Function: Handles any keyboard interrupts 
#define KEYBOARD(name, id)      \
    .globl name                ; \
    name:                      ; \
        pushal                 ; \
        pushl $id              ; \
        call keyboard_handler  ; \
        addl $4, %esp          ; \
        popal                  ; \
        iret                   ; \

# Calling KEYBOARD to set the keyboard handlers with the correct
# IDT indexes and potential error codes
KEYBOARD(keyboard, 0x21);

