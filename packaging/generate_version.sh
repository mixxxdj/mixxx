#!/bin/sh
GIT_COMMIT_DESCRIPTION=$(git describe --tags --always --dirty=-modified)
TZ=UTC GIT_COMMIT_DATE=$(git show --quiet --format=%cd --date=short)

OUTPUT_FILE="$(dirname "$0")/../src/git_info.h"
cp "$(dirname "$0")/../src/git_info.h.template" "$OUTPUT_FILE"
sed -i "s/@GIT_COMMIT_DESCRIPTION@/$GIT_COMMIT_DESCRIPTION/" "$OUTPUT_FILE"
sed -i "s/@GIT_COMMIT_DATE@/$GIT_COMMIT_DATE/" "$OUTPUT_FILE"
