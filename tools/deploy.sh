#!/bin/bash
#
# Deploy artifacts (e.g. dmg, deb files) built by CI to downloads.mixxx.org.

set -eu -o pipefail

[ -z "${SSH_HOST}" ] && echo "Please set the SSH_HOST env var." >&2 && exit 1
[ -z "${SSH_KEY}" ] && echo "Please set the SSH_KEY env var." >&2 && exit 1
[ -z "${SSH_PASSWORD}" ] && echo "Please set the SSH_PASSWORD env var." >&2 && exit 1
[ -z "${SSH_USER}" ] && echo "Please set the SSH_USER env var." >&2 && exit 1
[ -z "${DESTDIR}" ] && echo "Please set the DESTDIR env var." >&2 && exit 1

SSH="ssh -i ${SSH_KEY} -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null"

echo "Deploying contents of $1 to $DESTDIR."

# Remove permissions for group and other users so that ssh-keygen does not
# complain about the key not being protected.
chmod go-rwx "${SSH_KEY}"

# Unlock the key by removing its password. This is easier than messing with ssh-agent.
ssh-keygen -p -P "${SSH_PASSWORD}" -N "" -f "${SSH_KEY}"

rsync --rsh="${SSH}" --verbose --recursive --checksum --times --delay-updates "$@" "${SSH_USER}@${SSH_HOST}:${DESTDIR}/"
