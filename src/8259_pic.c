#include "isr.h"
#include "idt.h"
#include "io_ports.h"
#include "8259_pic.h"

void pic8259_init() {
    uint8 a1, a2;

    //save mask registers
    a1 = inportb(PIC1_DATA);
    a2 = inportb(PIC2_DATA);

    //send commands to pic to initialize both master & slave
    outportb(PIC1_COMMAND, ICW1);
    outportb(PIC2_COMMAND, ICW1);

    //map vector offset of all default irqs from 0x20 to 0x27 in master (ICW2)
    outportb(PIC1_DATA, 0x20);
    // map vector offset of all default irq's from 0x28 to 0x2F in slave (ICW2)
    outportb(PIC2_DATA, 0x28);

    // tell master pic that there is a slave pic at irq2  (0000 0100)
    outportb(PIC1_DATA, 4);
    // tell slave pic its cascade identity (0000 0010)
    outportb(PIC2_DATA, 2);

    //make set x86
    outportb(PIC1_DATA, ICW4_8086);
    outportb(PIC2_DATA, ICW4_8086);

    //restore the mask registers
    outportb(PIC1_DATA, a1);
    outportb(PIC2_DATA, a2);
}

//send eoi
void pic8259_eoi(uint8 irq) {
    if(irq >= 0x28)
        outportb(PIC2, PIC_EOI);
    outportb(PIC1, PIC_EOI);
}

