#include "util/sandbox.h"

#include <QtDebug>
#include <QFileInfo>
#include <QFileDialog>
#include <QObject>
#include <QMutexLocker>

#include "util/mac.h"

#ifdef Q_OS_MAC
#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>
#if __MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_7
#include <Security/SecCode.h>
#include <Security/SecRequirement.h>
#endif
#endif

const bool sDebug = false;

QMutex Sandbox::s_mutex(QMutex::Recursive);
bool Sandbox::s_bInSandbox = false;
QSharedPointer<ConfigObject<ConfigValue>> Sandbox::s_pSandboxPermissions;
QHash<QString, SecurityTokenWeakPointer> Sandbox::s_activeTokens;

// static
void Sandbox::initialize(const QString& permissionsFile) {
    QMutexLocker locker(&s_mutex);
    s_pSandboxPermissions = QSharedPointer<ConfigObject<ConfigValue>>(
        new ConfigObject<ConfigValue>(permissionsFile));

#ifdef Q_OS_MAC
#if __MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_7
    // If we are running on at least 10.7.0 and have the com.apple.security.app-sandbox
    // entitlement, we are in a sandbox
    SInt32 version = 0;
    Gestalt(gestaltSystemVersion, &version);
    SecCodeRef secCodeSelf;
    if (version >= 0x1070 && SecCodeCopySelf(kSecCSDefaultFlags, &secCodeSelf) == errSecSuccess) {
        SecRequirementRef sandboxReq;
        CFStringRef entitlement = CFSTR("entitlement [\"com.apple.security.app-sandbox\"]");
        if (SecRequirementCreateWithString(entitlement, kSecCSDefaultFlags,
                                           &sandboxReq) == errSecSuccess) {
            if (SecCodeCheckValidity(secCodeSelf, kSecCSDefaultFlags,
                                     sandboxReq) == errSecSuccess) {
                s_bInSandbox = true;
            }
            CFRelease(sandboxReq);
        }
        CFRelease(secCodeSelf);
    }
#endif
#endif
}

// static
void Sandbox::shutdown() {
    QMutexLocker locker(&s_mutex);
    QSharedPointer<ConfigObject<ConfigValue>> pSandboxPermissions = s_pSandboxPermissions;
    s_pSandboxPermissions.clear();
    if (pSandboxPermissions) {
        pSandboxPermissions->save();
    }
}

// static
bool Sandbox::askForAccess(const QString& canonicalPath) {
    if (sDebug) {
        qDebug() << "Sandbox::askForAccess" << canonicalPath;
    }
    if (!enabled()) {
        // Pretend we have access.
        return true;
    }

    QFileInfo info(canonicalPath);
    // We always want read/write access because we wouldn't want to have to
    // re-ask for access in the future if we need to write.
    if (canAccessFile(info)) {
        return true;
    }

    if (sDebug) {
        qDebug() << "Sandbox: Requesting user access to" << canonicalPath;
    }
    QString title = QObject::tr("Mixxx Needs Access to: %1")
            .arg(info.fileName());

    QMessageBox::question(
        NULL, title,
        QObject::tr(
            "Due to Mac Sandboxing, we need your permission to access this file:"
            "\n\n%1\n\n"
            "After clicking OK, you will see a file picker. "
            "To give Mixxx permission, you must select '%2' to proceed. "
            "If you do not want to grant Mixxx access click Cancel on the file picker. "
            "We're sorry for this inconvenience.\n\n"
            "To abort this action, press Cancel on the file dialog.")
        .arg(canonicalPath, info.fileName()));

    QString result;
    QFileInfo resultInfo;
    while (true) {
        if (info.isFile()) {
            result = QFileDialog::getOpenFileName(NULL, title, canonicalPath);
        } else if (info.isDir()) {
            result = QFileDialog::getExistingDirectory(NULL, title, canonicalPath);
        }

        if (result.isNull()) {
            if (sDebug) {
                qDebug() << "Sandbox: User rejected access to" << canonicalPath;
            }
            return false;
        }

        if (sDebug) {
            qDebug() << "Sandbox: User selected" << result;
        }
        resultInfo = QFileInfo(result);
        if (resultInfo == info) {
            break;
        }

        if (sDebug) {
            qDebug() << "User selected the wrong file.";
        }
        QMessageBox::question(
            NULL, title,
            QObject::tr("You selected the wrong file. To grant Mixxx access, "
                        "please select the file '%1'. If you do not want to "
                        "continue, press Cancel.").arg(info.fileName()));
    }

    return createSecurityToken(resultInfo);
}

// static
ConfigKey Sandbox::keyForCanonicalPath(const QString& canonicalPath) {
    return ConfigKey("[OSXBookmark]",
                     QString(canonicalPath.toLocal8Bit().toBase64()));
}

// static
bool Sandbox::createSecurityToken(const QString& canonicalPath,
                                  bool isDirectory) {
    if (sDebug) {
        qDebug() << "createSecurityToken" << canonicalPath << isDirectory;
    }
    if (!enabled()) {
        return false;
    }
    QMutexLocker locker(&s_mutex);
    if (s_pSandboxPermissions == NULL) {
        return false;
    }

#ifdef Q_OS_MAC
#if __MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_7
    CFURLRef url = CFURLCreateWithFileSystemPath(
            kCFAllocatorDefault, QStringToCFString(canonicalPath),
            kCFURLPOSIXPathStyle, isDirectory);
    if (url) {
        CFErrorRef error = NULL;
        CFDataRef bookmark = CFURLCreateBookmarkData(
                kCFAllocatorDefault, url,
                kCFURLBookmarkCreationWithSecurityScope, nil, nil, &error);
        CFRelease(url);
        if (bookmark) {
            QByteArray bookmarkBA = QByteArray(
                    reinterpret_cast<const char*>(CFDataGetBytePtr(bookmark)),
                    CFDataGetLength(bookmark));

            QString bookmarkBase64 = QString(bookmarkBA.toBase64());

            s_pSandboxPermissions->set(keyForCanonicalPath(canonicalPath),
                                       bookmarkBase64);
            CFRelease(bookmark);
            return true;
        } else {
            if (sDebug) {
                qDebug() << "Failed to create security-scoped bookmark for" << canonicalPath;
                if (error != NULL) {
                    qDebug() << "Error:" << CFStringToQString(CFErrorCopyDescription(error));
                }
            }
        }
    } else {
        if (sDebug) {
            qDebug() << "Failed to create security-scoped bookmark URL for" << canonicalPath;
        }
    }
#endif
#endif
    return false;
}

// static
SecurityTokenPointer Sandbox::openSecurityToken(const QFileInfo& file, bool create) {
    const QString& canonicalFilePath = file.canonicalFilePath();
    if (sDebug) {
        qDebug() << "openSecurityToken QFileInfo" << canonicalFilePath << create;
    }

    if (!enabled()) {
        return SecurityTokenPointer();
    }

    QMutexLocker locker(&s_mutex);
    if (s_pSandboxPermissions == NULL) {
        return SecurityTokenPointer();
    }

    QHash<QString, SecurityTokenWeakPointer>::iterator it = s_activeTokens
            .find(canonicalFilePath);
    if (it != s_activeTokens.end()) {
        SecurityTokenPointer pToken(it.value());
        if (pToken) {
            if (sDebug) {
                qDebug() << "openSecurityToken QFileInfo" << canonicalFilePath
                         << "using cached token for" << pToken->m_path;
            }
            return pToken;
        }
    }

    if (file.isDir()) {
        return openSecurityToken(QDir(canonicalFilePath), create);
    }

    // First, check for a bookmark of the key itself.
    ConfigKey key = keyForCanonicalPath(canonicalFilePath);
    if (s_pSandboxPermissions->exists(key)) {
        return openTokenFromBookmark(
                canonicalFilePath,
                s_pSandboxPermissions->getValueString(key));
    }

    // Next, try to open a bookmark for an existing directory but don't create a
    // bookmark.
    SecurityTokenPointer pDirToken = openSecurityToken(file.dir(), false);
    if (!pDirToken.isNull()) {
        return pDirToken;
    }

    if (!create) {
        return SecurityTokenPointer();
    }

    // Otherwise, try to create a token.
    bool created = createSecurityToken(file);

    if (created) {
        return openTokenFromBookmark(
                canonicalFilePath,
                s_pSandboxPermissions->getValueString(key));
    }
    return SecurityTokenPointer();
}

// static
SecurityTokenPointer Sandbox::openSecurityToken(const QDir& dir, bool create) {
    QDir walkDir = dir;
    QString walkDirCanonicalPath = walkDir.canonicalPath();
    if (sDebug) {
        qDebug() << "openSecurityToken QDir" << walkDirCanonicalPath << create;
    }

    if (!enabled()) {
        return SecurityTokenPointer();
    }

    QMutexLocker locker(&s_mutex);
    if (s_pSandboxPermissions.isNull()) {
        return SecurityTokenPointer();
    }

    while (true) {
        // Look for a valid token in the cache.
        QHash<QString, SecurityTokenWeakPointer>::iterator it = s_activeTokens
                .find(walkDirCanonicalPath);
        if (it != s_activeTokens.end()) {
            SecurityTokenPointer pToken(it.value());
            if (pToken) {
                if (sDebug) {
                    qDebug() << "openSecurityToken QDir" << walkDirCanonicalPath
                             << "using cached token for" << pToken->m_path;
                }
                return pToken;
            }
        }

        // Next, check if the key exists in the config.
        ConfigKey key = keyForCanonicalPath(walkDirCanonicalPath);
        if (s_pSandboxPermissions->exists(key)) {
            SecurityTokenPointer pToken = openTokenFromBookmark(
                    dir.canonicalPath(),
                    s_pSandboxPermissions->getValueString(key));
            if (pToken) {
                return pToken;
            }
        }

        // Go one step higher and repeat.
        if (!walkDir.cdUp()) {
            // There's nothing higher. Bail.
            break;
        }
        walkDirCanonicalPath = walkDir.canonicalPath();
    }

    // Last chance: Try to create a token for this directory.
    if (create && createSecurityToken(dir.canonicalPath(), true)) {
        ConfigKey key = keyForCanonicalPath(dir.canonicalPath());
        return openTokenFromBookmark(
                dir.canonicalPath(),
                s_pSandboxPermissions->getValueString(key));
    }
    return SecurityTokenPointer();
}

SecurityTokenPointer Sandbox::openTokenFromBookmark(const QString& canonicalPath,
                                                    const QString& bookmarkBase64) {
#ifdef Q_OS_MAC
#if __MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_7
    QByteArray bookmarkBA = QByteArray::fromBase64(bookmarkBase64.toLatin1());
    if (!bookmarkBA.isEmpty()) {
        CFDataRef bookmarkData = CFDataCreate(
                kCFAllocatorDefault, reinterpret_cast<const UInt8*>(bookmarkBA.constData()),
                bookmarkBA.length());
        Boolean stale;
        CFErrorRef error = NULL;
        CFURLRef url = CFURLCreateByResolvingBookmarkData(
                kCFAllocatorDefault, bookmarkData,
                kCFURLBookmarkResolutionWithSecurityScope, NULL, NULL,
                &stale, &error);
        if (error != NULL) {
            if (sDebug) {
                qDebug() << "Error creating URL from bookmark data:"
                         << CFStringToQString(CFErrorCopyDescription(error));
            }
        }
        CFRelease(bookmarkData);
        if (url != NULL) {
            if (!CFURLStartAccessingSecurityScopedResource(url)) {
                if (sDebug) {
                    qDebug() << "CFURLStartAccessingSecurityScopedResource failed for"
                             << canonicalPath;
                }
            } else {
                SecurityTokenPointer pToken = SecurityTokenPointer(
                    new SandboxSecurityToken(canonicalPath, url));
                s_activeTokens[canonicalPath] = pToken;
                return pToken;
            }
        } else {
            if (sDebug) {
                qDebug() << "Cannot resolve security-scoped bookmark for" << canonicalPath;
            }
        }
    }
#endif
#else
    Q_UNUSED(canonicalPath);
    Q_UNUSED(bookmarkBase64);
#endif

    return SecurityTokenPointer();
}

#ifdef Q_OS_MAC
SandboxSecurityToken::SandboxSecurityToken(const QString& path, CFURLRef url)
        : m_path(path),
          m_url(url) {
    if (m_url) {
        if (sDebug) {
            qDebug() << "SandboxSecurityToken successfully opened for" << path;
        }
    }
}
#endif

SandboxSecurityToken::~SandboxSecurityToken() {
#ifdef Q_OS_MAC
    if (sDebug) {
        qDebug() << "~SandboxSecurityToken" << m_path;
    }
#if __MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_7
    if (m_url) {
        CFURLStopAccessingSecurityScopedResource(m_url);
        CFRelease(m_url);
        m_url = 0;
    }
#endif
#endif
}
