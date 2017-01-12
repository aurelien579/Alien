MODULE_NAME= $(shell cat module_name)
LIB_FILE = lib$(MODULE_NAME).a

OBJECTS = \
	io.o \
	string.o

CC = gcc
CFLAGS = -m32 -ffreestanding -nostdlib -fno-stack-protector \
		 -nostartfiles -nodefaultlibs -Wall -Wextra -I../../include/


$(LIB_FILE): $(OBJECTS)
	ar rcs $(LIB_FILE) $(OBJECTS)

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

%.o: %.asm
	$(AS) $(AFLAGS) -o $@ $<

install: $(LIB_FILE)
	cp $(LIB_FILE) $(MOSS_BIN_DIR)/

clean:
	rm -f $(OBJECTS)
	rm -f $(LIB_FILE)
	rm -f $(MOSS_BIN_DIR)/$(LIB_FILE)
