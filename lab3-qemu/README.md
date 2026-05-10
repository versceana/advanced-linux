## Overview

This is the deliverables for the third lab assignment: demonstrate the Linux kernel boot process on an ARM target (vexpress-a9) using the chain:

**Bootloader (U-Boot) → Kernel → initramfs → rootfs**

The entire setup is emulated with QEMU, showing how an initial RAM filesystem
prepares the environment and switches to a real root filesystem.

## Repository structure

```
.
├── src/
│   ├── initramfs_init      # Init script for the initial RAM disk
│   └── rootfs_init         # Init script for the final root filesystem
├── artifacts/
│   ├── zImage              # Compiled Linux kernel image
│   ├── vexpress-v2p-ca9.dtb # Device tree blob
│   ├── initramfs.cpio.gz   # gzip-compressed initramfs image
│   └── rootfs.img           # Ext4 root filesystem image
├── screenshots/            # Screenshots of key steps and boot log
├── lab3_report.md          # Full report
└── README.md
```

## Requirements

- **Host:** Linux (x86_64) with cross-compilation tools for ARM.  
  Tested on Ubuntu 24.04 with `gcc-arm-linux-gnueabihf`.
- **Emulator:** QEMU (system-arm)
- **Additional tools:** `cpio`, `gzip`, `mkfs.ext4`, `git`, `make`, `wget`

## Quick start

### 1. Clone and build U-Boot

```bash
git clone --depth 1 -b v2022.01 https://github.com/u-boot/u-boot.git
cd u-boot
export ARCH=arm
export CROSS_COMPILE=arm-linux-gnueabihf-
make vexpress_ca9x4_defconfig
make -j$(nproc)
```

### 2. Build the Linux kernel

```bash
git clone --depth 1 -b v6.1 https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git
cd linux
export ARCH=arm
export CROSS_COMPILE=arm-linux-gnueabihf-
make vexpress_defconfig
make -j$(nproc) zImage modules dtbs
```

### 3. Build BusyBox (static, for initramfs and rootfs)

```bash
git clone --depth 1 -b 1_36_stable https://git.busybox.net/busybox
cd busybox
export ARCH=arm
export CROSS_COMPILE=arm-linux-gnueabihf-
make defconfig
sed -i 's/# CONFIG_STATIC is not set/CONFIG_STATIC=y/' .config
# optional: disable problematic tc applet
sed -i 's/CONFIG_TC=y/CONFIG_TC=n/' .config
make -j$(nproc)
make install   # installs into _install/
```

### 4. Create initramfs and rootfs images

Refer to `src/initramfs_init` and `src/rootfs_init` for the init scripts.
The initramfs mounts `/proc`, `/sys`, runs `mdev`, mounts `/dev/mmcblk0` and
switches to the real rootfs via `switch_root`.

```bash
cd ~/busybox/_install
# copy appropriate init script as 'init', then:
find . | cpio -o -H newc | gzip > ~/initramfs.cpio.gz

# Create rootfs.img (ext4) with busybox and rootfs_init script
dd if=/dev/zero of=~/rootfs.img bs=1M count=32
mkfs.ext4 ~/rootfs.img
sudo mount -o loop ~/rootfs.img /mnt/rootfs
sudo cp -a ~/rootfs/* /mnt/rootfs/
sudo umount /mnt/rootfs
```

### 5. Boot in QEMU

```bash
qemu-system-arm -M vexpress-a9 -m 512M \
  -kernel ~/linux/arch/arm/boot/zImage \
  -dtb ~/linux/arch/arm/boot/dts/vexpress-v2p-ca9.dtb \
  -initrd ~/initramfs.cpio.gz \
  -sd ~/rootfs.img \
  -nographic \
  -append "console=ttyAMA0 root=/dev/mmcblk0 rw"
```

Inside the VM, verify with `cat /proc/cmdline` and `uname -a`.