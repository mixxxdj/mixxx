#pragma once

#include <QString>

/**
 * When writing the tags in-place directly into the original file
 * an intermediate failure might corrupt this precious file. For
 * example this might occur if the application crashes or is quit
 * unexpectedly, if the original file becomes unavailable while
 * writing by disconnecting a drive, if the file system is running
 * out of free space, or if an unexpected driver or hardware failure
 * occurs.
 *
 * To reduce the risk of corrupting the original file all write
 * operations are performed on a temporary file that is created
 * as an exact copy of the original file. Only after all write
 * operations have finished successfully the original file is
 * replaced with the temporary file.
 */

namespace mixxx {

class SafelyWritableFile final {
  public:
    enum class SafetyMode {
        // Bypass
        Direct,
        // Create a temp file with suffix
        Edit,
        // Use temp file name with prefix
        Replace,
    };
    SafelyWritableFile(QString origFileName,
            SafelyWritableFile::SafetyMode safetyMode);
    ~SafelyWritableFile();

    const QString& fileName() const;

    bool isReady() const;

    bool commit();

    void cancel();

  private:
    QString m_origFileName;
    QString m_tempFileName;
};

} // namespace mixxx
