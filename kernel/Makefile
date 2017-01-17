include ../make.conf

CC = gcc
CFLAGS = -m32 -ffreestanding -nostdlib -fno-stack-protector \
		 -nostartfiles -nodefaultlibs -Wall -Wextra -Iinclude

AS = nasm
AFLAGS = -f elf32

LD = ld
LFLAGS = -melf_i386

OBJECTS = \
	boot/loader.o \
	boot/kernel.o \
	boot/gdt.o \
	boot/gdt_asm.o \
	boot/idt.o \
	boot/idt_asm.o \
	boot/console.o \
	boot/paging.o \
	lib/io.o \
	lib/printf.o \
	lib/string.o

$(KERNEL_OUT): $(OBJECTS) linker.ld
	$(LD) -T linker.ld -o $@ $(LFLAGS) $(OBJECTS)

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

%.o: %.asm
	$(AS) $(AFLAGS) -o $@ $<

clean:
	rm -f $(OBJECTS)
	rm -f $(KERNEL_OUT)

.PHONY: clean
