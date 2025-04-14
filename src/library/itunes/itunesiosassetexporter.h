#pragma once

#include <QDir>
#include <QString>
#include <QUrl>

/// A facility for exporting assets (e.g. audio files) on Apple platforms. This
/// is needed to copy audio files from the user's local music library into a
/// writable location in the sandbox.
class ITunesIOSAssetExporter {
  public:
    ITunesIOSAssetExporter(const QDir& outputDir);

    /// Synchronously exports the asset with the given URL to the output
    /// directory. URLs using the `ipod-library://` scheme are supported.
    /// Returns the output path.
    QString exportAsset(const QUrl& url);

  private:
    QDir outputDir;
};
