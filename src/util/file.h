#ifndef FILE_H
#define FILE_H

#include <QFile>
#include <QDir>

#include "util/sandbox.h"

class MFile {
  public:
    MFile();
    MFile(const QString& name);
    MFile(const MFile& other);
    virtual ~MFile();

    QFile& file() {
        return m_file;
    }

    const QFile& file() const {
        return m_file;
    }

    MFile& operator=(const MFile& other);

  private:
    QString m_fileName;
    QFile m_file;
    SecurityTokenPointer m_pSecurityToken;
};

class MDir {
  public:
    MDir();
    MDir(const QString& name);
    MDir(const MDir& other);
    virtual ~MDir();

    QDir& dir() {
        return m_dir;
    }

    const QDir& dir() const {
        return m_dir;
    }

    MDir& operator=(const MDir& other);

  private:
    QString m_dirPath;
    QDir m_dir;
    SecurityTokenPointer m_pSecurityToken;
};

#endif /* FILE_H */
