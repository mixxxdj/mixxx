#!/bin/sh
# This script is used when generating a release source code tarball.
# In any other situation, Mixxx should be built from the Git repository
# and CMake will do the same as this script.
GIT_TAG=$(git describe --tags --abbrev=0)
GIT_COMMIT_DESCRIPTION=$(git describe --tags --always --dirty=-modified)
TZ=UTC GIT_COMMIT_DATE=$(git show --quiet --format=%cd --date=short)

OUTPUT_FILE="$(dirname "$0")/../src/git_info.h"
cp "$(dirname "$0")/../src/git_info.h.template" "$OUTPUT_FILE"
sed -i "s/@GIT_TAG@/$GIT_TAG/" "$OUTPUT_FILE"
sed -i "s/@GIT_COMMIT_DESCRIPTION@/$GIT_COMMIT_DESCRIPTION/" "$OUTPUT_FILE"
sed -i "s/@GIT_COMMIT_DATE@/$GIT_COMMIT_DATE/" "$OUTPUT_FILE"
