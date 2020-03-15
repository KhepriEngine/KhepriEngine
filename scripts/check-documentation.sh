#!/bin/bash
# Checks documentation of all files in the repository

DOXYGEN=doxygen

exec

# Run doxygen without generating anything.
# Filter out the warning about having no output formats specified.
DOXYGEN_WARNINGS=$((cat DoxyFile; echo "GENERATE_HTML = NO") \
    | $DOXYGEN - 2>&1 >/dev/null | grep -v "GENERATE_*")

if [ "$DOXYGEN_WARNINGS" != "" ]; then
    echo "error: documentation warnings:"
    echo "$DOXYGEN_WARNINGS"
    exit 1
fi
