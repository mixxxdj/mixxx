#include "util/safelywritablefile.h"

#include <QFileInfo>

#include "util/assert.h"
#include "util/logger.h"

#if defined(__WINDOWS__)
#include <Windows.h>

#include <QThread>
#endif

namespace mixxx {

namespace {

const Logger kLogger("SafelyWritableFile");

// Appended to the original file name of the temporary file used for writing
const QString kSafelyWritableTempFileSuffix = QStringLiteral("_temp");

// SafelyWritableFile is also used to write cover arts.
// Cover art worker is suffix sensitive.
// In order to have a successful writing we need a correct suffix.
// So instead of suffix, we need a temp file prefix.
// There is a bug related with the image format reported
// See: https://bugreports.qt.io/browse/QTBUG-89022
const QString kSafelyWritableTempFilePrefix = QStringLiteral("temp_");

// Appended to the original file name for renaming and before deleting this
// file. Should not be longer than kSafelyWritableTempFileSuffix to avoid
// potential failures caused by exceeded path length.
const QString kSafelyWritableOrigFileSuffix = QStringLiteral("_orig");

#if defined(__WINDOWS__)
const int kWindowsSharingViolationMaxRetries = 5;
const int kWindowsSharingViolationSleepBeforeNextRetryMillis = 100;
#endif

} // namespace

SafelyWritableFile::SafelyWritableFile(QString origFileName,
        SafelyWritableFile::SafetyMode safetyMode) {
    // Both file names remain uninitialized until all prerequisite operations
    // in the constructor have been completed successfully. Otherwise failure
    // to create the temporary file will not be handled correctly!
    // See also: https://bugs.launchpad.net/mixxx/+bug/1815305
    DEBUG_ASSERT(m_origFileName.isNull());
    DEBUG_ASSERT(m_tempFileName.isNull());
    if (!QFileInfo(origFileName).isWritable()) {
        kLogger.warning()
                << "Failed to prepare file for writing:"
                << origFileName
                << "is not writable.";
        // Abort constructor
        return;
    }
    switch (safetyMode) {
    case SafetyMode::Edit: {
        QString origFilePath = QFileInfo(origFileName).canonicalFilePath();
        QString tempFileName = origFilePath + kSafelyWritableTempFileSuffix;
        QFile origFile(origFilePath);

        if (!origFile.copy(tempFileName)) {
            kLogger.warning()
                    << origFile.errorString()
                    << "- Failed to clone original into temporary file before writing:"
                    << origFileName
                    << "->"
                    << tempFileName;
            // Abort constructor
            return;
        }
        QFile tempFile(tempFileName);
        DEBUG_ASSERT(tempFile.exists());
        // Both file sizes are expected to be equal after successfully
        // copying the file contents.
        VERIFY_OR_DEBUG_ASSERT(origFile.size() == tempFile.size()) {
            kLogger.warning() << "Failed to verify size after cloning original "
                                 "into temporary file before writing:"
                              << origFile.size() << "<>" << tempFile.size();
            // Cleanup
            if (tempFile.exists() && !tempFile.remove()) {
                kLogger.warning()
                        << tempFile.errorString()
                        << "- Failed to remove temporary file:"
                        << tempFileName;
            }
            // Abort constructor
            return;
        }
        // Successfully cloned original into temporary file for writing - finish initialization
        m_origFileName = std::move(origFileName);
        m_tempFileName = std::move(tempFileName);
        break;
    }
    case SafetyMode::Replace: {
        QString origFilePath = QFileInfo(origFileName).canonicalPath();
        QString origFileCompleteBasename = QFileInfo(origFileName).completeBaseName();
        QString origFileSuffix = QFileInfo(origFileName).suffix();
        QString tempFileCompleteBasename = kSafelyWritableTempFilePrefix +
                origFileCompleteBasename + '.' + origFileSuffix;
        QString tempFileName = origFilePath + '/' + tempFileCompleteBasename;
        m_origFileName = std::move(origFileName);
        m_tempFileName = std::move(tempFileName);
        break;
    }
    case SafetyMode::Direct: {
        // Directly write into original file - finish initialization
        DEBUG_ASSERT(m_tempFileName.isNull());
        m_origFileName = std::move(origFileName);
    }
    }
}

SafelyWritableFile::~SafelyWritableFile() {
    cancel();
}

const QString& SafelyWritableFile::fileName() const {
    if (m_tempFileName.isNull()) {
        // If m_tempFileName has not been initialized then no temporary
        // copy was requested in the constructor.
        return m_origFileName;
    } else {
        return m_tempFileName;
    }
}

bool SafelyWritableFile::isReady() const {
    return !fileName().isEmpty();
}

bool SafelyWritableFile::commit() {
    if (m_tempFileName.isNull()) {
        return true; // nothing to do
    }

    QString origFilePath = QFileInfo(m_origFileName).canonicalFilePath();
    QString backupFileName = origFilePath + kSafelyWritableOrigFileSuffix;
#ifdef __WINDOWS__
    // After Mixxx has closed the track file, the indexer or virus scanner
    // might kick in and fail ReplaceFileW() with a sharing violation when
    // replacing the original file with the one with the updated metadata.
    int i = 0;
    for (; i < kWindowsSharingViolationMaxRetries; ++i) {
        if (ReplaceFileW(
                    reinterpret_cast<LPCWSTR>(origFilePath.utf16()),
                    reinterpret_cast<LPCWSTR>(m_tempFileName.utf16()),
                    reinterpret_cast<LPCWSTR>(backupFileName.utf16()),
                    REPLACEFILE_IGNORE_MERGE_ERRORS | REPLACEFILE_IGNORE_ACL_ERRORS,
                    nullptr,
                    nullptr)) {
            // Success, break retry loop
            break;
        } else {
            DWORD error = GetLastError();
            switch (error) {
            case ERROR_UNABLE_TO_MOVE_REPLACEMENT:
                // The m_tempFileName file could not be renamed. m_origFileName
                // file and m_tempFileName file retain their original file names.
                kLogger.critical()
                        << "Unable to rename replacement file"
                        << m_tempFileName
                        << "->"
                        << m_origFileName;
                return false;
            case ERROR_UNABLE_TO_MOVE_REPLACEMENT_2:
                // The m_tempFileName file could not be moved. The m_tempFileName file still exists
                // under its original name; however, it has inherited the file streams and
                // attributes from the file it is replacing. The m_origFileName file still exists.
                kLogger.critical()
                        << "Unable to move replacement file"
                        << m_tempFileName
                        << "->"
                        << m_origFileName;
                return false;
            case ERROR_UNABLE_TO_REMOVE_REPLACED:
                // The replaced file could not be deleted. The replaced and replacement files
                // retain their original file names.
                kLogger.critical()
                        << "Unable to remove"
                        << m_origFileName
                        << "before replacing by"
                        << m_tempFileName;
                return false;
            case ERROR_SHARING_VIOLATION:
                // The process cannot access the file because it is being used by another process.
                kLogger.warning()
                        << "Unable to replace"
                        << m_origFileName
                        << "by"
                        << m_tempFileName
                        << "because it is used by another process";
                QThread::msleep(kWindowsSharingViolationSleepBeforeNextRetryMillis);
                continue; // Retry
            case ERROR_ACCESS_DENIED:
                kLogger.critical()
                        << "Unable to replace"
                        << m_origFileName
                        << "by"
                        << m_tempFileName
                        << "Access is denied";
                return false;
            default:
                // If any other error is returned, such as ERROR_INVALID_PARAMETER, the replaced
                // and replacement files will retain their original file names. In this scenario,
                // a backup file does not exist and it is not guaranteed that the replacement file
                // will have inherited all of the attributes and streams of the replaced file.
                kLogger.critical()
                        << "Error"
                        << error
                        << "during replacing"
                        << m_origFileName
                        << "by"
                        << m_tempFileName;
                return false;
            }
        }
    }
    QFile backupFile(backupFileName);
    if (backupFile.exists()) {
        if (!backupFile.remove()) {
            kLogger.warning()
                    << backupFile.errorString()
                    << "- Failed to remove backup file after writing:"
                    << backupFile.fileName();
            return false;
        }
    }
    if (i >= kWindowsSharingViolationMaxRetries) {
        // We have given up after the maximum retries in the loop above.
        return false;
    }
#else
    QFile newFile(m_tempFileName);
    if (!newFile.exists()) {
        kLogger.warning()
                << "Temporary file not found:"
                << newFile.fileName();
        return false;
    }
    QFile oldFile(origFilePath);
    if (oldFile.exists()) {
        DEBUG_ASSERT(!QFile::exists(backupFileName)); // very unlikely, otherwise renaming fails
        if (!oldFile.rename(backupFileName)) {
            kLogger.critical()
                    << oldFile.errorString()
                    << "- Failed to rename the original file for backup before writing:"
                    << oldFile.fileName()
                    << "->"
                    << backupFileName;
            return false;
        }
    }
    DEBUG_ASSERT(!QFile::exists(origFilePath));
    if (!newFile.rename(origFilePath)) {
        kLogger.critical()
                << newFile.errorString()
                << "- Failed to rename temporary file after writing:"
                << newFile.fileName()
                << "->"
                << m_origFileName;
        if (oldFile.exists()) {
            // Try to restore the original file
            if (!oldFile.rename(origFilePath)) {
                // Undo operation failed
                kLogger.warning()
                        << oldFile.errorString()
                        << "- Both the original and the temporary file are still available:"
                        << oldFile.fileName()
                        << newFile.fileName();
            }
            return false;
        }
    }
    if (oldFile.exists()) {
        if (!oldFile.remove()) {
            kLogger.warning()
                    << oldFile.errorString()
                    << "- Failed to remove backup file after writing:"
                    << oldFile.fileName();
            return false;
        }
    }
#endif
    // Prevent any further interaction and file access
    m_origFileName = QString();
    m_tempFileName = QString();
    return true;
}

void SafelyWritableFile::cancel() {
    if (m_tempFileName.isNull()) {
        return; // nothing to do
    }
    QFile tempFile(m_tempFileName);
    if (tempFile.exists() && !tempFile.remove()) {
        kLogger.warning()
                << tempFile.errorString()
                << "- Failed to remove temporary file:"
                << m_tempFileName;
    }
    // Prevent any further interaction and file access
    m_origFileName = QString();
    m_tempFileName = QString();
}

} // namespace mixxx
