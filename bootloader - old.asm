;bootloader.asm multiboot compliant bootloader and interrupt stubs
section .text
    align 4
    dd 0x1BADB002           ;multiboot magic number
    dd 0x00                 ;flags
    dd -(0x1BADB002 + 0x00)  ;checksum

global start
extern k_main

;declare external c interrupt handlers
extern timer_handler
extern keyboard_handler

start:
    cli             ;disable interrupts
    call k_main     ;call the kernel main function
    hlt             ;halt if k_main ever returns

;idt loading routine
global idt_load
idt_load:
    mov eax, [esp+4]  ;get pointer to idt_ptr structure
    lidt [eax]        ;load idt register
    ret

;timer interrupt stub
global timer_handler_stub
timer_handler_stub:
    pusha                  ;save registers
    call timer_handler     ;call c timer handler
    popa                   ;restore registers
    iret                   ;return from interrupt

;keyboard interrupt stub
global keyboard_handler_stub
keyboard_handler_stub:
    pusha                   ;save registers
    call keyboard_handler   ;call c keyboard handler
    popa                    ;restore registers
    iret                    ;return from interrupt
