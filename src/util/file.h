#ifndef FILE_H
#define FILE_H

#include <QDir>
#include <QFile>
#include <QString>

#include "util/sandbox.h"

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

    SecurityTokenPointer token() {
        return m_pSecurityToken;
    }

    bool canAccess();

    MDir& operator=(const MDir& other);

  private:
    QString m_dirPath;
    QDir m_dir;
    SecurityTokenPointer m_pSecurityToken;
};

#endif /* FILE_H */
