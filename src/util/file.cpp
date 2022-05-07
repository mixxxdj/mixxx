#include "util/file.h"

#include <QFileDialog>
#include <QRegularExpression>

namespace {

const QRegularExpression kExtractExtensionRegex(R"(\(\*\.(.*)\)$)");

} //anonymous namespace

QString filePathWithSelectedExtension(const QString& fileLocationInput,
        const QString& fileFilter) {
    if (fileLocationInput.isEmpty()) {
        return {};
    }
    QString fileLocation = fileLocationInput;
    if (fileFilter.isEmpty()) {
        return fileLocation;
    }

    // Extract 'ext' from QFileDialog file filter string 'Funky type (*.ext)'
    const auto extMatch = kExtractExtensionRegex.match(fileFilter);
    const QString ext = extMatch.captured(1);
    if (ext.isNull()) {
        return fileLocation;
    }
    const QFileInfo fileName(fileLocation);
    if (!ext.isEmpty() && fileName.suffix() != ext) {
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
                selectedFileFilter);
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
