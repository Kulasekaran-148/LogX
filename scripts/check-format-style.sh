#!/bin/bash
echo "Running clang-format check..."
clang-format --dry-run --Werror src/*.c include/logx/*.h || exit 1

echo "Running clang-tidy for brace enforcement..."
clang-tidy src/*.c -- -Iinclude || exit 1

echo "✅ Style and braces check passed."
