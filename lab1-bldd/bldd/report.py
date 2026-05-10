"""Report generation in TXT and PDF formats."""

from pathlib import Path
from typing import Dict, List

from reportlab.lib.pagesizes import letter
from reportlab.lib.styles import getSampleStyleSheet
from reportlab.platypus import SimpleDocTemplate, Paragraph, Spacer


def generate_report(
    scan_dir: Path,
    libs: List[str],
    sorted_results: Dict[str, List[tuple[str, int, List[str]]]],
    output: Path,
    format: str = 'txt'
):
    """Generate report in specified format (TXT or PDF).
    
    Args:
        scan_dir: Directory that was scanned
        libs: Libraries that were searched for
        sorted_results: Sorted scan results from scanner.sort_results()
        output: Path to output file
        format: Report format ('txt' or 'pdf')
    """
    if format == 'txt':
        content = generate_txt_report(scan_dir, libs, sorted_results)
        output.write_text(content, encoding='utf-8')
    elif format == 'pdf':
        generate_pdf_report(scan_dir, libs, sorted_results, output)
    else:
        raise ValueError(f"Unsupported format: {format}")


def generate_txt_report(
    scan_dir: Path,
    libs: List[str],
    sorted_results: Dict[str, List[tuple[str, int, List[str]]]]
) -> str:
    """Generate text report content.
    
    Args:
        scan_dir: Directory that was scanned
        libs: Libraries that were searched for
        sorted_results: Sorted scan results
        
    Returns:
        Formatted text report as string
    """
    lines = []
    lines.append(f"Report on dynamic used libraries by ELF executables on {scan_dir}")
    lines.append("")

    for arch, lib_counts in sorted_results.items():
        lines.append(f"---------- {arch} -----------")
        for lib, count, execs in lib_counts:
            lines.append(f"{lib} ({count} execs)")
            for exec_path in execs:
                lines.append(f"-> {exec_path}")
            lines.append("")
        lines.append("")

    return "\n".join(lines)


def generate_pdf_report(
    scan_dir: Path,
    libs: List[str],
    sorted_results: Dict[str, List[tuple[str, int, List[str]]]],
    output: Path
):
    """Generate PDF report file.
    
    Args:
        scan_dir: Directory that was scanned
        libs: Libraries that were searched for
        sorted_results: Sorted scan results
        output: Path to output PDF file
    """
    doc = SimpleDocTemplate(str(output), pagesize=letter)
    styles = getSampleStyleSheet()
    story = []

    story.append(Paragraph(f"Report on dynamic used libraries by ELF executables on {scan_dir}", styles['Title']))
    story.append(Spacer(1, 12))

    for arch, lib_counts in sorted_results.items():
        story.append(Paragraph(f"---------- {arch} -----------", styles['Heading2']))
        for lib, count, execs in lib_counts:
            story.append(Paragraph(f"{lib} ({count} execs)", styles['Normal']))
            for exec_path in execs:
                story.append(Paragraph(f"-> {exec_path}", styles['Normal']))
            story.append(Spacer(1, 6))
        story.append(Spacer(1, 12))

    doc.build(story)
