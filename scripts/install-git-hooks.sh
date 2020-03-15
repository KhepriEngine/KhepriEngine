#!/bin/bash

FORCE=0
while [ "$1" != "" ]; do
  case "$1" in
  --force)
      FORCE=1
      ;;
  *)
      echo "error: unknown option \"$1\""
      exit 1
  esac
  shift
done

SCRIPT_DIR=$(dirname $(realpath "$0"))
HOOKS_SOURCES="$SCRIPT_DIR/git"

REL_HOOKS_DIR=$(git -C "${SCRIPT_DIR}" rev-parse --git-path hooks)
GIT_HOOKS_DIR=$(realpath "$SCRIPT_DIR/$REL_HOOKS_DIR")
echo "Installing Git hooks into: $GIT_HOOKS_DIR"

# Get the list of files to copy
FILES=$(find "$HOOKS_SOURCES" -type f | xargs realpath --relative-to="$HOOKS_SOURCES")

if [[ $FORCE -eq 0 ]]; then
    # Verify that we're not overwriting anything
    for file in $FILES; do
        if [ -f "$GIT_HOOKS_DIR/$file" ]; then
            echo "error: file \"$file\" already exists in $GIT_HOOKS_DIR (use --force to overwrite anyway)"
            exit 1
        fi
    done
fi

# Copy the files
for file in $FILES; do
    mkdir -p $(dirname "$GIT_HOOKS_DIR/$file")
    cp "$HOOKS_SOURCES/$file" "$GIT_HOOKS_DIR/$file" || exit 1
done

echo "Installed Git hooks"