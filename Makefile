include make.conf

all: $(ISO_FILE)

$(ISO_FILE): kernel
	mkdir -p $(ISO_DIR)/boot/grub
	cp config/menu.lst $(ISO_DIR)/boot/grub/
	cp deps/stage2_eltorito $(ISO_DIR)/boot/grub/
	cp $(KERNEL_DIR)$(KERNEL_OUT) $(ISO_DIR)/boot/kernel.bin

	genisoimage -R                          \
			-b boot/grub/stage2_eltorito    \
			-no-emul-boot                   \
			-boot-load-size 4               \
			-A os                           \
			-input-charset utf8             \
			-quiet                          \
			-boot-info-table                \
			-o $(ISO_FILE)                  \
			$(ISO_DIR)

kernel:
	cd kernel && make $(KERNEL_OUT)

run: all
	mkdir -p log
	bochs -f config/bochs.cfg -q

clean:
	cd kernel && make clean
	rm -f -R iso
	rm -f $(ISO_FILE)

.PHONY: clean kernel

