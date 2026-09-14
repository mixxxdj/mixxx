#pragma once

#include <QString>

namespace mixxx {
namespace android {

/// Requests the runtime permissions Mixxx needs to read the user's music
/// collection.
///
/// Which permission that is depends on the API level:
///  * API >= 33 (Android 13): READ_MEDIA_AUDIO
///  * API <= 32: READ_EXTERNAL_STORAGE
/// On API <= 29 WRITE_EXTERNAL_STORAGE is requested in addition, so that
/// Mixxx may write its own files (recordings, exported QML) to shared
/// storage.
///
/// Returns true if the collection may be read afterwards. Blocks until the
/// user has answered the system dialog. Must not be called from the Android
/// UI thread (Qt's main thread is a separate thread, so calling this from
/// Mixxx' main thread is fine).
bool requestMusicLibraryPermissions();

/// Returns true if the app is allowed to read/write arbitrary paths in shared
/// storage. This is MANAGE_EXTERNAL_STORAGE ("All files access") on API >= 30
/// and WRITE_EXTERNAL_STORAGE below that.
bool hasFullExternalStorageAccess();

/// Opens the system settings page where the user can grant "All files
/// access". Does nothing if the permission is already granted. This is
/// asynchronous, the result is only visible after the user returns to Mixxx.
void requestFullExternalStorageAccess();

/// Returns the directory Mixxx should offer as the default music directory,
/// creating it if necessary. Falls back to the app-private music directory if
/// shared storage is not accessible.
QString defaultMusicDirectory();

} // namespace android
} // namespace mixxx
