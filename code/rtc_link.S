#define ASM     1

# RTC(name, id)
# Inputs: name, id
# Return Value: void
# Function: Handles any RTC interrupts
#define RTC(name, id)      \
    .globl name                ; \
    name:                      ; \
        pushal                 ; \
        pushl $id              ; \
        call rtc_handler       ; \
        addl $4, %esp          ; \
        popal                  ; \
        iret                   ; \

# Calling RTC to set the RTC handlers with the correct
# IDT indexes and potential error codes
RTC(rtc, 0x28);

