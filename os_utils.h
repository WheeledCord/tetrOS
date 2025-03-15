#ifndef OS_UTLS_H
#define OS_UTLS_H

void outb(unsigned short port, unsigned char data) {
    asm volatile ("outb %0, %1" : : "a"(data), "Nd"(port));
}
unsigned char inb(unsigned short port) {
    unsigned char ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}
void pic_remap() {
    unsigned char a1 = inb(0x21);
    unsigned char a2 = inb(0xA1);
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0x20);
    outb(0xA1, 0x28);
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    outb(0x21, a1);
    outb(0xA1, a2);
}
struct idt_entry {
    unsigned short base_low;
    unsigned short sel;
    unsigned char always0;
    unsigned char flags;
    unsigned short base_high;
} __attribute__((packed));
struct idt_ptr {
    unsigned short limit;
    unsigned int base;
} __attribute__((packed));
struct idt_entry idt[256];
struct idt_ptr idtp;
void idt_set_gate(unsigned char num, unsigned int base, unsigned short sel, unsigned char flags) {
    idt[num].base_low  = base & 0xFFFF;
    idt[num].base_high = (base >> 16) & 0xFFFF;
    idt[num].sel       = sel;
    idt[num].always0   = 0;
    idt[num].flags     = flags;
}
extern void idt_load(unsigned int);
void k_install_idt() {
    idtp.limit = sizeof(struct idt_entry) * 256 - 1;
    idtp.base  = (unsigned int)&idt;
    extern void tick_handler_stub();
    extern void key_handler_stub();
    idt_set_gate(32, (unsigned int)tick_handler_stub, 0x08, 0x8E);
    idt_set_gate(33, (unsigned int)key_handler_stub, 0x08, 0x8E);
    idt_load((unsigned int)&idtp);
}

unsigned char read_cmos(unsigned char reg) {
    outb(0x70, reg);
    return inb(0x71);
}
unsigned char bcd_to_bin(unsigned char val) {
    return (val & 0x0F) + ((val >> 4) * 10);
}
unsigned int rtc_to_unix(int sec, int min, int hour, int day, int month, int year) {
    if (year < 1970) return 0;
    // Days since epoch (1970-01-01)
    int days = (year - 1970) * 365 + ((year - 1969) / 4);
    int month_days[12] = {0,31,59,90,120,151,181,212,243,273,304,334};
    days += month_days[month - 1] + (day - 1);
    if (month > 2 && (year % 4 == 0) && (year % 100 != 0 || year % 400 == 0)) {
        days++;
    }
    return days * 86400 + hour * 3600 + min * 60 + sec;
}

#endif