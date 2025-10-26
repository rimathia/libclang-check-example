#!/usr/bin/env python3
"""
Checker for detecting dangerous auto usage with Eigen expression templates.

This script uses libclang to parse C++ code and find auto declarations that may
capture Eigen expression templates instead of materialized results.

Usage:
    uv run eigen_auto_check.py <source_file> [build_dir]

Example:
    uv run eigen_auto_check.py ../examples.cpp
    uv run eigen_auto_check.py ../examples.cpp ../build
"""

import sys
import argparse
from pathlib import Path
import clang.cindex

# Global verbose flag
VERBOSE = False


def is_allowed_auto_type(canonical_type_name: str) -> bool:
    """
    Check if a canonical type is explicitly allowed to be used with auto.

    Args:
        canonical_type_name: The canonical type name from type.get_canonical().spelling
                            Should be fully qualified like "Eigen::Matrix<double, -1, -1>"

    Returns:
        True if the type is a plain storage type (safe), False otherwise
    """
    # Remove leading 'const ' if present
    type_to_check = canonical_type_name.removeprefix("const ")

    # Remove trailing ' &' or ' *' if present
    type_to_check = type_to_check.rstrip("&* ")

    if VERBOSE:
        print(
            f"[DEBUG] Checking allowlist for canonical type: '{canonical_type_name}' -> '{type_to_check}'"
        )

    # Allowlist: Only Eigen::Matrix and Eigen::Array are plain storage types
    # All other Eigen types (Product, CwiseBinaryOp, Transpose, etc.) are expression templates

    if type_to_check.startswith("Eigen::Matrix<") or type_to_check.startswith(
        "Eigen::Array<"
    ):
        if VERBOSE:
            print("[DEBUG] -> ALLOWED (plain Matrix/Array storage type)")
        return True

    if VERBOSE:
        print("[DEBUG] -> NOT ALLOWED (likely expression template)")
    return False


def get_source_range(lines: list, start_line: int, end_line: int) -> str:
    """
    Get a range of source lines from pre-loaded file content.

    Args:
        lines: List of lines from the source file
        start_line: Starting line number (1-indexed)
        end_line: Ending line number (1-indexed)

    Returns:
        Joined string of the line range
    """
    if 1 <= start_line <= len(lines) and 1 <= end_line <= len(lines):
        # Get all lines in the range
        source_lines = [lines[i - 1].rstrip() for i in range(start_line, end_line + 1)]
        # Join with spaces to make it a single line for display
        return " ".join(line.strip() for line in source_lines if line.strip())
    return "?"


def analyze_var_decl(cursor, filename: str, source_lines: list) -> list:
    """Analyze a variable declaration to see if it uses auto with Eigen types."""
    issues = []

    # Check if this is a variable declaration
    if cursor.kind != clang.cindex.CursorKind.VAR_DECL:
        return issues

    # Get the source location
    extent = cursor.extent
    if not extent or not extent.start.file:
        return issues

    # Only analyze declarations in the target file
    if extent.start.file.name != filename:
        return issues

    # Check if declaration uses auto/decltype(auto)
    tokens = list(cursor.get_tokens())
    if not tokens:
        return issues

    uses_auto = False
    uses_decltype_auto = False

    # Look for auto or decltype(auto) in the declaration
    for i, token in enumerate(tokens):
        if token.spelling == "auto":
            uses_auto = True
            # Check for decltype(auto)
            if i > 0 and tokens[i - 1].spelling == "(":
                if i > 1 and tokens[i - 2].spelling == "decltype":
                    uses_decltype_auto = True
            break

    if not uses_auto:
        return issues

    # Get the actual deduced type
    var_type = cursor.type
    type_name = var_type.spelling

    # Get the canonical type (strips typedef sugar)
    canonical_type = var_type.get_canonical()
    canonical_name = canonical_type.spelling

    if VERBOSE:
        print(
            f"[DEBUG] Variable '{cursor.spelling}': type='{type_name}', canonical='{canonical_name}'"
        )

    # Only check Eigen types (canonical types always start with Eigen::)
    if not canonical_name.startswith("Eigen::"):
        return issues

    # Check if the canonical type is on the allowlist
    is_allowed = is_allowed_auto_type(canonical_name)

    if not is_allowed:
        # This is an Eigen type that's NOT on the allowlist
        # It's likely an expression template or other unsafe type
        location = extent.start
        end_location = extent.end
        auto_kind = "decltype(auto)" if uses_decltype_auto else "auto"

        # Get the complete source range (handles multi-line expressions)
        source_text = get_source_range(source_lines, location.line, end_location.line)

        issues.append(
            {
                "file": filename,
                "line": location.line,
                "column": location.column,
                "variable": cursor.spelling,
                "type": canonical_name,
                "type_as_written": type_name,
                "auto_kind": auto_kind,
                "source": source_text,
            }
        )

    return issues


def load_compilation_database(build_dir: str):
    """
    Load compilation database from build directory.

    Args:
        build_dir: Directory containing compile_commands.json

    Returns:
        CompilationDatabase object
    """
    build_path = Path(build_dir).resolve()
    compdb_path = build_path / "compile_commands.json"

    if VERBOSE:
        print(f"[DEBUG] Loading compile_commands.json from: {build_path}")
        print(f"[DEBUG] compile_commands.json exists: {compdb_path.exists()}")

    if not compdb_path.exists():
        raise FileNotFoundError(f"compile_commands.json not found at {compdb_path}")

    return clang.cindex.CompilationDatabase.fromDirectory(str(build_path))


def get_compile_args(compdb, filename: str) -> list:
    """
    Get compilation arguments for a specific file from the compilation database.

    Args:
        compdb: CompilationDatabase object
        filename: The source file to get compile commands for

    Returns:
        List of compiler arguments
    """
    commands = compdb.getCompileCommands(filename)

    if not commands:
        raise ValueError(f"No compilation commands found for {filename} in database")

    if VERBOSE:
        print(
            f"[DEBUG] Found {len(list(commands))} compilation command(s) for {filename}"
        )

    # Extract compiler flags from compilation database
    cmd_args = []
    for cmd in compdb.getCompileCommands(filename):
        cmd_args.extend(cmd.arguments)

    # Skip the compiler executable name and filter out problematic flags
    filtered_args = []
    skip_next = False
    for arg in cmd_args[1:]:  # Skip compiler name
        if skip_next:
            skip_next = False
            continue
        # Skip output-related flags that libclang doesn't need
        if arg in ["-o", "-c"]:
            skip_next = True
            continue
        if arg.startswith("-o"):
            continue
        # Skip driver mode flags
        if arg.startswith("--driver-mode"):
            continue
        # Skip architecture flags that might confuse libclang
        if arg in ["-arch"]:
            skip_next = True
            continue
        filtered_args.append(arg)

    if VERBOSE:
        print(f"[DEBUG] Filtered compile args: {' '.join(filtered_args)}")

    return filtered_args


def check_file(filename: str, compdb) -> list:
    """Check a single C++ file for auto/Eigen issues."""
    issues = []

    # Read source file once for later use
    with open(filename, "r", encoding="utf-8") as f:
        source_lines = f.readlines()

    # Initialize libclang
    index = clang.cindex.Index.create()

    # Get compilation arguments for this file
    args = get_compile_args(compdb, filename)

    # Parse the file
    if VERBOSE:
        print(f"[DEBUG] Parsing {filename}...")

    translation_unit = index.parse(filename, args=args)

    # Check for parse errors
    if VERBOSE:
        print(f"[DEBUG] Checking diagnostics...")
    has_errors = False
    for diag in translation_unit.diagnostics:
        if diag.severity >= clang.cindex.Diagnostic.Error:
            has_errors = True
            if VERBOSE:
                severity_name = {
                    clang.cindex.Diagnostic.Ignored: "Ignored",
                    clang.cindex.Diagnostic.Note: "Note",
                    clang.cindex.Diagnostic.Warning: "Warning",
                    clang.cindex.Diagnostic.Error: "Error",
                    clang.cindex.Diagnostic.Fatal: "Fatal",
                }.get(diag.severity, "Unknown")
                print(f"[DEBUG] {severity_name}: {diag.spelling}", file=sys.stderr)

    if VERBOSE:
        if has_errors:
            print(
                "[DEBUG] ✗ File has parse errors, results may be incomplete",
                file=sys.stderr,
            )
        else:
            print("[DEBUG] ✓ Parse successful")

    # Walk the AST
    def visit_node(cursor):
        # Analyze this node
        issues.extend(analyze_var_decl(cursor, filename, source_lines))

        # Recurse into children
        for child in cursor.get_children():
            visit_node(child)

    visit_node(translation_unit.cursor)

    return issues


def main():
    global VERBOSE

    parser = argparse.ArgumentParser(
        description="Check for dangerous auto usage with Eigen expression templates",
        epilog="Examples:\n"
        "  uv run eigen_auto_check.py ../examples.cpp ../build\n"
        "  uv run eigen_auto_check.py ../examples.cpp ../build --verbose",
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    parser.add_argument("source_file", help="C++ source file to check")
    parser.add_argument(
        "build_dir", help="Build directory containing compile_commands.json"
    )
    parser.add_argument(
        "--verbose", "-v", action="store_true", help="Enable verbose debug output"
    )

    args = parser.parse_args()

    VERBOSE = args.verbose
    source_file = args.source_file
    build_dir = args.build_dir

    # Resolve to absolute path
    source_path = Path(source_file).resolve()
    if not source_path.exists():
        print(f"Error: File not found: {source_file}", file=sys.stderr)
        sys.exit(1)

    if not VERBOSE:
        print(f"Checking {source_path}...")

    # Load compilation database once
    compdb = load_compilation_database(build_dir)

    # Check the file
    issues = check_file(str(source_path), compdb)

    if not issues:
        print(f"✓ No issues found")
        return 0

    print(f"Found {len(issues)} issue(s):\n")

    for issue in issues:
        print(
            f"{issue['file']}:{issue['line']}:{issue['column']}: error: "
            f"'{issue['variable']}' uses {issue['auto_kind']} with Eigen expression template"
        )
        print(f"  Type: {issue['type']}")
        print(f"  Source: {issue['source']}")
        print()

    return 1


if __name__ == "__main__":
    sys.exit(main())
