_END=$'\x1b[0m
_BOLD=$'\x1b[1m
_PURPLE=$'\x1b[35m

SOURCES = ../sources
KERNEL = $(SOURCES)/core/kernel/target
EXTERN = $(SOURCES)/extern

info_kernel = /bin/echo -e "$(_PURPLE)[core/kernel]$(_END) $(_BOLD)$1$(_END)"
info_system = /bin/echo -e "$(_PURPLE)[core/system]$(_END) $(_BOLD)$1$(_END)"

# Limine AMD64 (liamd)

liamd/build:
	@ rm -rf liamd/
	@ mkdir -m 777 -p liamd/disk/
	@ $(call info_kernel,"Compiling...")
	@ make -C "$(KERNEL)/limine/amd64" kernel.elf 
	@ cp $(KERNEL)/limine/amd64/bin/kernel.elf liamd/disk/
	@ $(call info_kernel,"done.")
	@ make -C "$(EXTERN)" limine
	@ cp limine.cfg $(EXTERN)/limine/limine-bios.sys $(EXTERN)/limine/limine-bios-cd.bin $(EXTERN)/limine/limine-uefi-cd.bin liamd/disk/
	@ mkdir -p liamd/disk/EFI/BOOT
	@ cp $(EXTERN)/limine/BOOT*.EFI liamd/disk/EFI/BOOT/
	@ xorriso -as mkisofs -b limine-bios-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table \
		--efi-boot limine-uefi-cd.bin \
		-efi-boot-part --efi-boot-image --protective-msdos-label \
		liamd/disk/ -o liamd/violet.iso
	@ $(EXTERN)/limine/limine bios-install liamd/violet.iso

ovmf-amd64:
	@ make -C "$(EXTERN)" ovmf-amd64

LIAMD_QEMU_FLAGS = \
	-bios $(EXTERN)/ovmf-amd64/OVMF.fd \
	-cdrom liamd/violet.iso \
	-cpu host \
	-machine q35 \
	-m 2G \
	-smp cores=4 \
	-serial stdio \
	-enable-kvm \
	-no-reboot \
	-no-shutdown

liamd/run: liamd/build ovmf-amd64
	qemu-system-x86_64 $(LIAMD_QEMU_FLAGS)

liamd/debug: liamd/build ovmf-amd64
	qemu-system-x86_64 $(LIAMD_QEMU_FLAGS) -S -s

# Limine AARCH64 (liarm)

liarm/build:
	@ rm -rf liarm/
	@ mkdir -m 777 -p liarm/disk/
	@ $(call info_kernel,"Compiling...")
	@ make -C "$(KERNEL)/limine/aarch64" kernel.elf 
	@ cp $(KERNEL)/limine/aarch64/bin/kernel.elf liarm/disk/
	@ $(call info_kernel,"done.")
	@ make -C "$(EXTERN)" limine
	@ cp limine.cfg $(EXTERN)/limine/limine-bios.sys $(EXTERN)/limine/limine-bios-cd.bin $(EXTERN)/limine/limine-uefi-cd.bin liarm/disk/
	@ mkdir -p liarm/disk/EFI/BOOT
	@ cp $(EXTERN)/limine/BOOT*.EFI liarm/disk/EFI/BOOT/
	@ xorriso -as mkisofs -b limine-bios-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table \
		--efi-boot limine-uefi-cd.bin \
		-efi-boot-part --efi-boot-image --protective-msdos-label \
		liarm/disk/ -o liarm/violet.iso
	@ $(EXTERN)/limine/limine bios-install liarm/violet.iso

ovmf-aarch64:
	@ make -C "$(EXTERN)" ovmf-aarch64

LIARM_QEMU_FLAGS = \
	-bios $(EXTERN)/ovmf-aarch64/OVMF.fd \
	-cdrom liarm/violet.iso \
	-cpu cortex-a76 \
	-machine virt \
	-m 2G \
	-smp cores=4 \
	-serial stdio \
	-device ramfb \
	-no-reboot \
	-no-shutdown

liarm/run: liarm/build ovmf-aarch64
	qemu-system-aarch64 $(LIARM_QEMU_FLAGS)

liarm/debug: liarm/build ovmf-aarch64
	qemu-system-aarch64 $(LIARM_QEMU_FLAGS) -S -s
