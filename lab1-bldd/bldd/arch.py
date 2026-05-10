"""Architecture mappings and helpers for ELF file handling."""

def machine_to_arch(machine: int) -> str:
    machine_map = {
        3: 'x86',          # EM_386
        62: 'x86_64',      # EM_X86_64
        40: 'armv7',       # EM_ARM
        183: 'aarch64',    # EM_AARCH64
    }
    return machine_map.get(machine, f'unknown_{machine}')


def get_supported_architectures() -> dict:
    return {
        'x86': 3,
        'x86_64': 62,
        'armv7': 40,
        'aarch64': 183,
    }
