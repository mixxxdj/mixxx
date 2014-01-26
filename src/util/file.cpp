#include "util/file.h"

MFile::MFile() {
}

MFile::MFile(const QString& file)
        : m_fileName(file),
          m_file(file),
          m_pSecurityToken(Sandbox::openSecurityToken(m_file, true)) {
}

MFile::MFile(const MFile& other)
        : m_fileName(other.m_fileName),
          m_file(m_fileName),
          m_pSecurityToken(other.m_pSecurityToken) {
}

MFile::~MFile() {
}

MFile& MFile::operator=(const MFile& other) {
    m_fileName = other.m_fileName;
    m_file.setFileName(m_fileName);
    m_pSecurityToken = other.m_pSecurityToken;
    return *this;
}

bool MFile::canAccess() {
    QFileInfo info(m_file);
    return Sandbox::canAccessPath(info.canonicalFilePath());
}

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
    return Sandbox::canAccessPath(m_dir.canonicalPath());
}
