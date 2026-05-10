#!/usr/bin/env python3
import sys, os

if len(sys.argv) != 2:
    print(f"Usage: {sys.argv[0]} <binary>")
    sys.exit(1)

bin_path = sys.argv[1]
offset = 0x15fc  # file offset of jne instruction

# Оригинальные байты: 75 33 (jne +0x33)
# Патч: EB 00 (jmp +2, безусловный переход на следующую инструкцию)
patch = b'\xEB\x00'

with open(bin_path, 'r+b') as f:
    f.seek(offset)
    orig = f.read(2)
    if orig != b'\x75\x33':
        print(f"ERROR: unexpected bytes at {offset:#x}: {orig.hex()} != 7533")
        sys.exit(1)
    f.seek(offset)
    f.write(patch)
    print(f"Patched {bin_path} at offset {offset:#x}: {orig.hex()} -> {patch.hex()}")
