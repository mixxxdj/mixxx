# Mixxx Portable installation

## How it works

The Mixxx binary itself is the same for portable or normal installation.
Portable status is determined at runtime if Mixxx finds directories named
`settings` and `res` at the same place the executable resides.
It will them use its resources from the `res` subdirectory and store all
settings from the `settings` subdirectory. No attempt will be made to access or
upgrade configuration outside of that directory. This allow you to used
different portable Mixxx versions without any sharing between their
configuration.

## Upgrade

If you want to upgrade from one portable version to another, simply copy the
`settings` directory in the old version to the new one.

## Track management

Currently, Mixxx can't handle relative library folders so if you store portable
Mixxx and your track on a removable drive, be aware that a different mountpoint
of drive letter (depending on your OS) for that drive will lead to missing
tracks (Mixxx is unable to find its tracks himself).
To recover your tracks, You need to manually add your music directory to the
library preferences and let Mixxx browse your music folder. It should
automatically recover missing tracks.

## Performance

If you plan to use portable Mixxx directly from a removable media (USB key or
external hard drive), chose a drive with good performance as all access to
Mixxx configuration database and logging will be made on that removable media.
It is recommended to copy the portable Mixxx folder on the hard drive of the
computer you use before using portable Mixxx and to copy back the settings
folder to your removable media after you have finished using Mixxx to backup
your latest data.
