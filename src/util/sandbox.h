#ifndef SANDBOX_H
#define SANDBOX_H

#include <unistd.h>

#include <QFile>
#include <QDir>
#include <QFileInfo>

#include "configobject.h"
#include "util/singleton.h"

#ifdef Q_OS_MAC
#include <CoreFoundation/CFURL.h>
#endif

struct SandboxSecurityToken {
    ~SandboxSecurityToken();
    QString m_path;
#ifdef Q_OS_MAC
    SandboxSecurityToken(const QString& path, CFURLRef url);
    CFURLRef m_url;
#endif
};

class Sandbox : public Singleton<Sandbox> {
  public:
    Sandbox();
    ~Sandbox();

    void setPermissionsFile(const QString& file);

    // Returns true if we are in a sandbox.
    bool enabled() const {
        return m_bInSandbox;
    }

    // Prompt the user to give us access to the path with an open-file dialog.
    bool askForAccess(const QString& canonicalPath);

    bool canAccessFile(const QFileInfo& file) {
        SandboxSecurityToken* pToken = openSecurityToken(file, true);
        bool result = canAccessPath(file.absoluteFilePath());
        closeSecurityToken(pToken);
        return result;
    }

    bool canAccessFile(const QDir& dir) {
        SandboxSecurityToken* pToken = openSecurityToken(dir, true);
        bool result = canAccessPath(dir.canonicalPath());
        closeSecurityToken(pToken);
        return result;
    }

    bool createSecurityToken(const QFileInfo& info) {
        return createSecurityToken(info.canonicalFilePath(), info.isDir());
    }

    bool createSecurityToken(const QDir& dir) {
        return createSecurityToken(dir.canonicalPath(), true);
    }

    SandboxSecurityToken* openSecurityToken(const QFileInfo& info, bool create);
    SandboxSecurityToken* openSecurityToken(const QDir& dir, bool create);


    bool closeSecurityToken(SandboxSecurityToken* pToken);

  private:
    ConfigKey keyForCanonicalPath(const QString& canonicalPath) const;
    SandboxSecurityToken* openTokenFromBookmark(const QString& canonicalPath,
                                                const QString& bookmarkBase64);
    bool createSecurityToken(const QString& canonicalPath, bool isDirectory);

    bool canAccessPath(const QString& canonicalPath) const {
        QByteArray pathEncoded = canonicalPath.toLocal8Bit();
        return access(pathEncoded.constData(), R_OK) == 0;
    }

    bool m_bInSandbox;
    ConfigObject<ConfigValue>* m_pSandboxPermissions;
};


#endif /* SANDBOX_H */
