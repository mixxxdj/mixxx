#!/usr/bin/python

# double check with: git log --pretty=full |grep "^Author:" |sort |uniq

import os, sys

if len(sys.argv) < 2:
    print "Need a csv filename with rewrite rules"
    sys.exit(1)

print """#!/bin/bash
git filter-branch -f --env-filter '
if [ "$GIT_AUTHOR_NAME" = "" ] ; then
  if [ "$GIT_AUTHOR_EMAIL" = "" ]; then
    GIT_AUTHOR_NAME="unknown" ;
    GIT_AUTHOR_EMAIL="unknown@unknown.com" ;
  else
    GIT_AUTHOR_NAME=$GIT_AUTHOR_EMAIL ;
  fi ;
  export GIT_AUTHOR_NAME ;
fi ;
if [ "$GIT_COMMITTER_NAME" = "" ] ; then
  if [ "$GIT_COMMITTER_EMAIL" = "" ]; then
    GIT_COMMITTER_NAME=$GIT_AUTHOR_NAME;
    GIT_COMMITTER_EMAIL=$GIT_AUTHOR_EMAIL;
  else
    GIT_COMMITTER_NAME=$GIT_COMMITTER_EMAIL;
  fi;
  export GIT_COMMITTER_NAME;
  export GIT_COMMITTER_EMAIL;
fi;"""

def name_email(s):
    if s.find('<') == -1:
        return s.strip(),""
    name = s.split('<')[0].strip()
    email = s[s.find('<') + 1 : s.find('>')].strip()
    return name,email

f = open(sys.argv[1])
for l in f.readlines():
    before, after = l.split(',')[0:2]
    if len(after.strip()) == 0:
        continue
    bad_name, bad_email = name_email(before)
    new_name, new_email = name_email(after)
    if len(bad_email.strip()) == 0:
        print """if [ "$GIT_AUTHOR_NAME" = "%s" ]; then
  GIT_AUTHOR_EMAIL="%s";
  GIT_AUTHOR_NAME="%s";
fi;""" % (bad_name, new_email, new_name)
        print """if [ "$GIT_COMMITTER_NAME" = "%s" ]; then
  GIT_COMMITTER_EMAIL="%s";
  GIT_COMMITTER_NAME="%s";
fi;""" % (bad_name, new_email, new_name)
    else:
        print """if [ "$GIT_AUTHOR_EMAIL" = "%s" ]; then
  GIT_AUTHOR_EMAIL="%s";
  GIT_AUTHOR_NAME="%s";
fi;""" % (bad_email, new_email, new_name)
        print """if [ "$GIT_COMMITTER_EMAIL" = "%s" ]; then
  GIT_COMMITTER_EMAIL="%s";
  GIT_COMMITTER_NAME="%s";
fi;""" % (bad_email, new_email, new_name)

print """
export GIT_AUTHOR_NAME
export GIT_AUTHOR_EMAIL
export GIT_COMMITTER_NAME
export GIT_COMMITTER_EMAIL'
"""
