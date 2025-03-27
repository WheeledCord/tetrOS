BITS 32
SECTION .text
align 4
dd 0x1BADB002
dd 0x00
dd -(0x1BADB002+0x00)
GLOBAL start
GLOBAL load_idt
GLOBAL inb
GLOBAL outb
GLOBAL keyboard_interrupt_wrapper
EXTERN kernel_main
EXTERN keyboard_handler
start:
    cli
    call kernel_main
    jmp end
load_idt:
    mov eax, [esp+4]
    lidt [eax]
    ret
inb:
    mov dx, [esp+4]
    in al, dx
    ret
outb:
    mov dx, [esp+4]
    mov al, [esp+8]
    out dx, al
    ret
keyboard_interrupt_wrapper:
    pusha
    call keyboard_handler
    popa
    iret
end:
    hlt
    jmp end
