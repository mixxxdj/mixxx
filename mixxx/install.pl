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

# Copy skins and configs to $BASE/share/mixxx
`install -d $BASE/share/mixxx/skins/outline`;
`install -d $BASE/share/mixxx/skins/traditional`;
`install src/skins/outline/* $BASE/share/mixxx/skins/outline`;
`install src/skins/traditional/* $BASE/share/mixxx/skins/traditional`;

# Copy midi config files
`install -d $BASE/share/mixxx/midi`;
`install src/midi/* $BASE/share/mixxx/midi`;

# Copy mixxx binary to $BASE/bin
`install src/mixxx $BASE/bin`;

printf("Install finished\n");

