OBJS = out/kernel.o out/common.o out/monitor.o out/main.o out/gdtidt.o out/nasmfunc.o out/isr.o out/interrupt.o \
     out/string.o out/timer.o out/memory.o out/mtask.o out/keyboard.o out/keymap.o out/fifo.o out/syscall.o out/syscall_impl.o \
     out/stdio.o out/kstdio.o out/shell.o out/hd.o out/fat16.o out/cmos.o out/file.o out/exec.o

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

hd.img : out/boot.bin out/loader.bin out/kernel.bin
	ftimage hd.img -size 80 -bs out/boot.bin
	ftcopy hd.img -srcpath out/loader.bin -to -dstpath /loader.bin
	ftcopy hd.img -srcpath out/kernel.bin -to -dstpath /kernel.bin

run : hd.img
	qemu-system-i386 -hda hd.img -boot a

clean :
	cmd /c del /f /s /q out

default : clean run