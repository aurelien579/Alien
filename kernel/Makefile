include ../make.conf

CC = gcc
CFLAGS = -m32 -ffreestanding -nostdlib -fno-stack-protector \
		 -nostartfiles -nodefaultlibs -Wall -Wextra

AS = nasm
AFLAGS = -f elf32

LD = ld
LFLAGS = -melf_i386

OBJECTS = \
	boot/loader.o \
	boot/kernel.o

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
