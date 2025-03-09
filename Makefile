AS = nasm
CC = i686-linux-musl-gcc
LD = i686-linux-musl-ld

ASFLAGS = -f elf32
CFLAGS = -ffreestanding -m32 -c -g
LDFLAGS = -T linker.ld

BOOTLOADER_SRC = bootloader.asm
KERNEL_SRC = kernel.c
BOOTLOADER_OBJ = bootloader.o
KERNEL_OBJ = kernel.o
KERNEL_BIN = kernel.elf
ISO_NAME = tetros.iso
ISO_DIR = iso
BOOTLOADER_BIN = bootloader.bin

all: $(KERNEL_BIN)

$(BOOTLOADER_OBJ): $(BOOTLOADER_SRC)
	$(AS) $(ASFLAGS) $< -o $@

$(KERNEL_OBJ): $(KERNEL_SRC) kernel.h
	$(CC) $(CFLAGS) $< -o $@

$(KERNEL_BIN): $(BOOTLOADER_OBJ) $(KERNEL_OBJ)
	$(LD) $(LDFLAGS) -o $@ $^

$(BOOTLOADER_BIN): $(BOOTLOADER_OBJ)
	dd if=$(BOOTLOADER_OBJ) of=$@ bs=512 seek=4

run: $(KERNEL_BIN)
	qemu-system-i386 -kernel $(KERNEL_BIN)

debug: $(KERNEL_BIN)
	qemu-system-i386 -kernel $(KERNEL_BIN) -S -s

iso: $(BOOTLOADER_BIN) $(KERNEL_BIN)
	mkdir -p $(ISO_DIR)/boot/grub
	cp $(BOOTLOADER_BIN) $(ISO_DIR)/boot/
	cp $(KERNEL_BIN) $(ISO_DIR)/boot/
	echo 'set timeout=0' > $(ISO_DIR)/boot/grub/grub.cfg
	echo 'set default=0' >> $(ISO_DIR)/boot/grub/grub.cfg
	echo 'menuentry "TetrOS" {' >> $(ISO_DIR)/boot/grub/grub.cfg
	echo '  multiboot /boot/$(KERNEL_BIN)' >> $(ISO_DIR)/boot/grub/grub.cfg
	echo '  boot' >> $(ISO_DIR)/boot/grub/grub.cfg
	echo '}' >> $(ISO_DIR)/boot/grub/grub.cfg
	grub-mkrescue -o $(ISO_NAME) $(ISO_DIR)

clean:
	rm -f $(BOOTLOADER_OBJ) $(KERNEL_OBJ) $(KERNEL_BIN) $(BOOTLOADER_BIN)
	rm -rf $(ISO_DIR) $(ISO_NAME)