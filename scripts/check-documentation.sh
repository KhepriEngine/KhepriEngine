#!/bin/bash
# Checks documentation of all files in the repository

DOXYGEN=doxygen

SCRIPT_DIR=$(dirname $(realpath "$0"))
GIT_REPO=$(git -C "$SCRIPT_DIR" rev-parse --show-toplevel)

exec
cd $GIT_REPO

# Extra options to check for developer documentation
DOXYGEN_DEVELOPER_CHECKS=$(cat <<EOF
GENERATE_HTML = NO

# Check local classes as well
EXTRACT_LOCAL_CLASSES = YES
EOF
)

# Run doxygen without generating anything.
# Filter out the warning about having no output formats specified.
DOXYGEN_WARNINGS=$((cat DoxyFile; echo "$DOXYGEN_DEVELOPER_CHECKS") \
    | $DOXYGEN - 2>&1 >/dev/null | grep -v "GENERATE_*")

if [ "$DOXYGEN_WARNINGS" != "" ]; then
    echo "error: documentation warnings:"
    echo "$DOXYGEN_WARNINGS"
    exit 1
fi
