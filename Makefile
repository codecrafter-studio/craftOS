OBJS = out/kernel.o out/common.o out/monitor.o out/main.o out/gdtidt.o out/nasmfunc.o out/isr.o  \
	 out/interrupt.o out/string.o out/timer.o out/memory.o out/mtask.o out/keyboard.o out/keymap.o \
	 out/fifo.o out/syscall.o out/syscall_impl.o out/stdio.o out/kstdio.o out/hd.o out/fat16.o \
	 out/cmos.o out/file.o out/exec.o out/elf.o out/ansi.o

LIBC_OBJECTS = out/syscall_impl.o out/stdio.o out/string.o out/malloc.o

APPS = out/testc.bin out/csh.bin out/c4.bin out/colorful.bin out/cat.bin out/ver.bin out/help.bin \
	 out/echo.bin

define print_target
    @echo "Running $@..."
endef

out/%.bin : apps/%.asm
	$(call print_target)
	@nasm apps/$*.asm -o out/$*.o -f elf
	@i686-elf-ld -s -Ttext 0x0 -o out/$*.bin out/$*.o
	@echo "Built $@ successfully"

out/tulibc.a : $(LIBC_OBJECTS)
	$(call print_target)
	@i686-elf-ar rcs out/tulibc.a $(LIBC_OBJECTS)
	@echo "Built out/tulibc.a"

out/%.bin : apps/%.c apps/start.c out/tulibc.a
	$(call print_target)
	@i686-elf-gcc -c -I include apps/start.c -o out/start.o -fno-builtin
	@i686-elf-gcc -c -I include apps/$*.c -o out/$*.o -fno-builtin
	@i686-elf-ld -s -Ttext 0x0 -o out/$*.bin out/$*.o out/start.o out/tulibc.a
	@echo "Built $@ successfully"

out/%.o : kernel/%.c
	$(call print_target)
	@i686-elf-gcc -c -I include -O0 -fno-builtin -fno-stack-protector -o out/$*.o kernel/$*.c
	@echo "Built out/$*.o for kernel"

out/%.o : kernel/%.asm
	$(call print_target)
	@nasm -f elf -o out/$*.o kernel/$*.asm
	@echo "Built out/$*.o from assembly"

out/%.o : lib/%.c
	$(call print_target)
	@i686-elf-gcc -c -I include -O0 -fno-builtin -fno-stack-protector -o out/$*.o lib/$*.c
	@echo "Built out/$*.o for lib"

out/%.o : lib/%.asm
	$(call print_target)
	@nasm -f elf -o out/$*.o lib/$*.asm
	@echo "Built out/$*.o from assembly (lib)"

out/%.o : drivers/%.c
	$(call print_target)
	@i686-elf-gcc -c -I include -O0 -fno-builtin -fno-stack-protector -o out/$*.o drivers/$*.c
	@echo "Built out/$*.o for drivers"

out/%.o : drivers/%.asm
	$(call print_target)
	@nasm -f elf -o out/$*.o drivers/$*.asm
	@echo "Built out/$*.o from assembly (drivers)"

out/%.o : fs/%.c
	$(call print_target)
	@i686-elf-gcc -c -I include -O0 -fno-builtin -fno-stack-protector -o out/$*.o fs/$*.c
	@echo "Built out/$*.o for filesystem"

out/%.o : fs/%.asm
	$(call print_target)
	@nasm -f elf -o out/$*.o fs/$*.asm
	@echo "Built out/$*.o from assembly (filesystem)"

out/%.bin : boot/%.asm
	$(call print_target)
	@nasm -I boot/include -o out/$*.bin boot/$*.asm
	@echo "Built bootloader $@"

out/kernel.bin : $(OBJS)
	$(call print_target)
	@i686-elf-ld -s -Ttext 0x100000 -o out/kernel.bin $(OBJS)
	@echo "Linked kernel"

build/hd.img : out/boot.bin out/loader.bin out/kernel.bin $(APPS)
	$(call print_target)
	@fat/ftimage build/hd.img -size 5 -bs out/boot.bin
	@fat/ftcopy build/hd.img -srcpath out/loader.bin -to -dstpath /loader.cxp
	@fat/ftcopy build/hd.img -srcpath out/kernel.bin -to -dstpath /kernel.cxp
	@fat/ftcopy build/hd.img -srcpath out/testc.bin -to -dstpath /testc.cxp
	@fat/ftcopy build/hd.img -srcpath out/csh.bin -to -dstpath /csh.cxp
	@fat/ftcopy build/hd.img -srcpath out/colorful.bin -to -dstpath /colorful.cxp
	@fat/ftcopy build/hd.img -srcpath out/c4.bin -to -dstpath /c4.cxp
	@fat/ftcopy build/hd.img -srcpath out/cat.bin -to -dstpath /cat.cxp
	@fat/ftcopy build/hd.img -srcpath apps/testc.c -to -dstpath /testc.c
	@fat/ftcopy build/hd.img -srcpath out/ver.bin -to -dstpath /ver.cxp
	@fat/ftcopy build/hd.img -srcpath out/help.bin -to -dstpath /help.cxp
	@fat/ftcopy build/hd.img -srcpath out/echo.bin -to -dstpath /echo.cxp
	@echo "Created img file: build/hd.img"

run : build/hd.img
	@qemu-system-i386 -hda build/hd.img

clean :
	$(call print_target)
	@rm -rf out/* build/*
	@echo "Cleaned successfully"

all : clean run

default : build/hd.img

.PHONY : default
