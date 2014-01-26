#include "util/sandbox.h"

#include <QtDebug>
#include <QFileInfo>
#include <QFileDialog>
#include <QObject>

#include "util/mac.h"

#ifdef Q_OS_MAC
#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>
#if __MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_6
#include <Security/SecCode.h>
#include <Security/SecRequirement.h>
#endif
#endif

bool Sandbox::s_bInSandbox = false;
ConfigObject<ConfigValue>* Sandbox::s_pSandboxPermissions = NULL;

// static
void Sandbox::initialize(const QString& permissionsFile) {
    s_pSandboxPermissions = new ConfigObject<ConfigValue>(permissionsFile);

#ifdef Q_OS_MAC
#if __MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_6
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
    ConfigObject<ConfigValue>* pSandboxPermissions = s_pSandboxPermissions;
    s_pSandboxPermissions = NULL;
    if (pSandboxPermissions) {
        pSandboxPermissions->Save();
        delete pSandboxPermissions;
    }
}

// static
bool Sandbox::askForAccess(const QString& canonicalPath) {
    qDebug() << "Sandbox::askForAccess" << canonicalPath;
    QFileInfo info(canonicalPath);

    // We always want read/write access because we wouldn't want to have to
    // re-ask for access in the future if we need to write.
    if (canAccessFile(info)) {
        return true;
    }

    qDebug() << "Sandbox: Requesting user access to" << canonicalPath;
    QString title = QObject::tr("Mixxx Needs Access to: %1")
            .arg(info.fileName());

    QMessageBox::StandardButton button = QMessageBox::question(
        NULL, title,
        QObject::tr(
            "Due to Mac Sandboxing, we need your permission to access this file:"
            "\n %1\n"
            "After clicking OK, you will see a file picker. "
            "To give Mixxx permission, you must select '%2' to proceed."
            "If you do not want to grant Mixxx access, click Cancel on the file dialog."
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
            qDebug() << "Sandbox: User rejected access to" << canonicalPath;
            return false;
        }

        qDebug() << "Sandbox: User selected" << result;
        resultInfo = QFileInfo(result);
        if (resultInfo == info) {
            break;
        }

        qDebug() << "User selected the wrong file.";
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
    qDebug() << "createSecurityToken" << canonicalPath << isDirectory;
    if (s_pSandboxPermissions == NULL) {
        return false;
    }
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
            qDebug() << "Failed to create security-scoped bookmark for" << canonicalPath;
            if (error != NULL) {
                qDebug() << "Error:" << CFStringToQString(CFErrorCopyDescription(error));
            }
        }
    } else {
        qDebug() << "Failed to create security-scoped bookmark URL for" << canonicalPath;
    }
    return false;
}

// static
SecurityTokenPointer Sandbox::openSecurityToken(const QFileInfo& file, bool create) {
    qDebug() << "openSecurityToken QFileInfo" << file.canonicalFilePath() << create;
    if (s_pSandboxPermissions == NULL) {
        return SecurityTokenPointer();
    }

    if (file.isDir()) {
        return openSecurityToken(QDir(file.canonicalFilePath()), create);
    }

    // First, check for a bookmark of the key itself.
    ConfigKey key = keyForCanonicalPath(file.canonicalFilePath());
    if (s_pSandboxPermissions->exists(key)) {
        return openTokenFromBookmark(
                file.canonicalFilePath(),
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
                file.canonicalFilePath(),
                s_pSandboxPermissions->getValueString(key));
    }
    return SecurityTokenPointer();
}

// static
SecurityTokenPointer Sandbox::openSecurityToken(const QDir& dir, bool create) {
    qDebug() << "openSecurityToken QDir" << dir.canonicalPath() << create;
    if (s_pSandboxPermissions == NULL) {
        return SecurityTokenPointer();
    }

    QDir walkDir = dir;
    ConfigKey key = keyForCanonicalPath(walkDir.canonicalPath());

    while (!s_pSandboxPermissions->exists(key)) {
        // There's nothing higher. Bail.
        if (!walkDir.cdUp()) {
            if (create && createSecurityToken(dir.canonicalPath(), true)) {
                key = keyForCanonicalPath(dir.canonicalPath());
                return openTokenFromBookmark(
                        dir.canonicalPath(),
                        s_pSandboxPermissions->getValueString(key));
            }
            return SecurityTokenPointer();
        }

        key = keyForCanonicalPath(walkDir.canonicalPath());
    }

    // At this point, key is present in s_pSandboxPermissions.
    return openTokenFromBookmark(dir.canonicalPath(),
                                 s_pSandboxPermissions->getValueString(key));
}

SecurityTokenPointer Sandbox::openTokenFromBookmark(const QString& canonicalPath,
                                                     const QString& bookmarkBase64) {
    QByteArray bookmarkBA = QByteArray::fromBase64(bookmarkBase64.toLatin1());
#ifdef Q_OS_MAC
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
            qDebug() << "Error creating URL from bookmark data:"
                     << CFStringToQString(CFErrorCopyDescription(error));
        }
        CFRelease(bookmarkData);
        if (url != NULL) {
            if (!CFURLStartAccessingSecurityScopedResource(url)) {
                qDebug() << "CFURLStartAccessingSecurityScopedResource failed for"
                         << canonicalPath;
            } else {
                return SecurityTokenPointer(new SandboxSecurityToken(
                        canonicalPath, url));
            }
        } else {
            qDebug() << "Cannot resolve security-scoped bookmark for" << canonicalPath;
        }
    }
#endif
    return SecurityTokenPointer();
}

#ifdef Q_OS_MAC
SandboxSecurityToken::SandboxSecurityToken(const QString& path, CFURLRef url)
        : m_path(path),
          m_url(url) {
    if (m_url) {
        qDebug() << "SandboxSecurityToken successfully created for" << path;
    }
}
#endif

SandboxSecurityToken::~SandboxSecurityToken() {
#ifdef Q_OS_MAC
    qDebug() << "~SandboxSecurityToken" << m_path;
    if (m_url) {
        CFURLStopAccessingSecurityScopedResource(m_url);
        CFRelease(m_url);
        m_url = 0;
    }
#endif
}
