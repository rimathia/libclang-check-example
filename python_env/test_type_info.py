#!/usr/bin/env python3
"""
Test script to explore what type information libclang provides.
"""

import sys
import clang.cindex
from pathlib import Path


def explore_type_info(cursor, filename):
    """Explore detailed type information for auto declarations."""
    if cursor.kind != clang.cindex.CursorKind.VAR_DECL:
        return

    extent = cursor.extent
    if not extent or not extent.start.file:
        return

    if extent.start.file.name != filename:
        return

    # Check if uses auto
    tokens = list(cursor.get_tokens())
    uses_auto = any(t.spelling == 'auto' for t in tokens)

    if not uses_auto:
        return

    var_type = cursor.type

    print(f"\n{'='*80}")
    print(f"Variable: {cursor.spelling} at line {extent.start.line}")
    print(f"Type spelling: '{var_type.spelling}'")
    print(f"Type kind: {var_type.kind}")
    print(f"Type kind name: {var_type.kind.name if hasattr(var_type.kind, 'name') else 'N/A'}")

    # Try to get the canonical type
    canonical = var_type.get_canonical()
    print(f"Canonical type spelling: '{canonical.spelling}'")
    print(f"Canonical == original: {canonical.spelling == var_type.spelling}")

    # Check if it's a typedef
    is_typedef = var_type.kind == clang.cindex.TypeKind.TYPEDEF
    print(f"Is typedef: {is_typedef}")

    # If it's a typedef, get the underlying type
    if is_typedef:
        underlying = canonical
        print(f"Underlying type (from canonical): '{underlying.spelling}'")

    # Try to get the declaration from original type
    print(f"\nDeclaration from original type:")
    decl = var_type.get_declaration()
    if decl and decl.kind != clang.cindex.CursorKind.NO_DECL_FOUND:
        print(f"  Declaration spelling: '{decl.spelling}'")
        print(f"  Declaration kind: {decl.kind}")

        # Get the fully qualified name by walking up the semantic parents
        def get_qualified_name(cursor):
            """Get fully qualified name of a cursor."""
            if cursor is None:
                return ""

            # Get semantic parent
            parent = cursor.semantic_parent
            if parent and parent.kind != clang.cindex.CursorKind.TRANSLATION_UNIT:
                parent_name = get_qualified_name(parent)
                if parent_name:
                    return f"{parent_name}::{cursor.spelling}"
            return cursor.spelling

        qualified_name = get_qualified_name(decl)
        print(f"  Qualified name: '{qualified_name}'")
    else:
        print(f"  No declaration found")

    # Try to get the declaration from canonical type
    print(f"\nDeclaration from canonical type:")
    canonical_decl = canonical.get_declaration()
    if canonical_decl and canonical_decl.kind != clang.cindex.CursorKind.NO_DECL_FOUND:
        print(f"  Declaration spelling: '{canonical_decl.spelling}'")
        print(f"  Declaration kind: {canonical_decl.kind}")
        canonical_qualified_name = get_qualified_name(canonical_decl)
        print(f"  Qualified name: '{canonical_qualified_name}'")

    # Get template argument count
    num_template_args = var_type.get_num_template_arguments()
    print(f"Number of template arguments: {num_template_args}")

    if num_template_args > 0:
        print("Template arguments:")
        for i in range(num_template_args):
            arg = var_type.get_template_argument_type(i)
            if arg:
                print(f"  [{i}]: {arg.spelling}")


def main():
    if len(sys.argv) < 3:
        print("Usage: test_type_info.py <source_file> <build_dir>")
        sys.exit(1)

    source_file = sys.argv[1]
    build_dir = sys.argv[2]

    source_path = Path(source_file).resolve()

    # Initialize libclang
    index = clang.cindex.Index.create()

    # Load from compilation database
    build_path = Path(build_dir).resolve()
    compdb = clang.cindex.CompilationDatabase.fromDirectory(str(build_path))
    commands = compdb.getCompileCommands(str(source_path))

    cmd_args = []
    for cmd in commands:
        cmd_args.extend(cmd.arguments)

    # Filter args
    args = []
    skip_next = False
    for arg in cmd_args[1:]:
        if skip_next:
            skip_next = False
            continue
        if arg in ['-o', '-c', '-arch']:
            skip_next = True
            continue
        if arg.startswith('-o') or arg.startswith('--driver-mode'):
            continue
        args.append(arg)

    print(f"Parsing {source_path}...")
    tu = index.parse(str(source_path), args=args)

    # Walk AST
    def visit(cursor):
        explore_type_info(cursor, str(source_path))
        for child in cursor.get_children():
            visit(child)

    visit(tu.cursor)


if __name__ == '__main__':
    main()
