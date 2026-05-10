"""Command-line interface for bldd."""

from pathlib import Path
from typing import List, Optional

import typer
from rich.console import Console
from rich.panel import Panel

from bldd.report import generate_report
from bldd.scanner import scan_directory, sort_results

app = typer.Typer(
    help="Backward ldd: find ELF executables that depend on given shared libraries.",
    epilog=(
        "Examples:\n"
        "  bldd --scan-dir /usr/bin --libs libc.so.6 --output report.txt\n"
        "  bldd --scan-dir /usr/bin --libs libc.so.6 --output report.pdf --format pdf\n"
        "  bldd --scan-dir /bin --libs libc.so.6 --verbose"
    ),
)
console = Console()


def validate_format(value: str) -> str:
    normalized = value.lower()
    if normalized not in {"txt", "pdf"}:
        raise typer.BadParameter("format must be 'txt' or 'pdf'")
    return normalized


@app.callback(invoke_without_command=True)
def main(
    ctx: typer.Context,
    version: bool = typer.Option(
        False, "--version", help="Show application version."
    ),
    scan_dir: Path = typer.Option(
        Path.cwd(),
        exists=True,
        file_okay=False,
        dir_okay=True,
        readable=True,
        help="Directory to scan recursively for ELF executables.",
    ),
    libs: Optional[List[str]] = typer.Option(
        None, "--libs", "-l", help="Shared library name(s) to search for, e.g. libc.so.6."
    ),
    output: Optional[Path] = typer.Option(
        None,
        "--output",
        "-o",
        help="Output report file path. If omitted, defaults to bldd_report.<format> in current dir.",
    ),
    format: str = typer.Option(
        "txt",
        "--format",
        "-f",
        callback=validate_format,
        help="Report format: txt or pdf.",
    ),
    verbose: bool = typer.Option(
        False,
        "--verbose",
        "-v",
        help="Show progress and summary information on the console.",
    ),
):
    if version:
        console.print("bldd 0.1.0")
        raise typer.Exit()

    if ctx.invoked_subcommand is not None:
        return

    if not libs:
        raise typer.BadParameter("At least one library must be provided with --libs")

    if output is None:
        reports_dir = Path("reports/generated")
        reports_dir.mkdir(parents=True, exist_ok=True)
        output = reports_dir / f"bldd_report.{format}"

    elif output.is_dir():
        raise typer.BadParameter("--output must be a file path, not a directory")

    if verbose:
        console.print(Panel.fit(
            f"Scanning [cyan]{scan_dir}[/cyan]\n" f"Libraries: [green]{', '.join(libs)}[/green]\n" f"Report: [yellow]{output}[/yellow]",
            title="bldd start",
        ))

    results = scan_directory(scan_dir, libs, debug=verbose)
    sorted_results = sort_results(results)
    generate_report(scan_dir, libs, sorted_results, output, format=format)

    total_matches = sum(count for arch_libs in sorted_results.values() for _, count, _ in arch_libs)

    console.print(f"[bold green]Report generated:[/bold green] {output}")
    console.print(f"Total matched library usages: [bold]{total_matches}[/bold]")
    if not sorted_results:
        console.print("[yellow]No matching executables found.[/yellow]")


if __name__ == "__main__":
    app()
