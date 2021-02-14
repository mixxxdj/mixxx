#include "util/sandbox.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QMutexLocker>
#include <QObject>
#include <QtDebug>

#include "util/mac.h"

#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>
#include <Security/SecCode.h>
#include <Security/SecRequirement.h>
#endif

const bool sDebug = false;

QMutex Sandbox::s_mutex(QMutex::Recursive);
bool Sandbox::s_bInSandbox = false;
QSharedPointer<ConfigObject<ConfigValue>> Sandbox::s_pSandboxPermissions;
QHash<QString, SecurityTokenWeakPointer> Sandbox::s_activeTokens;

// static
void Sandbox::checkSandboxed() {
#ifdef __APPLE__
    SecCodeRef secCodeSelf;
    if (SecCodeCopySelf(kSecCSDefaultFlags, &secCodeSelf) == errSecSuccess) {
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
}

void Sandbox::setPermissionsFilePath(const QString& permissionsFile) {
    QMutexLocker locker(&s_mutex);
    s_pSandboxPermissions = QSharedPointer<ConfigObject<ConfigValue>>(
            new ConfigObject<ConfigValue>(permissionsFile));
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

    QMessageBox::question(nullptr,
            title,
            QObject::tr(
                    "Due to Mac Sandboxing, we need your permission to access "
                    "this file:"
                    "\n\n%1\n\n"
                    "After clicking OK, you will see a file picker. "
                    "To give Mixxx permission, you must select '%2' to "
                    "proceed. "
                    "If you do not want to grant Mixxx access click Cancel on "
                    "the file picker. "
                    "We're sorry for this inconvenience.\n\n"
                    "To abort this action, press Cancel on the file dialog.")
                    .arg(canonicalPath, info.fileName()));

    QString result;
    QFileInfo resultInfo;
    while (true) {
        if (info.isFile()) {
            result = QFileDialog::getOpenFileName(nullptr, title, canonicalPath);
        } else if (info.isDir()) {
            result = QFileDialog::getExistingDirectory(nullptr, title, canonicalPath);
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
                nullptr, title, QObject::tr("You selected the wrong file. To grant Mixxx access, "
                                            "please select the file '%1'. If you do not want to "
                                            "continue, press Cancel.")
                                        .arg(info.fileName()));
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
    if (s_pSandboxPermissions == nullptr) {
        return false;
    }

#ifdef __APPLE__
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
    if (s_pSandboxPermissions == nullptr) {
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
#ifdef __APPLE__
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
#else
    Q_UNUSED(canonicalPath);
    Q_UNUSED(bookmarkBase64);
#endif

    return SecurityTokenPointer();
}

QString Sandbox::migrateOldSettings() {
    // QStandardPaths::DataLocation returns a different location depending on whether the build
    // is signed (and therefore sandboxed with the hardened runtime), so use the absolute path
    // that the sandbox uses regardless of whether this build is actually sandboxed.
    // Otherwise, developers would need to run with --settingsPath every time or symlink
    // to use the same settings directory with signed and unsigned builds.

    // QDir::homePath returns a path inside the sandbox when running sandboxed
    QString homePath = QLatin1String("/Users/") + qgetenv("USER");
    if (qEnvironmentVariableIsEmpty("USER") || qgetenv("USER").contains("/")) {
        qCritical() << "Cannot find home directory (USER environment variable invalid)";
        return QString();
    }

    QDir homeDir(homePath);
    if (!homeDir.exists()) {
        qCritical() << "Home directory does not exist" << homePath;
        return QString();
    }

    // The parent of the sandboxed path needs to be created before the legacySettingsPath
    // can be moved there. This is not necessary when running in a sandbox because macOS
    // automatically creates it.
    QString sandboxedParentPath = homePath +
            QLatin1String(
                    "/Library/Containers/org.mixxx.mixxx/Data/Library/"
                    "Application Support");
    QString sandboxedPath = sandboxedParentPath + QLatin1String("/Mixxx");
    QDir sandboxedDir(sandboxedPath);

    QString legacySettingsPath = homePath + QLatin1String("/Library/Application Support/Mixxx");
    // The user has no settings from Mixxx < 2.3.0, so there is no migration to do.
    if (!QDir(legacySettingsPath).exists()) {
        return sandboxedPath;
    }

    // The user already has settings in the sandboxed path, so there is no migration to do.
    if (sandboxedDir.exists() && !sandboxedDir.isEmpty()) {
        return sandboxedPath;
    }

    // Sandbox::askForAccess cannot be used here because it depends on settings being
    // initialized. There is no need to store the bookmark anyway because this is a
    // one time process.
    QString title = QObject::tr("Upgrading old Mixxx settings");
    QMessageBox::information(nullptr,
            title,
            QObject::tr(
                    "Due to macOS sandboxing, Mixxx needs your permission "
                    "to access your music library and settings from Mixxx "
                    "versions before 2.3.0. After clicking OK, you will see a "
                    "file selection dialog. "
                    "\n\n"
                    "To allow Mixxx to use your old library and settings, "
                    "click the Open button in the file selection dialog. "
                    "Mixxx will then move your old settings into the sandbox. "
                    "This only needs to be done once."
                    "\n\n"
                    "If you do not want to grant Mixxx access, click Cancel "
                    "on the file picker. Mixxx will create a new music library"
                    "and use default settings."));

    QString result = QFileDialog::getExistingDirectory(
            nullptr,
            title,
            legacySettingsPath);
    if (result != legacySettingsPath) {
        qInfo() << "Sandbox::migrateOldSettings: User declined to migrate old settings from"
                << legacySettingsPath << "User selected" << result;
        return sandboxedPath;
    }

#ifdef __APPLE__
    CFURLRef url = CFURLCreateWithFileSystemPath(
            kCFAllocatorDefault, QStringToCFString(legacySettingsPath), kCFURLPOSIXPathStyle, true);
    if (url) {
        CFErrorRef error = NULL;
        if (s_bInSandbox) {
            // Request permissions to the old unsandboxed sandboxed settings path
            // and move the directory into the sandbox
            CFDataRef bookmark = CFURLCreateBookmarkData(
                    kCFAllocatorDefault,
                    url,
                    kCFURLBookmarkCreationWithSecurityScope,
                    nil,
                    nil,
                    &error);
            CFRelease(url);
            if (bookmark) {
                QFile oldSettings(legacySettingsPath);
                if (oldSettings.rename(sandboxedPath)) {
                    qInfo() << "Sandbox::migrateOldSettings: Successfully "
                               "migrated old settings from"
                            << legacySettingsPath << "to new path" << sandboxedPath;
                } else {
                    qWarning() << "Sandbox::migrateOldSettings: Failed to migrate "
                                  "old settings from"
                               << legacySettingsPath
                               << "to new path" << sandboxedPath;
                }
                CFRelease(bookmark);
            } else {
                qWarning() << "Sandbox::migrateOldSettings: Failed to access old "
                              "settings path"
                           << legacySettingsPath
                           << "Cannot migrate to new path" << sandboxedPath;
            }
        } else {
            // Move old unsandboxed settings directory into the sandbox

            // Ensure the parent directory of the destination path exists, otherwise
            // moving to the new path will fail.
            QDir sandboxedParentDir(sandboxedParentPath);
            if (!sandboxedParentDir.exists()) {
                if (!sandboxedParentDir.mkpath(sandboxedParentPath)) {
                    qWarning() << "Could not create sandboxed application data directory"
                               << sandboxedParentPath;
                }
            }

            QFile oldSettings(legacySettingsPath);
            if (oldSettings.rename(sandboxedPath)) {
                qInfo() << "Sandbox::migrateOldSettings: Successfully "
                           "migrated old settings from"
                        << legacySettingsPath << "to new path"
                        << sandboxedPath;
            } else {
                qWarning() << "Sandbox::migrateOldSettings: Failed to migrate old settings from"
                           << legacySettingsPath << "to new path" << sandboxedPath;
            }
        }
    }
#endif
    return sandboxedPath;
}

#ifdef __APPLE__
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
#ifdef __APPLE__
    if (sDebug) {
        qDebug() << "~SandboxSecurityToken" << m_path;
    }
    if (m_url) {
        CFURLStopAccessingSecurityScopedResource(m_url);
        CFRelease(m_url);
        m_url = 0;
    }
#endif
}
