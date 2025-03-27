#define VGA_HEIGHT 25
#define VGA_WIDTH 80
char *vga_buffer = (char *)0xB8000;
int vga_index = 0;
extern void load_idt(void *);
extern unsigned char inb(unsigned short);
extern void outb(unsigned short, unsigned char);
extern void keyboard_interrupt_wrapper(void);
typedef struct {
    unsigned short offset_low;
    unsigned short selector;
    unsigned char zero;
    unsigned char type_attr;
    unsigned short offset_high;
} __attribute__((packed)) idt_gate_t;
typedef struct {
    unsigned short limit;
    unsigned int base;
} __attribute__((packed)) idt_register_t;
#define IDT_ENTRIES 256
idt_gate_t idt[IDT_ENTRIES];
idt_register_t idt_reg;
void clear_screen() {
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT * 2; i += 2) {
        vga_buffer[i] = 0;
        vga_buffer[i + 1] = 0x07;
    }
    vga_index = 0;
}
void print_char(char c) {
    if (c == '\n') {
        vga_index = ((vga_index / (VGA_WIDTH * 2)) + 1) * (VGA_WIDTH * 2);
        return;
    }
    vga_buffer[vga_index] = c;
    vga_buffer[vga_index + 1] = 0x07;
    vga_index += 2;
}
void print_string(const char *str) {
    while (*str != '\0') {
        print_char(*str);
        str++;
    }
}
void set_idt_gate(int gate, unsigned int handler) {
    idt[gate].offset_low = handler & 0xFFFF;
    idt[gate].selector = 0x08;
    idt[gate].zero = 0;
    idt[gate].type_attr = 0x8E;
    idt[gate].offset_high = (handler >> 16) & 0xFFFF;
}
void keyboard_handler() {
    unsigned char scan_code = inb(0x60);
    if (scan_code == 0x3B) {
        print_string("Keyboard Detected: F1 Pressed!\n");
    }
    outb(0x20, 0x20);
}
void init_pic() {
    outb(0x20, 0x11);
    outb(0x21, 0x20);
    outb(0x21, 0x04);
    outb(0x21, 0x01);
    outb(0xA0, 0x11);
    outb(0xA1, 0x28);
    outb(0xA1, 0x02);
    outb(0xA1, 0x01);
    outb(0x21, 0xFD);
    outb(0xA1, 0xFF);
}
void kernel_main() {
    clear_screen();
    print_string("Welcome to My Kernel!\n");
    init_pic();
    set_idt_gate(0x21, (unsigned int)keyboard_interrupt_wrapper);
    idt_reg.limit = sizeof(idt) - 1;
    idt_reg.base = (unsigned int)&idt;
    load_idt(&idt_reg);
    __asm__ __volatile__("sti");
    while (1) {}
}
