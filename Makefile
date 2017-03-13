include make.conf

all: $(ISO_FILE)
all_old: $(ISO_FILE)_old

$(ISO_FILE): kernel
	cp kernel/kernel.bin $(ISO_DIR)/boot/kernel.bin
	sudo grub-mkrescue iso -o $(ISO_FILE) -d /usr/lib/grub/i386-pc

$(ISO_FILE)_old: kernel
	mkdir -p $(ISO_DIR)/boot/grub
	cp config/menu.lst $(ISO_DIR)/boot/grub/
	cp deps/stage2_eltorito $(ISO_DIR)/boot/grub/
	cp $(KERNEL_DIR)$(KERNEL_OUT) $(ISO_DIR)/boot/kernel.bin

	xorriso -as mkisofs                     \
			-b boot/grub/stage2_eltorito 	\
			-no-emul-boot                   \
			-boot-load-size 4               \
			-boot-info-table                \
			--eltorito-alt-boot				\
			-e efi/grub.efi	 				\
			-no-emul-boot                   \
			-isohybrid-gpt-basdat			\
			-o $(ISO_FILE)                  \
			$(ISO_DIR)

kernel:
	cd kernel && make $(KERNEL_OUT)

run_old: all
	mkdir -p log
	bochs -f config/bochs_old.cfg -q

run: all
	mkdir -p log
	bochs -f config/bochs.cfg -q

clean:
	cd kernel && make clean
	rm -f -R iso
	rm -f $(ISO_FILE)

.PHONY: clean kernel

