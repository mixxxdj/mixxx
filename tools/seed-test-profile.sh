#!/bin/bash
# Pre-seeds a Mixxx test profile with a library directory so the
# "Choose music library directory" dialog does not appear on startup.
#
# If the database does not exist yet, it creates a minimal one with
# the directories table pre-populated.
#
# Usage: seed-test-profile.sh <settingsPath> [libraryDir]

set -euo pipefail

SETTINGS_PATH="${1:?Usage: seed-test-profile.sh <settingsPath> [libraryDir]}"
LIBRARY_DIR="${2:-/media/audio/music/modules/modarchive/modarchive_chillout}"

DB_FILE="${SETTINGS_PATH}/mixxxdb.sqlite"

if [ ! -d "${SETTINGS_PATH}" ]; then
    mkdir -p "${SETTINGS_PATH}"
fi

if [ ! -f "${DB_FILE}" ]; then
    # Create a minimal database with just the directories table.
    # Mixxx will populate the rest of the schema on startup.
    sqlite3 "${DB_FILE}" "
        CREATE TABLE IF NOT EXISTS directories (
            directory TEXT UNIQUE
        );
        INSERT OR IGNORE INTO directories (directory) VALUES ('${LIBRARY_DIR}');
    "
    echo "Created ${DB_FILE} with library directory: ${LIBRARY_DIR}"
else
    sqlite3 "${DB_FILE}" "
        CREATE TABLE IF NOT EXISTS directories (
            directory TEXT UNIQUE
        );
        INSERT OR IGNORE INTO directories (directory) VALUES ('${LIBRARY_DIR}');
    "
    echo "Seeded existing ${DB_FILE} with library directory: ${LIBRARY_DIR}"
fi
