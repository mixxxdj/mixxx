#pragma once

#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QSharedPointer>
#include <QHash>
#include <QMutex>

#include "preferences/configobject.h"

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

// Reference counted pointer to SandboxSecurityToken.
typedef QSharedPointer<SandboxSecurityToken> SecurityTokenPointer;
typedef QWeakPointer<SandboxSecurityToken> SecurityTokenWeakPointer;

class Sandbox {
  public:
    static void checkSandboxed();
    static void setPermissionsFilePath(const QString& permissionsFile);
    static void shutdown();

    static QString migrateOldSettings();

    // Returns true if we are in a sandbox.
    static bool enabled() {
        return s_bInSandbox;
    }

    // Prompt the user to give us access to the path with an open-file dialog.
    static bool askForAccess(const QString& canonicalPath);

    static bool canAccessFile(const QFileInfo& file) {
        SecurityTokenPointer pToken = openSecurityToken(file, true);
        return file.isReadable();
    }

    static bool canAccessFile(const QDir& dir) {
        SecurityTokenPointer pToken = openSecurityToken(dir, true);
        QFileInfo info(dir.canonicalPath());
        return info.isReadable();
    }

    static bool createSecurityToken(const QFileInfo& info) {
        return createSecurityToken(info.canonicalFilePath(), info.isDir());
    }

    static bool createSecurityToken(const QDir& dir) {
        return createSecurityToken(dir.canonicalPath(), true);
    }

    static SecurityTokenPointer openSecurityToken(const QFileInfo& info, bool create);
    static SecurityTokenPointer openSecurityToken(const QDir& dir, bool create);

  private:
    Sandbox() {}

    static ConfigKey keyForCanonicalPath(const QString& canonicalPath);

    // Must hold s_mutex to call this.
    static SecurityTokenPointer openTokenFromBookmark(const QString& canonicalPath,
                                                      const QString& bookmarkBase64);

    // Creates a security token. s_mutex is not needed for this method.
    static bool createSecurityToken(const QString& canonicalPath, bool isDirectory);

    static QMutex s_mutex;
    static bool s_bInSandbox;
    static QSharedPointer<ConfigObject<ConfigValue>> s_pSandboxPermissions;
    static QHash<QString, SecurityTokenWeakPointer> s_activeTokens;
};
