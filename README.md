### delete this later
make iso with `make iso`
run iso with `qemu-system-i386 -cdrom tetros.iso`



# tetrOS

tetrOS is a simple, bootable OS designed to run a game of Tetris.

---

## **1. Installing Dependencies**

### **Arch Linux**
Install the required packages using `pacman`:
```bash
sudo pacman -S --needed base-devel qemu nasm
```
For musl cross-compilation:
```bash
sudo pacman -S --needed musl musl-dev musl-tools
```

### **Ubuntu/Debian**
Install required packages using `apt`:
```bash
sudo apt update
sudo apt install build-essential qemu-system-x86 nasm musl-tools
```
For musl cross-compilation:
```bash
sudo apt install musl musl-dev
```

---

## **2. Installing `i686-linux-musl-gcc`**

If `i686-linux-musl-gcc` is not available on your system, you can either **build it manually** or **use a prebuilt version**.

### **Method 1: Use Prebuilt Musl Cross Compiler (Recommended)**
1. **Download the prebuilt compiler**:
   ```bash
   wget https://musl.cc/i686-linux-musl-cross.tgz
   ```
2. **Extract it**:
   ```bash
   tar -xvzf i686-linux-musl-cross.tgz
   ```
3. **Add it to PATH**:
   ```bash
   export PATH=$PWD/i686-linux-musl-cross/bin:$PATH
   ```
4. **Verify the installation**:
   ```bash
   i686-linux-musl-gcc --version
   ```
5. **Make it permanent** (optional):
   ```bash
   echo 'export PATH=$HOME/i686-linux-musl-cross/bin:$PATH' >> ~/.bashrc
   source ~/.bashrc
   ```

### **Method 2: Build `i686-linux-musl-gcc` Manually**
1. **Install required dependencies**:
   ```bash
   sudo apt update
   sudo apt install build-essential wget musl musl-dev musl-tools gcc-multilib
   ```
2. **Clone the Musl cross-compiler repository**:
   ```bash
   git clone https://github.com/richfelker/musl-cross-make.git
   cd musl-cross-make
   ```
3. **Configure the build**:
   ```bash
   nano config.mak
   ```
   Find `TARGET` and set it to:
   ```
   TARGET = i686-linux-musl
   ```
4. **Build the toolchain**:
   ```bash
   make install
   ```
5. **Add it to PATH**:
   ```bash
   export PATH=$HOME/musl-cross-make/output/bin:$PATH
   ```
6. **Verify the installation**:
   ```bash
   i686-linux-musl-gcc --version
   ```

---

## **3. Compiling tetrOS**

~~### **Step 1: Assemble `bootloader.asm`**~~
~~Convert the assembly startup code into an object file:~~
```bash
nasm -f elf32 bootloader.asm -o bootloader_asm.o
```

~~### **Step 2: Compile `kernel.c`**~~
~~Compile the kernel C code with **musl** and `i686` architecture:~~
```bash
i686-linux-musl-gcc -ffreestanding -m32 -c kernel.c -o kernel_c.o
```

~~### **Step 3: Link Everything**~~
~~Link the object files into an **executable kernel**:~~
```bash
i686-linux-musl-ld -T linker.ld -o kernel.elf bootloader_asm.o kernel_c.o
```
nvm just run ``make`` to build and ``make run`` to run

---

## **4. Running tetrOS in QEMU**
Run the compiled kernel using QEMU:
```bash
qemu-system-i386 -kernel kernel.elf
```

To debug with GDB:
```bash
qemu-system-i386 -kernel kernel.elf -S -s
```
Then, in another terminal:
```bash
gdb -ex "target remote localhost:1234" kernel.elf
```

---

## **5. Contributing**
Feel free to contribute by submitting issues and pull requests.

---

## **6. License**
tetrOS is open-source and released under the **MIT License**.
