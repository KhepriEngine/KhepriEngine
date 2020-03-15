#!/bin/bash
# Formats all files in the repository
# Note: please ensure the clang-format version here is aligned with
# the one in scripts/git/pre-commit.d/check-format

CLANG_FORMAT=clang-format

SCRIPT_DIR=$(dirname $(realpath "$0"))
GIT_REPO=$(git -C "$SCRIPT_DIR" rev-parse --show-toplevel)

# Redirect output to stderr.
exec 1>&2
cd $GIT_REPO

FILES=$(find . -type f | grep -iE "\.(cpp|cc|c|h|hpp)$")
for file in $FILES; do
    $CLANG_FORMAT -Werror -i ${file}
done