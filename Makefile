OBJS = out/kernel.o out/common.o out/monitor.o out/main.o out/gdtidt.o out/nasmfunc.o out/isr.o out/interrupt.o \
     out/string.o out/timer.o out/memory.o out/mtask.o out/keyboard.o out/keymap.o out/fifo.o out/syscall.o out/syscall_impl.o \
     out/stdio.o out/kstdio.o out/hd.o out/fat16.o out/cmos.o out/file.o out/exec.o out/elf.o

LIBC_OBJECTS = out/syscall_impl.o out/stdio.o out/string.o out/malloc.o

APPS = out/test_c.bin out/shell.bin out/c4.bin

out/%.bin : apps/%.asm
	nasm apps/$*.asm -o out/$*.o -f elf
	i686-elf-ld -s -Ttext 0x0 -o out/$*.bin out/$*.o

out/tulibc.a : $(LIBC_OBJECTS)
	i686-elf-ar rcs out/tulibc.a $(LIBC_OBJECTS)

out/%.bin : apps/%.c apps/start.c out/tulibc.a
	i686-elf-gcc -c -I include apps/start.c -o out/start.o -fno-builtin
	i686-elf-gcc -c -I include apps/$*.c -o out/$*.o -fno-builtin
	i686-elf-ld -s -Ttext 0x0 -o out/$*.bin out/$*.o out/start.o out/tulibc.a

out/%.o : kernel/%.c
	i686-elf-gcc -c -I include -O0 -fno-builtin -fno-stack-protector -o out/$*.o kernel/$*.c

out/%.o : kernel/%.asm
	nasm -f elf -o out/$*.o kernel/$*.asm

out/%.o : lib/%.c
	i686-elf-gcc -c -I include -O0 -fno-builtin -fno-stack-protector -o out/$*.o lib/$*.c

out/%.o : lib/%.asm
	nasm -f elf -o out/$*.o lib/$*.asm

out/%.o : drivers/%.c
	i686-elf-gcc -c -I include -O0 -fno-builtin -fno-stack-protector -o out/$*.o drivers/$*.c

out/%.o : drivers/%.asm
	nasm -f elf -o out/$*.o drivers/$*.asm

out/%.o : fs/%.c
	i686-elf-gcc -c -I include -O0 -fno-builtin -fno-stack-protector -o out/$*.o fs/$*.c

out/%.o : fs/%.asm
	nasm -f elf -o out/$*.o fs/$*.asm

out/%.bin : boot/%.asm
	nasm -I boot/include -o out/$*.bin boot/$*.asm

out/kernel.bin : $(OBJS)
	i686-elf-ld -s -Ttext 0x100000 -o out/kernel.bin $(OBJS)

hd.img : out/boot.bin out/loader.bin out/kernel.bin $(APPS)
	ftimage hd.img -size 80 -bs out/boot.bin
	ftcopy hd.img -srcpath out/loader.bin -to -dstpath /loader.bin 
	ftcopy hd.img -srcpath out/kernel.bin -to -dstpath /kernel.bin
	ftcopy hd.img -srcpath out/test_c.bin -to -dstpath /test_c.bin
	ftcopy hd.img -srcpath out/test2.bin -to -dstpath /test2.bin
	ftcopy hd.img -srcpath out/shell.bin -to -dstpath /shell.bin
	ftcopy hd.img -srcpath out/c4.bin -to -dstpath /c4.bin
	ftcopy hd.img -srcpath apps/test_c.c -to -dstpath /test_c.c
	ftcopy hd.img -srcpath apps/c4.c -to -dstpath /c4.c

run : hd.img
	qemu-system-i386 -hda hd.img

clean :
	cmd /c del /f /s /q out

default : clean run