#include "util/fileutils.h"

#include <QRegularExpression>

namespace {
// see https://stackoverflow.com/questions/1976007/what-characters-are-forbidden-in-windows-and-linux-directory-names
const auto kIllegalCharacters = QRegularExpression("([<>:\"\\|\\?\\*]|[\x01-\x1F])");
const auto kDirChars = QRegularExpression("[/\\\\]");
} // namespace

QString FileUtils::safeFilename(const QString& input, const QString& replacement) {
    auto output = QString(input);
    output.replace(kIllegalCharacters, replacement);
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
    output.replace(kIllegalCharacters, fileReplaceChar);
    return output.replace(QChar::Null, fileReplaceChar);
}
