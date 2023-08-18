#!/bin/bash

EXTERN=../sources/extern

# Create virtual HDD file
dd if=/dev/zero bs=1M count=0 seek=64 of=liamd/boot-violet.hdd

# Configure GPT partition table
parted -s liamd/boot-violet.hdd mklabel gpt
parted -s liamd/boot-violet.hdd mkpart ESP fat32 2048s 100%
parted -s liamd/boot-violet.hdd set 1 esp on

# Install Limine in the virtual HDD file
${EXTERN}/limine/limine bios-install liamd/boot-violet.hdd

# Set up loopback device for the HDD file
LOOPBACK=$(sudo losetup -Pf --show liamd/boot-violet.hdd)

# Format partition as FAT32 filesystem
sudo mkfs.fat -F 32 ${LOOPBACK}p1

# Mount the partition
mkdir -p boot-violet_mount
sudo mount ${LOOPBACK}p1 boot-violet_mount

# Copy necessary files
sudo mkdir -p boot-violet_mount/EFI/BOOT
sudo cp -r liamd/flash/. boot-violet_mount/.

# Sync data
sync

# Unmount the partition
sudo umount boot-violet_mount

# Release loopback device
sudo losetup -d ${LOOPBACK}

# Remove mount folder
sudo rm -rf boot-violet_mount
