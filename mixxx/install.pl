#!/usr/bin/perl

# Perl script for installing Mixxx on Linux. Set $BASE to dir where you
# want Mixxx installed
# 
# August 2003, Tue Haste Andersen <haste@diku.dk>
#

$BASE='/usr'; 

# Check if this is running on Linux
$_ = `uname`;
if (!m/Linux/) 
{
	die('This script only works on Linux');
}

# Check if mixxx exists
@out = `ls src/mixxx`;
if ($#out<0)
{
	die('You need to compile Mixxx first');
}

# Copy skins to $BASE/share/mixxx
`install -m 644 -d $BASE/share/mixxx/skins/outline`;
`install -m 644 -d $BASE/share/mixxx/skins/traditional`;
`install -m 644 src/skins/outline/* $BASE/share/mixxx/skins/outline`;
`install -m 644 src/skins/traditional/* $BASE/share/mixxx/skins/traditional`;

# Copy midi config files
`install -m 644 -d $BASE/share/mixxx/midi`;
`install -m 644 src/midi/* $BASE/share/mixxx/midi`;

# Copy mixxx binary to $BASE/bin
`install -m 644 -d $BASE/bin`;
`install -m 644 src/mixxx $BASE/bin`;

# Copy doc files
`install -m 644 -d $BASE/share/doc/mixxx-1.0`;
`install -m 644 README $BASE/share/doc/mixxx-1.0`;
`install -m 644 LICENCE $BASE/share/doc/mixxx-1.0`;

printf("Install finished\n");

