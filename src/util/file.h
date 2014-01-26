#ifndef FILE_H
#define FILE_H

#include <QFile>
#include <QDir>

#include "util/sandbox.h"

class MFile {
  public:
    MFile();
    MFile(const QString& name);
    virtual ~MFile();

    QFile& file();

  private:
    QFile m_file;
    SandboxSecurityToken* m_pSecurityToken;
};

class MDir {
  public:
    MDir();
    MDir(const QString& name);
    virtual ~MDir();

    QDir& dir() {
        return m_dir;
    }

  private:
    QDir m_dir;
    SandboxSecurityToken* m_pSecurityToken;
};

#endif /* FILE_H */
