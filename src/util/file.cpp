#include "util/file.h"

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
