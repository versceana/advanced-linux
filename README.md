# Advanced Linux Course

Systems programming labs covering ELF binaries, reverse engineering, kernel compilation, and kernel modules, and USB gadgets.

## [Lab 1: bldd](./lab1-bldd/)
Python tool that finds executable ELF files using specific shared libraries (reverse `ldd`). Recursive directory scanning with ELF header parsing.

## [Lab 2: gdb](./lab2-gdb/)
Binary reverse engineering and patching. Defeating license checks using GDB, strace, keygen, and binary patching.

## [Lab 3: qemu](./lab3-qemu/)
Building complete Linux system from scratch: U-Boot bootloader, Linux kernel for ARM (vexpress-a9), minimal initramfs/rootfs, booted in QEMU.

## [Lab 4: kmod](./lab4-kmod/)
Kernel module implementing thread-safe integer stack as character device. Dynamic memory management, ioctl operations, userspace CLI tool.

## [Lab 5: usb](./lab5-usb/)
USB gadget configuration on Linux. CDC ACM serial device emulation via configfs, gadget driver setup, and device enumeration.

---

Each lab includes source code, artifacts, detailed report with screenshots.

**Diana Yakupova**
