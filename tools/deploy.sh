#!/bin/bash
#
# Deploy artifacts (e.g. dmg, deb files) built by CI to downloads.mixxx.org.

set -eu -o pipefail

USER=mixxx
HOSTNAME=downloads-hostgator.mixxx.org
DESTDIR=public_html/downloads/builds
SSH="ssh -i ${SSH_KEY} -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null"
GIT_BRANCH=$(git rev-parse --abbrev-ref HEAD)
DEST_PATH=${DESTDIR}/${GIT_BRANCH}/${OS}
TMP_PATH=${DESTDIR}/.tmp/${GIT_BRANCH}/${OS}

echo "Deploying to $TMP_PATH, then to $DEST_PATH."

# Remove permissions for group and other users so that ssh-keygen does not
# complain about the key not being protected.
chmod go-rwx ${SSH_KEY}

# "Unlock" the key by removing its password. This is easier than messing with ssh-agent.
ssh-keygen -p -P ${DOWNLOADS_HOSTGATOR_DOT_MIXXX_DOT_ORG_KEY_PASSWORD} -N "" -f ${SSH_KEY}

# Always upload to a temporary path.
# This prevents users from downloading an incomplete file from the server which has not yet finished deploying.
shopt -s extglob
rsync -e "${SSH}" --rsync-path="mkdir -p ${TMP_PATH} && rsync" -r --delete-after ${FILE_TO_DEPLOY} ${USER}@${HOSTNAME}:${TMP_PATH}

FILE_NAME=$(basename $FILE_TO_DEPLOY)
FILE_EXTENSION="${FILE_NAME##*.}"
SYMLINK_NAME="Mixxx-${GIT_BRANCH}-latest.${FILE_EXTENSION}"

# Move from the temporary path to the final destination.
$SSH ${USER}@${HOSTNAME} << EOF
mkdir -p ${DEST_PATH} &&
mv ${TMP_PATH}/* ${DEST_PATH} &&
rmdir ${TMP_PATH} &&
cd ${DEST_PATH} &&
ln -sf ${FILE_NAME} ${SYMLINK_NAME}
EOF
