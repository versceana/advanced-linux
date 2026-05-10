#!/usr/bin/env python3
import sys, hashlib

hwid = sys.argv[1] if len(sys.argv) > 1 else "54060500FFFB8B1F"
digest = hashlib.md5(hwid.encode()).digest()
key = ''.join(f'{b:02x}' for b in reversed(digest))   # 15-i
print(key)
