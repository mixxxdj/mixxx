#!/usr/bin/perl -w

#
# Perl script for making linux and mac binary package of Mixxx.
#
#

$BASE='~mixxxdist';

# Check if this is running on Linux
$_ = `uname`;
if (m/Linux/)
{
    $arch = 'i586';
}
elsif (m/Darwin/)
{
    $arch = 'mac';
}
else
{
    die('This script only works on Linux or Mac');
}

# Check if mixxx exists
@out = `ls src/mixxx`;
if ($#out<0)
{
    die('You need to compile Mixxx first');
}

# Get version
($t1, $t2, $version) = split(' ',`grep VERSION src/defs.h`);
printf("Version: %s\n",$version);

# Construct package base dir
`install -d $BASE`;

# Copy skins to $BASE/share/mixxx
`install -d $BASE/src/skins/outline`;
`install -d $BASE/src/skins/traditional`;
`install -d $BASE/src/skins/outlineClose`;
`install src/skins/outline/* $BASE/src/skins/outline`;
`install src/skins/outlineClose/* $BASE/src/skins/outlineClose`;
`install src/skins/traditional/* $BASE/src/skins/traditional`;

# Copy midi config files
`install -d $BASE/src/midi`;
`install src/midi/* $BASE/src/midi`;

# Copy keyboard config files
`install -d $BASE/src/keyboard`;
`install src/keyboard/* $BASE/src/keyboard`;

# Copy mixxx binary to $BASE/bin
`install -d $BASE/src`;
`install src/mixxx $BASE/src`;

# Copy doc files
`install README $BASE`;
`install LICENCE $BASE`;
`install COPYING $BASE`;
`install Configuration.txt $BASE`;

# Copy install script if running on Linux
if ($arch =~ /i586/)
{
    `install install.pl $BASE`;
}

# Make tar file
$path = `pwd`;
chdir $BASE or die "Can't change to package dir!\n";
`tar -cvzf ~/mixxx-$version-$arch.tar.gz *`;
chdir $path;

printf("Package finished\n");
