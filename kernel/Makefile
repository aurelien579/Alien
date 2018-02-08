INCLUDES	= -Iio/include -Ilib/include -Iboot/include -Icpu/include \
			  -Idebug/include -Imemory/include -Ikernel/include

SOURCES		= $(wildcard boot/*.c) \
			  $(wildcard boot/*.asm) \
			  $(wildcard lib/*.c) \
			  $(wildcard debug/*.c) \
			  $(wildcard cpu/*.c) \
			  $(wildcard cpu/*.asm) \
			  $(wildcard memory/*.c) \
			  $(wildcard memory/*.asm) \
			  $(wildcard kernel/*.c) \

OBJECTS		= $(patsubst %.c, %.o, $(patsubst %.asm, %.o, $(SOURCES)))

LD			= i686-elf-ld
LFLAGS		=
ASM			= nasm
ASMFLAGS	= -f elf32
CC			= i686-elf-gcc
CFLAGS		= -ffreestanding -Wall -Wextra $(INCLUDES)
TARGET		= kernel.bin

all: $(TARGET)

$(TARGET): $(OBJECTS) linker.ld
	@echo 'Linking $@'
	@$(LD)  $(LFLAGS) -T linker.ld -o $@ $(OBJECTS)

%.o: %.asm
	@echo 'Compiling $<'
	@$(ASM) $(ASMFLAGS) $< -o $@

%.o: %.c
	@echo 'Compiling $<'
	@$(CC) $(CFLAGS) -o $@ -c $<

.PHONY: clean

clean:
	@rm -f $(OBJECTS) $(TARGET)
