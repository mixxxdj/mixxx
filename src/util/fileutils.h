#include <QString>

namespace {
const QString kDefaultFileReplacementCharacter = QString("#");
const QString kDefaultDirReplacementCharacter = QString("-");
} // namespace

class FileUtils {
  public:
    // returns a filename that is safe on all platforms and does not contain
    // unwanted characters like newline
    static QString safeFilename(const QString& input,
            const QString& replacement = kDefaultFileReplacementCharacter);
    static QString replaceDirChars(const QString& input,
            const QString& replacement = kDefaultDirReplacementCharacter);
    static QString escapeFileName(const QString& input,
            const QString& fileReplaceChar = kDefaultFileReplacementCharacter,
            const QString& dirReplaceChar = kDefaultDirReplacementCharacter);
};
