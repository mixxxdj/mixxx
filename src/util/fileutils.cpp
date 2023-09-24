#include "util/fileutils.h"

#include <QRegularExpression>

namespace {
// see https://stackoverflow.com/questions/1976007/what-characters-are-forbidden-in-windows-and-linux-directory-names
// Note, that the colon can be part of Windows paths starting with drive letters: C:
const auto kIllegalCharacters = QRegularExpression("([<>:\"\\|\\?\\*]|[\x01-\x1F])");
const auto kDirChars = QRegularExpression("[/\\\\]");
} // namespace

QString FileUtils::safeFilename(const QString& input, const QString& replacement) {
    auto output = QString(input);
    output.replace(kDirChars, "/");
    bool windowsDriveLetter = false;
    if (output.size() > 2 && output[0].toUpper() >= 'A' &&
            output[0].toUpper() <= 'Z' && output[1] == ':') {
        windowsDriveLetter = true;
    }
    output.replace(kIllegalCharacters, replacement);
    if (windowsDriveLetter) {
        output[1] = ':';
    }
    return output.replace(QChar::Null, replacement);
}

QString FileUtils::replaceDirChars(const QString& input, const QString& replacement) {
    auto output = QString(input);
    return output.replace(kDirChars, replacement);
}

QString FileUtils::escapeFileName(
        const QString& input,
        const QString& fileReplaceChar,
        const QString& dirReplaceChar) {
    auto output = QString(input);
    output.replace(kDirChars, dirReplaceChar);

    bool windowsDriveLetter = false;
    if (output.size() > 2 && output[0].toUpper() >= 'A' &&
            output[0].toUpper() <= 'Z' && output[1] == ':') {
        windowsDriveLetter = true;
    }
    output.replace(kIllegalCharacters, fileReplaceChar);
    if (windowsDriveLetter) {
        output[1] = ':';
    }

    return output.replace(QChar::Null, fileReplaceChar);
}
