#!/usr/bin/perl -w

#
# Perl script for making linux and mac binary package of Mixxx.
#
#

$BASE='dist';

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
if ($arch =~ /i586/)
{
    @out = `ls src/mixxx`;
} else {
    @out = `ls src/mixxx.app`;
}

if ($#out<0)
{
    die('You need to compile Mixxx first');
}

# Get version
($_, $_, $version) = split(' ',`grep VERSION src/defs.h`);
# Strip off quotes
print $version;
$version =~ s/\"//g;
print $version;
print $BASE;

# Add version to base
$BASE = $BASE . "/mixxx-$version";

# Determine base for misc files like keyborad, midi and skins:
if ($arch =~ /i586/)
{
    $BASEMISC = $BASE . "/src";
} else {
    $BASEMISC = $BASE . "/mixxx.app";
}

# Construct package base dir
`install -d $BASEMISC`;

# Copy skins to $BASE/share/mixxx
`install -d $BASEMISC/skins/outline`;
`install -d $BASEMISC/skins/outlineClose`;
`install -d $BASEMISC/skins/outlineSmall`;
`install -d $BASEMISC/skins/traditional`;
`cp src/skins/outline/* $BASEMISC/skins/outline`;
`cp src/skins/outlineClose/* $BASEMISC/skins/outlineClose`;
`cp src/skins/outlineSmall/* $BASEMISC/skins/outlineSmall`;
`cp src/skins/traditional/* $BASEMISC/skins/traditional`;

# Copy midi config files
`install -d $BASEMISC/midi`;
`install src/midi/*.midi.cfg $BASEMISC/midi`;

# Copy keyboard config files
`install -d $BASEMISC/keyboard`;
`install src/keyboard/*.kbd.cfg $BASEMISC/keyboard`;

if ($arch =~ /i586/)
{
    # Copy mixxx binary to $BASE/bin
    `install src/mixxx $BASEMISC`;
    `install src/mixxx-with-jack $BASEMISC`;

    # Copy install script
    `install install.pl $BASE`;
} else {
    # Install bundle
    `cp -R src/mixxx.app $BASE/.`;
}

# Copy doc files
`install README $BASE`;
`install LICENSE $BASE`;
`install COPYING $BASE`;
`install Mixxx-Manual.pdf $BASE`;

$path = `pwd`;
chdir $BASE or die "Can't change to package dir!\n";
`tar -cvzf ~/mixxx-$version-$arch.tar.gz *`;
chdir $path;

printf("Package finished\n");
