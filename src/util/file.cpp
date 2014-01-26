#include "util/file.h"

MFile::MFile()
        : m_pSecurityToken(NULL) {
}

MFile::MFile(const QString& file)
        : m_file(file),
          m_pSecurityToken(Sandbox::instance()->openSecurityToken(m_file, true)) {
}

MFile::~MFile() {
    Sandbox::instance()->closeSecurityToken(m_pSecurityToken);
}

MDir::MDir()
        : m_pSecurityToken(NULL) {
}

MDir::MDir(const QString& path)
        : m_dir(path),
          m_pSecurityToken(Sandbox::instance()->openSecurityToken(m_dir, true)) {
}

MDir::~MDir() {
    Sandbox::instance()->closeSecurityToken(m_pSecurityToken);
}
