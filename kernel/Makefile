INCLUDES	= -Iio/include -Ilib/include
SOURCES		= $(wildcard boot/*.c) \
			  $(wildcard boot/*.asm) \
			  $(wildcard lib/*.c)

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
	$(LD)  $(LFLAGS) -T linker.ld -o $@ $(OBJECTS)

%.o: %.asm
	$(ASM) $(ASMFLAGS) $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

.PHONY: clean

clean:
	@rm -f $(OBJECTS) $(TARGET)
