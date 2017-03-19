include make.conf

all: $(ISO_FILE)

$(ISO_FILE): kernel
	@mkdir -p iso/boot/grub
	@cp kernel/kernel.bin $(ISO_DIR)/boot/kernel.bin
	@cp config/grub.cfg iso/boot/grub/
	@sudo grub-mkrescue iso -o $(ISO_FILE) -d /usr/lib/grub/i386-pc

kernel:
	@cd kernel && make $(KERNEL_OUT)

run: all
	@mkdir -p log
	@bochs -f config/bochs.cfg -q

clean:
	@cd kernel && make clean
	@rm -f -R iso
	@rm -f $(ISO_FILE)

.PHONY: clean kernel

