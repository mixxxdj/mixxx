# Mixxx Portable installation

## How it works

The Mixxx binary itself is the same for portable or normal installation.
Portable status is determined at runtime if Mixxx executable is named
`mixxx-portable` (case insensitive and without extension) or if the `--portable`
command-line argument is passed to Mixxx.

In portable mode, Mixxx looks for its resource files (like skins, controller
mappings, translations and so on) in a sundirectory `mixxx-resources` at the
same place the executable resides (location can be overwritten with the
command-line option `--resourcePath`).

It also store all settings in a subdirectory `mixxx-settings`
at the same place the executable resides (location can be overwritten with the
command-line option `--settingsPath`).

No attempt will be made to access or upgrade configuration outside of this
`mixxx-settings` directory. This allows you to use different portable Mixxx
versions with completely separate/isolated configurations.

## Upgrade

If you want to upgrade from one portable version to another, simply copy the
`mixxx-settings` directory in the old version to the new one.

## Track management

Currently, Mixxx can't handle relative library folders so if you store portable
Mixxx and your tracks on a removable drive, be aware that a different mountpoint
or drive letter (depending on your OS) for that drive will lead to missing
tracks because Mixxx will not be able to find them on its own.
To recover your tracks, You need to manually add your music directory to the
library preferences and let Mixxx browse your music folder. It should
automatically recover missing tracks.

## Performance

If you plan to use portable Mixxx directly from a removable media (USB key or
external hard drive), chose a drive with good performance as all access to
Mixxx configuration database and logging will be made on that removable media.
It is recommended to copy the portable Mixxx folder to the hard drive of the
computer you use before using portable Mixxx and to copy back the settings
folder to your removable media after you have finished using Mixxx to backup
your latest data.
