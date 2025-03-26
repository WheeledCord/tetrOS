NASM = nasm
GCC = gcc
LD = ld

ARCH_FLAGS = -m32

ASM_SRC = kernel.asm
C_SRC = kernel.c
ASM_OBJ = kernelasm.o
C_OBJ = kernelc.o
KERNEL_BIN = kernel

LINKER_SCRIPT = linker.ld


all: $(KERNEL_BIN)

$(ASM_OBJ): $(ASM_SRC)
	$(NASM) -f elf32 $(ASM_SRC) -o $(ASM_OBJ)

$(C_OBJ): $(C_SRC)
	$(GCC) $(ARCH_FLAGS) -c $(C_SRC) -o $(C_OBJ)

$(KERNEL_BIN): $(ASM_OBJ) $(C_OBJ)
	$(LD) -m elf_i386 -T $(LINKER_SCRIPT) -o $(KERNEL_BIN) $(ASM_OBJ) $(C_OBJ)

clean:
	rm -f $(ASM_OBJ) $(C_OBJ) $(KERNEL_BIN)

.PHONY: all clean

