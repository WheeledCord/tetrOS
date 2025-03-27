NASM = nasm
GCC = gcc
LD = ld

ARCH_FLAGS = -m32 -fno-stack-protector -g

ASM_SRC = kernel.asm
C_SRC = kernel.c
ASM_OBJ = kernelasm.o
C_OBJ = kernelc.o
KERNEL_BIN = kernel
LINKER_SCRIPT = linker.ld

ISO_DIR = iso
BOOT_DIR = $(ISO_DIR)/boot
KERNEL_DST = $(BOOT_DIR)/kernel

all: $(KERNEL_BIN)

$(ASM_OBJ): $(ASM_SRC)
	$(NASM) -f elf32 $(ASM_SRC) -o $(ASM_OBJ)

$(C_OBJ): $(C_SRC)
	$(GCC) $(ARCH_FLAGS) -c $(C_SRC) -o $(C_OBJ)

$(KERNEL_BIN): $(ASM_OBJ) $(C_OBJ)
	$(LD) -m elf_i386 -T $(LINKER_SCRIPT) -o $(KERNEL_BIN) $(ASM_OBJ) $(C_OBJ)

iso: $(KERNEL_BIN)
	mkdir -p $(BOOT_DIR)
	cp $(KERNEL_BIN) $(KERNEL_DST)
	grub-mkrescue -o kernel.iso $(ISO_DIR)

clean:
	rm -f $(ASM_OBJ) $(C_OBJ) $(KERNEL_BIN)
	rm -rf $(ISO_DIR)

.PHONY: all clean iso

