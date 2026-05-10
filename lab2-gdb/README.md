## Overview

This is the second lab assignment:  
reverse engineer the license validation logic of a native Linux binary and
produce two artifacts:

- **Keygen** - generates a valid license key for any hardware ID (HWID)
- **Binary patch** - modifies the executable to bypass the license check entirely

## Repository structure

```
.
├── src/
│   ├── keygen.py              # Key generator
│   └── patch_license.py       # Binary patcher
├── artifacts/
│   ├── hack_app.tar.gz      # Original binary
│   ├── hack_app      # Original binary
│   └── hack_app_patched       # Patched binary (accepts any key)
├── screenshots/               # All demonstration screenshots
├── lab2_report.md             # Full report
└── README.md
```

## Requirements

-  Linux that use OpenSSL 1.1 for `libssl1.1` package (Ubuntu 24.04 x86_64 in this sultion)
- Python 3 (no extra modules needed)
- `attr` package (for `getfattr`/`setfattr`)

## Quick start

### 1. Keygen

```bash
python3 src/keygen.py <HWID>
```

The script computes MD5 of the 16‑character uppercase HWID, reverses the
16‑byte digest and formats each byte as two lowercase hex digits – exactly
reproducing the algorithm found during reverse engineering.

### 2. Binary patch

```bash
# Apply the patch (modifies the file in‑place)
python3 src/patch_license.py ./hack_app

# (Optional) remove any previously stored license to test the patch
setfattr -x user.license ./hack_app

# Run the patched binary – enter anything as the key
./hack_app
```

After patching, **any** entered key is accepted. The program prints
“Now you app is activated!” and writes the (fake) key into the file’s
extended attributes.

## How it works (brief)

The original binary:

1. Reads CPUID to build a 16‑char uppercase HWID
2. Computes MD5 of that HWID
3. Reverses the digest bytes and formats them as a 32‑char lowercase string
4. Compares the result against either the `user.license` xattr or user input

The keygen replicates the exact hashing and formatting steps.  
The patcher replaces a `jne` (jump if not equal) after the user‑input
comparison with an unconditional `jmp`, forcing the success path.