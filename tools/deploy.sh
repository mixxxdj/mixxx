#!/bin/bash
#
# Deploy artifacts (e.g. dmg, deb files) built by CI to downloads.mixxx.org.

set -eu -o pipefail

USER=mixxx
HOSTNAME=downloads-hostgator.mixxx.org
DESTDIR=public_html/downloads/builds
SSH="ssh -i ${SSH_KEY} -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null"
GIT_BRANCH="$(git rev-parse --abbrev-ref HEAD)"
DEST_PATH="${DESTDIR}/${GIT_BRANCH}/${OS}"
TMP_PATH="${DESTDIR}/.tmp/${GIT_BRANCH}/${OS}"

echo "Deploying to $TMP_PATH, then to $DEST_PATH."

# Remove permissions for group and other users so that ssh-keygen does not
# complain about the key not being protected.
chmod go-rwx "${SSH_KEY}"

# Unlock the key by removing its password. This is easier than messing with ssh-agent.
ssh-keygen -p -P "${DOWNLOADS_HOSTGATOR_DOT_MIXXX_DOT_ORG_KEY_PASSWORD}" -N "" -f "${SSH_KEY}"

# realpath does not exist on macOS
command -v realpath >/dev/null 2>&1 || realpath() {
    [[ "$1" = /* ]] && echo "$1" || echo "${PWD}/${1#./}"
}

# sha256sum doesn't exist on Windows (Git Bash) or macOS
command -v sha256sum >/dev/null 2>&1 || sha256sum() {
    openssl dgst -sha256 "$@" | sed 's/^SHA256(\(.*\))= \(\w\+\)$/\2  \1/'
}

for FILEPATH in "$@"
do
    # Always upload to a temporary path.
    # This prevents users from downloading an incomplete file from the server which has not yet finished deploying.
    echo "Deploying artifact: ${FILEPATH}"
    FILENAME="$(basename "${FILEPATH}")"
    FILENAME_HASH="${FILENAME}.sha256sum"
    FILEPATH_HASH="${FILEPATH}.sha256sum"

    # There should be no path components in the shasum file, so we need to cd to it first.
    pushd "$(dirname "$(realpath "${FILEPATH}")")"
    sha256sum "${FILENAME}" > "${FILENAME_HASH}"
    popd

    FILEEXT="${FILENAME##*.}"
    SYMLINK_NAME="Mixxx-${GIT_BRANCH}-latest.${FILEEXT}"

    rsync -e "${SSH}" --rsync-path="mkdir -p ${TMP_PATH} && rsync" -r --delete-after "${FILEPATH}" "${FILEPATH_HASH}" "${USER}@${HOSTNAME}:${TMP_PATH}"

    # Move from the temporary path to the final destination.
    ${SSH} "${USER}@${HOSTNAME}" << EOF
    mkdir -p "${DEST_PATH}" &&
    mv "${TMP_PATH}/${FILENAME}" "${TMP_PATH}/${FILENAME_HASH}" "${DEST_PATH}" &&
    rm -rf "${TMP_PATH}" &&
    cd "${DEST_PATH}" &&
    ln -sf "${FILENAME_HASH}" "${SYMLINK_NAME}.sha256sum" &&
    ln -sf "${FILENAME}" "${SYMLINK_NAME}"
EOF
done
