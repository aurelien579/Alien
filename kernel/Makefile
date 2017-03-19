export INCLUDE_DIR=$(shell pwd)/include

include ../make.conf
include make.conf

MODULES 	= memory
LIBS		= $(foreach m, $(MODULES), -l$(m))
SEARCH_DIR 	= $(foreach m, $(MODULES), -L$(m)/)
LD 			= ld
LFLAGS 		= -melf_i386 $(SEARCH_DIR) $(LIBS)

OBJECTS = \
	boot/loader.o \
	boot/kernel.o \
	boot/gdt.o \
	boot/gdt_asm.o \
	boot/idt.o \
	boot/idt_asm.o \
	boot/task_asm.o \
	boot/vm86.o \
	boot/task.o \
	lib/io.o \
	lib/printf.o \
	lib/string.o \
	core/vga.o \
	devices/console.o

$(KERNEL_OUT): $(OBJECTS) linker.ld modules
	$(LD) -T linker.ld -o $@  $(OBJECTS) $(LFLAGS)

modules:
	$(foreach m, $(MODULES), cd $(m) && make)

%.o: %.c modules
	$(CC) $(CFLAGS) -o $@ -c $<

%.o: %.asm modules
	$(AS) $(AFLAGS) -o $@ $<

clean:
	rm -f $(OBJECTS)
	rm -f $(KERNEL_OUT)
	cd memory && make clean

.PHONY: clean modules
