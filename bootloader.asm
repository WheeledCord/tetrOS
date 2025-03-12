;bootloader.asm multiboot compliant bootloader and interrupt stubs
section .text
    align 4
    dd 0x1BADB002           ;multiboot magic number
    dd 0x00                 ;flags
    dd -(0x1BADB002 + 0x00)  ;checksum

global start
extern kernel_init

;declare external c interrupt handlers
extern tick_handler
extern key_handler

start:
    cli             ;disable interrupts
    call kernel_init     ;call the kernel_init function
    hlt             ;halt if kernel_init ever returns

;idt loading routine
global idt_load
idt_load:
    mov eax, [esp+4]  ;get pointer to idt_ptr structure
    lidt [eax]        ;load idt register
    ret

;tick interrupt stub
global tick_handler_stub
tick_handler_stub:
    pusha                  ;save registers
    call tick_handler     ;call c tick_handler
    popa                   ;restore registers
    iret                   ;return from interrupt

;key interrupt stub
global key_handler_stub
key_handler_stub:
    pusha                   ;save registers
    call key_handler   ;call c key_handler
    popa                    ;restore registers
    iret                    ;return from interrupt
