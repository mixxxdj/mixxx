#!/bin/bash

if [ "$2"x == "x" ] ; then
    echo "Need the name of the branch to pull from lp, and the name of the git branch"
    echo "like, trunk-update and master,"
    echo "or 1.11-update and 1.11."
    echo "where does -update come from?  the original git bzr import command"
    exit 1
fi

export PATH="/home/owen/bin:$PATH"
source /home/owen/agent.sh
cd /home/owen/src/github/mixxx-official/mixxx

sync_branch="$1"
git_branch="$2"

git checkout "$1" && git bzr pull
if [ $? -ne 0 ] ; then
	echo "Error pulling $1 from bzr"
	exit 1
fi
git checkout -b "$1"-rewrite
if [ $? -ne 0 ] ; then
	echo "Error pulling $1 from bzr"
	exit 1
fi
~/src/github/mixxx-conversion-stuff/git-filter-branch-command-all.sh
if [ $? -ne 0 ] ; then
	echo "Error rewriting history"
	exit 1
fi
git checkout "$2"
if [ $? -ne 0 ] ; then
	echo "Error checking out $2"
	exit 1
fi
git merge "$1"-rewrite
if [ $? -ne 0 ] ; then
	echo "Error merging from $1-rewrite"
	exit 1
fi
git push
if [ $? -ne 0 ] ; then
	echo "Error pushing"
	exit 1
fi
git branch -d "$1"-rewrite
if [ $? -ne 0 ] ; then
	echo "Error deleting update branch"
	exit 1
fi
