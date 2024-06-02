# Project Overview

This project involved the development of a Unix-like operating system from scratch, focusing on critical OS components, system calls, file system integration, and advanced scheduling functionalities. The goal was to gain hands-on experience in building core OS features and ensuring efficient multitasking and program execution.

## Key Features

### Core OS Components
- **Interrupt Descriptor Table (IDT)**: Set up the IDT for handling exceptions, interrupts, and system calls.
- **Paging Support**: Implemented basic paging for memory management.
- **Device Drivers**: Developed drivers for keyboard, PIC, and RTC devices.
- **Bootloader**: Utilized GRUB for system initialization and booting into protected mode.

### System Calls and File System
- **System Calls**: Developed ten essential system calls (`halt`, `execute`, `read`, `write`, `open`, `close`, `getargs`, `vidmap`, `set handler`, `sigreturn`) using `int $0x80`, enabling user-level programs to interact with the OS.
- **File System**: Designed and integrated a read-only file system to support program execution and file operations.

### Advanced OS Functionality
- **Round-Robin Scheduler**: Configured the Programmable Interval Timer (PIT) for a round-robin scheduler, supporting multiple terminals and preemptive multitasking.
- **Virtual Memory Management**: Implemented virtual memory management for efficient terminal switching and active task management, ensuring non-disruptive operation across multiple processes.

## Project Structure

- **BUG_LOG.md**: Documented the bugs encountered and their resolutions.
- **DEBUG/**: Directory containing debug-related files and scripts.
- **INSTALL**: Instructions for installing and booting the OS.
- **Makefile**: Build configuration for the project.
- **src/**: Source code directory containing all OS components and drivers.
  - `boot.S`: Assembly code for bootstrapping and initializing the system.
  - `kernel.c`: Core kernel functionality.
  - `paging.c`, `paging.h`: Paging and memory management.
  - `idt.c`, `idt.h`: Interrupt descriptor table setup.
  - `pit.c`, `pit.h`: Programmable Interval Timer configuration.
  - `rtc.c`, `rtc.h`: Real-time clock driver.
  - `keyboard.c`, `keyboard.h`: Keyboard driver.
  - `file_system.c`, `file_system.h`: File system implementation.
  - `system_call.c`, `system_call.h`: System call interface and handlers.
  - `terminal.c`, `terminal.h`: Terminal driver.
  - `processes.c`, `processes.h`: Process control and management.
  - `i8259.c`, `i8259.h`: PIC (Programmable Interrupt Controller) setup.


### Prerequisites
- GRUB bootloader
- QEMU or similar emulator for testing


