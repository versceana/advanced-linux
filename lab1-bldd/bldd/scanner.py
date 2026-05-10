"""ELF file scanning and dependency extraction."""

import os
from pathlib import Path
from typing import Dict, List

from elftools.elf.elffile import ELFFile
from elftools.common.exceptions import ELFError

from .arch import machine_to_arch

def is_executable(elf: ELFFile, filepath: Path) -> bool:

    etype = elf.header.e_type

    if etype == "ET_EXEC":
        return True

    if etype == "ET_DYN":
        return any(
            segment.header.p_type == "PT_INTERP"
            for segment in elf.iter_segments()
        )

    return False

def get_dependencies(elf: ELFFile) -> List[str]:
    deps = []
    dynsec = elf.get_section_by_name('.dynamic')
    if dynsec:
        for tag in dynsec.iter_tags():
            if tag.entry.d_tag == 'DT_NEEDED':
                # tag.needed gives us the string name directly
                deps.append(tag.needed)
    return deps


def scan_directory(scan_dir: Path, libs: List[str], debug: bool = False) -> Dict[str, Dict[str, List[str]]]:
    results: Dict[str, Dict[str, List[str]]] = {}
    files_processed = 0
    elf_files = 0
    matched_files = 0

    for root, dirs, files in os.walk(scan_dir):
        for file in files:
            filepath = Path(root) / file
            files_processed += 1
            try:
                with open(filepath, 'rb') as f:
                    elf = ELFFile(f)
                    elf_files += 1
                    
                    if not is_executable(elf, filepath):
                        continue
                    
                    arch = machine_to_arch(elf.header.e_machine)
                    deps = get_dependencies(elf)
                    
                    if debug and files_processed <= 5:
                        print(f"[DEBUG] {filepath.name}: e_machine={elf.header.e_machine}, arch={arch}, deps={deps[:3]}...")
                    
                    for lib in libs:
                        if lib in deps:
                            matched_files += 1
                            if arch not in results:
                                results[arch] = {}
                            if lib not in results[arch]:
                                results[arch][lib] = []
                            results[arch][lib].append(str(filepath))
            except (ELFError, OSError):
                continue
    
    if debug:
        print(f"[DEBUG] Processed {files_processed} files, {elf_files} ELF, {matched_files} matched")

    return results


def sort_results(results: Dict[str, Dict[str, List[str]]]) -> Dict[str, List[tuple[str, int, List[str]]]]:
    sorted_results = {}
    for arch in sorted(results.keys()):
        lib_counts = []
        for lib, execs in results[arch].items():
            lib_counts.append((lib, len(execs), execs))
        lib_counts.sort(key=lambda x: x[1], reverse=True)
        sorted_results[arch] = lib_counts
    return sorted_results
