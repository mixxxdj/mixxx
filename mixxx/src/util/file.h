#pragma once

#include <QString>

// Check if the extension from the file filter was added to the file base name.
// Otherwise add it manually.
// Works around https://bugreports.qt.io/browse/QTBUG-27186
QString filePathWithSelectedExtension(const QString& fileLocationInput,
        const QString& fileFilter,
        const QString& fileFilters);

// Due to Qt bug https://bugreports.qt.io/browse/QTBUG-27186 we may need to
// manually add the selected extension to the selected file name.
// Unfortunately, this would bypass Qt's file overwrite dialog. To avoid
// creating our own file overwrite dialog we show the file dialog again with
// the repaired file path pre-selected so Qt's overwrite dialog can kick in.
QString getFilePathWithVerifiedExtensionFromFileDialog(
        const QString& caption,
        const QString& preSelectedDirectory,
        const QString& fileFilters,
        const QString& preSelectedFileFilter);
