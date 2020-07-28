#!/bin/bash
#
# Deploy artifacts (e.g. dmg, deb files) built by Travis to downloads.mixxx.org.
# Run within the cmake_build directory.

set -eu -o pipefail

USER=mixxx
HOSTNAME=downloads-hostgator.mixxx.org
TRAVIS_DESTDIR=public_html/downloads/builds/travis
SSH_KEY=../build/certificates/downloads-hostgator.mixxx.org.key
SSH="ssh -i ${SSH_KEY} -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null"
DEST_PATH=${TRAVIS_DESTDIR}/${TRAVIS_BRANCH}/
TMP_PATH=${TRAVIS_DESTDIR}/.tmp/$TRAVIS_BUILD_ID/

echo "Deploying to $TMP_PATH, then to $DEST_PATH."

# Remove permissions for group and other users so that ssh-keygen does not
# complain about the key not being protected.
chmod go-rwx ${SSH_KEY}

# "Unlock" the key by removing its password. This is easier than messing with ssh-agent.
ssh-keygen -p -P ${DOWNLOADS_HOSTGATOR_DOT_MIXXX_DOT_ORG_KEY_PASSWORD} -N "" -f ${SSH_KEY}

# Always upload to a temporary path.
shopt -s extglob
rsync -e "${SSH}" --rsync-path="mkdir -p ${TMP_PATH} && rsync" -r --delete-after --quiet ./*.@(deb|dmg) ${USER}@${HOSTNAME}:${TMP_PATH}

# Move from the temporary path to the final destination.
$SSH ${USER}@${HOSTNAME} "mkdir -p ${DEST_PATH} && mv ${TMP_PATH}/* ${DEST_PATH} && rmdir ${TMP_PATH}"
