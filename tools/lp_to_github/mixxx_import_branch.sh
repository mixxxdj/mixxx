#!/bin/bash

if [ "$3"x == "x" ] ; then
	echo "BE IN THE GIT DIR ALREADY"
	echo "Need the lp url, the name of the lp sync branch, and the name of the git branch"
    echo "like, lp:mixxx and trunk-update and master,"
    echo "or lp:mixxx/1.11 and 1.11-update and 1.11."
    exit 1
fi

export PATH="/home/owen/bin:$PATH"
source /home/owen/agent.sh

lp_url="$1"
sync_branch="$2"
git_branch="$3"

git bzr import "$lp_url" "$sync_branch"
if [ $? -ne 0 ] ; then
    echo "Error import $1 from bzr"
    exit 1
fi

git checkout "$sync_branch"
if [ $? -ne 0 ] ; then
	echo "Error checkout out $sync_branch"
	exit 1
fi
git checkout -b "$sync_branch"-rewrite
if [ $? -ne 0 ] ; then
	echo "Error branching???"
	exit 1
fi
~/src/github/mixxx-conversion-stuff/git-filter-branch-command-all.sh
if [ $? -ne 0 ] ; then
	echo "Error rewriting history"
	exit 1
fi
git checkout -b "$git_branch"
if [ $? -ne 0 ] ; then
	echo "Error checking out $git_branch"
	exit 1
fi
git push origin HEAD
if [ $? -ne 0 ] ; then
	echo "Error pushing"
	exit 1
fi
git branch -d "$sync_branch"-rewrite
if [ $? -ne 0 ] ; then
	echo "Error deleting update branch"
	exit 1
fi
