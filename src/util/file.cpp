#include "util/file.h"

#include <QRegExp>

namespace {
//const auto kIllegalCharacters = QRegExp("[\0/<>:\"\\|\\?\\*]");
// see https://stackoverflow.com/questions/1976007/what-characters-are-forbidden-in-windows-and-linux-directory-names
const auto kIllegalCharacters = QRegExp("([/<>:\"\\\\\\|\\?\\*]|[\x01-\x1F])");
} // namespace

MDir::MDir() {
}

MDir::MDir(const QString& path)
        : m_dirPath(path),
          m_dir(path),
          m_pSecurityToken(Sandbox::openSecurityToken(m_dir, true)) {
}

MDir::MDir(const MDir& other)
        : m_dirPath(other.m_dirPath),
          m_dir(m_dirPath),
          m_pSecurityToken(other.m_pSecurityToken) {
}

MDir::~MDir() {
}

MDir& MDir::operator=(const MDir& other) {
    m_dirPath = other.m_dirPath;
    m_dir = QDir(m_dirPath);
    m_pSecurityToken = other.m_pSecurityToken;
    return *this;
}

bool MDir::canAccess() {
    return Sandbox::canAccessFile(m_dir);
}

QString FileUtils::safeFilename(const QString& input, const QString& replacement) {
    auto output = QString(input);
    output.replace(kIllegalCharacters, replacement);
    return output.replace(QChar::Null, replacement);
}
