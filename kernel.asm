bits 32                       ;specifies that the code is for 32 bit architecture

section .text
        ;multiboot spec
        align 4               ;align the next data to a 4 byte boundary because you need that for 32 bit mutiboot
        dd 0x1BADB002         ;multiboot header magic number that tells grub that its multiboot
        dd 0x00               ;no flags
        dd - (0x1BADB002 + 0x00) ; i dont even know bro

global start                  ;declare start as global symbol
extern kmain                  ;declare kmain as external thing

start:                        ;the entry point of the kernel.
        cli                    ;disables interupts
        mov esp, stack_space   ;set stack pointer to start of stack space
        call kmain             ;call k main in kernel.c
        hlt                    ;halt the cpu to be safe

section .bss                  ;sect for uninitialized data like a buffer or whatever
resb 8192                     ; reserve 8 kb of memory in the .bss section for buffers or varibales or all that

stack_space:                  ; start of stack space
