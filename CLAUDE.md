# General
This is a playground for getting a clang-tidy check for a c++ project to work. The purpose of the clang-tidy check is specific to the eigen library: find places where we deduce an expression template type but we actually meant to deduce a result type (typically a matrix or vector).

# Python

We use uv to manage python environments. One example how to run a script is changing the the `python_env` directory and executing `uv run test_libclang.py`.
