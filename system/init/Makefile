CC = gcc
CFLAGS = -m32 -ffreestanding -nostdlib -fno-stack-protector \
		 -nostartfiles -nodefaultlibs -Wall -Wextra -I../kernel/include

OBJECTS = \
	build/main.o \

all: init

init: $(OBJECTS)
	$(CC) -o $@ $(CFLAGS) $(OBJECTS)

build/%.o: src/%.c
	$(CC) $(CFLAGS) -o $@ -c $<
	
clean:
	rm build/*