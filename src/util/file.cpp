#include "util/file.h"

#include <QFileDialog>
#include <QRegExp> // required for 'indexIn(QString &str, int pos)
#include <QRegularExpression>

#include "util/assert.h"

namespace {

const QRegularExpression kExtractExtensionRegex(R"(\(\*\.(.*)\)$)");
// extract all extensions from the file filters string "Abc (*.m3u);;Cbx (*.pls)..."
// lazy match doesn't work here unfortunately "(\(\*\.(.*?)\))"
const QRegExp kFileFilterRegex(R"(\(\*\.(.[^\)]*)\))");

} //anonymous namespace

QString filePathWithSelectedExtension(const QString& fileLocationInput,
        const QString& selectedFileFilter,
        const QString& fileFilters) {
    if (fileLocationInput.isEmpty()) {
        return {};
    }
    QString fileLocation = fileLocationInput;
    if (selectedFileFilter.isEmpty()) {
        return fileLocation;
    }

    // Extract 'ext' from QFileDialog file filter string 'Funky type (*.ext)'
    const auto extMatch = kExtractExtensionRegex.match(selectedFileFilter);
    const QString ext = extMatch.captured(1);
    VERIFY_OR_DEBUG_ASSERT(!ext.isEmpty()) {
        return fileLocation;
    }
    const QFileInfo fileName(fileLocation);
    if (fileName.suffix().isEmpty()) {
        fileLocation += QChar('.') + ext;
    } else if (fileName.suffix() != ext) {
        // Check if fileLocation ends with any of the available extensions
        int pos = 0;
        // Extract all extensions from the filter list
        while ((pos = kFileFilterRegex.indexIn(fileFilters, pos)) != -1) {
            if (ext == kFileFilterRegex.cap(1)) {
                // If it matches chop the current extension incl. dot and break
                fileLocation.chop(ext.length() + 1);
                break;
            }
            pos += kFileFilterRegex.matchedLength();
        }
        fileLocation.append(".").append(ext);
    }
    return fileLocation;
}

QString getFilePathWithVerifiedExtensionFromFileDialog(
        const QString& caption,
        const QString& preSelectedDirectory,
        const QString& fileFilters,
        const QString& preSelectedFileFilter) {
    QString selectedDirectory(preSelectedDirectory);
    QString selectedFileFilter(preSelectedFileFilter);
    QString fileLocation;

    while (true) {
        fileLocation = QFileDialog::getSaveFileName(
                nullptr,
                caption,
                selectedDirectory,
                fileFilters,
                &selectedFileFilter);
        // Exit method if user cancelled the save dialog.
        if (fileLocation.isEmpty()) {
            break;
        }
        const QString fileLocationAdjusted = filePathWithSelectedExtension(
                fileLocation,
                selectedFileFilter,
                fileFilters);
        // If the file path has the selected suffix we can assume the user either
        // selected a new file or already confirmed overwriting an existing file.
        // Return the file path. Also when the adjusted file path does not exist yet.
        // Otherwise show the dialog again with the repaired file path pre-selected.
        if (fileLocation == fileLocationAdjusted ||
                !QFileInfo::exists(fileLocationAdjusted)) {
            fileLocation = fileLocationAdjusted;
            break;
        } else {
            selectedDirectory = fileLocationAdjusted;
        }
    }
    return fileLocation;
}
