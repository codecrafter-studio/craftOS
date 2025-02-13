OBJS = out/kernel.o out/common.o out/monitor.o out/main.o out/gdtidt.o out/nasmfunc.o out/isr.o  \
	 out/interrupt.o out/string.o out/timer.o out/memory.o out/mtask.o out/keyboard.o out/keymap.o \
	 out/fifo.o out/syscall.o out/syscall_impl.o out/stdio.o out/kstdio.o out/hd.o out/fat16.o \
	 out/cmos.o out/file.o out/exec.o out/elf.o out/ansi.o

LIBC_OBJECTS = out/syscall_impl.o out/stdio.o out/string.o out/malloc.o

APPS = out/testc.bin out/csh.bin out/c4.bin out/colorful.bin out/cat.bin out/ver.bin out/help.bin \
	 out/echo.bin

out/%.bin : apps/%.asm
	@nasm apps/$*.asm -o out/$*.o -f elf
	@i686-elf-ld -s -Ttext 0x0 -o out/$*.bin out/$*.o
	@echo "Built $@.bin"

out/tulibc.a : $(LIBC_OBJECTS)
	@i686-elf-ar rcs out/tulibc.a $(LIBC_OBJECTS)
	@echo "Built out/tulibc.a"

out/%.bin : apps/%.c apps/start.c out/tulibc.a
	@i686-elf-gcc -c -I include apps/start.c -o out/start.o -fno-builtin
	@i686-elf-gcc -c -I include apps/$*.c -o out/$*.o -fno-builtin
	@i686-elf-ld -s -Ttext 0x0 -o out/$*.bin out/$*.o out/start.o out/tulibc.a
	@echo "Built $@.bin"

out/%.o : kernel/%.c
	@i686-elf-gcc -c -I include -O0 -fno-builtin -fno-stack-protector -o out/$*.o kernel/$*.c
	@echo "Built out/$*.o"

out/%.o : kernel/%.asm
	@nasm -f elf -o out/$*.o kernel/$*.asm
	@echo "Built out/$*.o"

out/%.o : lib/%.c
	@i686-elf-gcc -c -I include -O0 -fno-builtin -fno-stack-protector -o out/$*.o lib/$*.c
	@echo "Built out/$*.o"

out/%.o : lib/%.asm
	@nasm -f elf -o out/$*.o lib/$*.asm
	@echo "Built out/$*.o"

out/%.o : drivers/%.c
	@i686-elf-gcc -c -I include -O0 -fno-builtin -fno-stack-protector -o out/$*.o drivers/$*.c
	@echo "Built out/$*.o"

out/%.o : drivers/%.asm
	@nasm -f elf -o out/$*.o drivers/$*.asm
	@echo "Built out/$*.o"

out/%.o : fs/%.c
	@i686-elf-gcc -c -I include -O0 -fno-builtin -fno-stack-protector -o out/$*.o fs/$*.c
	@echo "Built out/$*.o"

out/%.o : fs/%.asm
	@nasm -f elf -o out/$*.o fs/$*.asm
	@echo "Built out/$*.o"

out/%.bin : boot/%.asm
	@nasm -I boot/include -o out/$*.bin boot/$*.asm
	@echo "Built bootloader $@.bin"

out/kernel.bin : $(OBJS)
	@i686-elf-ld -s -Ttext 0x100000 -o out/kernel.bin $(OBJS)
	@echo "Linked kernel"

build/hd.img : out/boot.bin out/loader.bin out/kernel.bin $(APPS)
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
	@rm -rf out/* build/*
	@echo "Cleaned successfully"

all : run

default : clean run

.PHONY : run
