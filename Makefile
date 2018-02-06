ISO_DIR		= iso
ISO_FILE	= alien-os.iso

all: $(ISO_FILE)

$(ISO_FILE): kernel/kernel.bin
	@mkdir -p iso/boot/grub
	@cp kernel/kernel.bin $(ISO_DIR)/boot/kernel.bin
	@cp config/grub.cfg iso/boot/grub/
	@sudo grub-mkrescue iso -o $(ISO_FILE) -d /usr/lib/grub/i386-pc

run: $(ISO_FILE)
	@mkdir -p log
	@bochs -f config/bochs.cfg -q
	
kernel/kernel.bin:
	@cd kernel && $(MAKE)

clean:
	@cd kernel && $(MAKE) clean
	@rm -Rf $(ISO_DIR) log/
	@rm -f $(ISO_FILE)

.PHONY: clean run kernel/kernel.bin
