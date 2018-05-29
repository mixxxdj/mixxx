#ifndef LIBRARY_PREFERENCES_H
#define LIBRARY_PREFERENCES_H

#define PREF_LEGACY_LIBRARY_DIR ConfigKey("[Playlist]","Directory")

// disable inline metadata editing in track table on selected click
// by default on macOS due to https://bugs.launchpad.net/mixxx/+bug/1665583
#ifdef Q_OS_MAC
#define PREF_LIBRARY_EDIT_METADATA_DEFAULT false
#else
#define PREF_LIBRARY_EDIT_METADATA_DEFAULT true
#endif

#endif /* LIBRARY_PREFERENCES_H */
