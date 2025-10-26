#!/usr/bin/env python3
"""
Smoke test to verify libclang is working correctly.
"""

import sys

def test_import():
    """Test that we can import clang.cindex"""
    try:
        import clang.cindex
        print("✓ Successfully imported clang.cindex")
        return True
    except ImportError as e:
        print(f"✗ Failed to import clang.cindex: {e}")
        return False


def test_parse():
    """Test that we can parse a simple C++ file"""
    try:
        import clang.cindex

        # Create a simple test file
        test_code = """
        int main() {
            int x = 42;
            return 0;
        }
        """

        index = clang.cindex.Index.create()
        tu = index.parse('test.cpp', args=['-std=c++20'],
                         unsaved_files=[('test.cpp', test_code)])

        # Check for errors
        has_errors = False
        for diag in tu.diagnostics:
            if diag.severity >= clang.cindex.Diagnostic.Error:
                has_errors = True
                print(f"  Parse error: {diag.spelling}")

        if not has_errors:
            print("✓ Successfully parsed test C++ code")
            return True
        else:
            print("✗ Failed to parse test C++ code")
            return False

    except Exception as e:
        print(f"✗ Failed to parse test code: {e}")
        return False


def test_ast_traversal():
    """Test that we can traverse the AST"""
    try:
        import clang.cindex

        test_code = """
        int main() {
            auto x = 42;
            return 0;
        }
        """

        index = clang.cindex.Index.create()
        tu = index.parse('test.cpp', args=['-std=c++20'],
                         unsaved_files=[('test.cpp', test_code)])

        # Find the auto declaration
        found_auto = False

        def visit_node(cursor):
            nonlocal found_auto
            if cursor.kind == clang.cindex.CursorKind.VAR_DECL:
                if cursor.spelling == 'x':
                    found_auto = True
                    print(f"  Found variable: {cursor.spelling}, type: {cursor.type.spelling}")

            for child in cursor.get_children():
                visit_node(child)

        visit_node(tu.cursor)

        if found_auto:
            print("✓ Successfully traversed AST and found auto declaration")
            return True
        else:
            print("✗ Failed to find auto declaration in AST")
            return False

    except Exception as e:
        print(f"✗ Failed to traverse AST: {e}")
        import traceback
        traceback.print_exc()
        return False


def main():
    print("Testing libclang installation...\n")

    tests = [
        ("Import test", test_import),
        ("Parse test", test_parse),
        ("AST traversal test", test_ast_traversal),
    ]

    results = []
    for name, test_func in tests:
        print(f"\n{name}:")
        result = test_func()
        results.append(result)

    print("\n" + "="*50)
    if all(results):
        print("✓ All tests passed! libclang is working correctly.")
        return 0
    else:
        print("✗ Some tests failed. Please check your libclang installation.")
        return 1


if __name__ == '__main__':
    sys.exit(main())
