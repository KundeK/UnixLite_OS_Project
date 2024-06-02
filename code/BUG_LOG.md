CHECKPOINT 1 BUGS (3/19/24)
1. EOI message was not being sent to the PIC after a keyboard interrupt
    Explanation: In our send_eoi() function, we were writing our EOI message to the Master and Slave data ports instead of the command ports. In order for the PIC to 
                receive an EOI, it must be sent through the command ports. This means that the initial interrupt never ended which didn't allow for another interrupt to occur.

2. Our RTC was not being initialized and was not triggering any interrupts
    Explanation: We placed the code that activated the periodic interrupts into the rtc handler instead of our rtc_init() function. We only enabled the IRQ on the PIC, but did not 
                enable to the RTC. To correct this we moved our periodic interrupt activation code into our from the handler to our init function.
    
3. Our RTC was not triggering periodic interrupts
    Explanation: When implementing the rtc_init() and rtc handler functions, we swapped the data and port inputs of the outb() function. This meant we weren't properly activating the 
                periodic interrupt functionality of the RTC.

4. Paging was being enabled properly after running in-line assembly line in paging_init(), but was being disabled one line later causing boot loop.
    Explanation: 4 MB is equivalent to 2^22, so there needs to be a 1 in the 22nd address bit. However, this is bit 22 of a page directory entry, and we were putting it in bit 13.


CHECKPOINT 2 BUGS (3/25/24)
1. Terminal read was not copying buffer correctly
    Explanation: The buffer was not being passed by pointer so the values were not being saved.

2. File system read_dentry_by_name could not find right file
    Explanation: We were originally only comparing the first character of the file names which was not returning the right file. We changed this to strcmp which fixed the searching issues in read_dentry_by_name

3. File system read_data was not returning correct data/inode/length
    Explanation: We were calculating the starting address incorrectly, dividing the correct address value by 4, rather than taking the regular value. Fixing this resulted in being able to get the data correctly.


CHECKPOINT 3 BUGS (4/8/24)
1. Stack was overwriting the PCB.
    Explanation: read_data was pushing an inode and data block object onto the stack, which are both 4KB. This meant the PCBs that were at the bottom of the kernel space were being overwritten by these stack values. This was fixed by using pointers rather than new inode/data_block objects.

2. Halt system calls were resulting in page faults
    Explanation: We were not executing a "leave" instruction in our inline assembly, which would update the esp and ebp to the correct values.

3. System call return values were the same as the system call number
    Explanation: In the assembly linkage file, we were saving eax and restoring it at the end of the assembly linkage file, which was setting eax back to the system call number rather than the system call's return value.


CHECKPOINT 4 BUGS (4/15/24)
1. Cursor code was overdeleting video memory data
    Explanation: The cursor is continuously drawn and undrawn to the screen, and when hitting backspace or enter the undrawing code would erase certain parts of the video memory which should be maintained. We fixed this by slightly constraining the for loop which was responsible for undrawing the cursor character from the shell.

2. RTC Virtualization wasn't calculating the rate correctly
    Explanation: We were incorrectly clearing the RTC wait flag because our order of operations for the mod calcuation with our ticks was incorrect, so we added in the necessary parentheses.


CHECKPOINT 5 BUGS (4/28/24)
1. Scheduling was causing stack overflow
    Explanation: We were initially setting the tss.ESP0 to the top of the kernel stack instead of the bottom of that stack which meant that the stack would grow into the PCB

2. Switching terminals would corrupt the keyboard buffer
    Explanation: By adding a keyboard buffer index to the terminal struct, we were able to save and restore each terminals unique keyboard buffer index when switching terminals.

3. Page faulting because improper context switching
    Explanation: We were loading in the previous terminals process context instead of the active terminals process context, so we needed to add saving the ebp and esp for each process to the PCB. 