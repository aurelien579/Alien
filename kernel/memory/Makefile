include ../make.conf

OBJECTS := $(patsubst src/%.c, build/%.o, $(wildcard src/*.c))
OBJECTS := $(OBJECTS) $(patsubst src/%.asm, build/%.o, $(wildcard src/*.asm))

all: headers $(OBJECTS)
	ar rcs libmemory.a $(OBJECTS)

build/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

build/%.o: src/%.asm
	$(AS) $(AFLAGS) -o $@ $<

headers:
	mkdir -p $(INCLUDE_DIR)/memory
	cp -r include/ $(INCLUDE_DIR)/../

clean:
	rm -f build/*.o
	rm -f libmemory.a

.PHONY: clean headers
