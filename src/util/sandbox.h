#ifndef SANDBOX_H
#define SANDBOX_H

#include <unistd.h>

#include <QFile>
#include <QDir>
#include <QFileInfo>

#include "configobject.h"

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

class Sandbox {
  public:
    static void initialize(const QString& permissionsFile);
    static void shutdown();

    // Returns true if we are in a sandbox.
    static bool enabled() {
        return s_bInSandbox;
    }

    // Prompt the user to give us access to the path with an open-file dialog.
    static bool askForAccess(const QString& canonicalPath);

    static bool canAccessFile(const QFileInfo& file) {
        SandboxSecurityToken* pToken = openSecurityToken(file, true);
        bool result = canAccessPath(file.absoluteFilePath());
        closeSecurityToken(pToken);
        return result;
    }

    static bool canAccessFile(const QDir& dir) {
        SandboxSecurityToken* pToken = openSecurityToken(dir, true);
        bool result = canAccessPath(dir.canonicalPath());
        closeSecurityToken(pToken);
        return result;
    }

    static bool createSecurityToken(const QFileInfo& info) {
        return createSecurityToken(info.canonicalFilePath(), info.isDir());
    }

    static bool createSecurityToken(const QDir& dir) {
        return createSecurityToken(dir.canonicalPath(), true);
    }

    static SandboxSecurityToken* openSecurityToken(const QFileInfo& info, bool create);
    static SandboxSecurityToken* openSecurityToken(const QDir& dir, bool create);
    static bool closeSecurityToken(SandboxSecurityToken* pToken);

  private:
    Sandbox() {}

    static ConfigKey keyForCanonicalPath(const QString& canonicalPath);
    static SandboxSecurityToken* openTokenFromBookmark(const QString& canonicalPath,
                                                       const QString& bookmarkBase64);
    static bool createSecurityToken(const QString& canonicalPath, bool isDirectory);

    static bool canAccessPath(const QString& canonicalPath) {
        QByteArray pathEncoded = canonicalPath.toLocal8Bit();
        return access(pathEncoded.constData(), R_OK) == 0;
    }

    static bool s_bInSandbox;
    static ConfigObject<ConfigValue>* s_pSandboxPermissions;
};


#endif /* SANDBOX_H */
