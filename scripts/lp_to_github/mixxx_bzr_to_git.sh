#!/bin/bash
export PATH="/home/owen/bin:$PATH"
source /home/owen/agent.sh
cd /home/owen/src/github/mixxx-official/mixxx

trunk=1
oneeleven=1
if [ "$1"x == "--trunk"x ]  ; then
	oneeleven=0
fi

mixxx_migrate_branch.sh trunk-update master

if [ $oneeleven -eq 0 ] ; then
	exit
fi

mixxx_migrate_branch.sh 1.11-update 1.11

mixxx_migrate_branch.sh master_sync-update master_sync

git checkout master
