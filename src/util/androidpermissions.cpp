#include "util/androidpermissions.h"

#include <QtCore/private/qandroidextras_p.h>
#include <android/api-level.h>

#include <QCoreApplication>
#include <QDir>
#include <QJniObject>
#include <QStandardPaths>

#include "util/assert.h"

namespace {

bool requestPermission(const QString& permission) {
    if (QtAndroidPrivate::checkPermission(permission).result() ==
            QtAndroidPrivate::Authorized) {
        return true;
    }
    // Qt runs main() on a dedicated thread, not on the Android UI thread, so
    // waiting for the dialog result here does not block the dialog itself.
    const auto result = QtAndroidPrivate::requestPermission(permission).result();
    if (result != QtAndroidPrivate::Authorized) {
        qWarning() << "Android permission denied:" << permission;
        return false;
    }
    return true;
}

} // namespace

namespace mixxx {
namespace android {

bool requestMusicLibraryPermissions() {
    const int apiLevel = android_get_device_api_level();
    bool granted = false;
    if (apiLevel >= 33) {
        granted = requestPermission(
                QStringLiteral("android.permission.READ_MEDIA_AUDIO"));
    } else {
        granted = requestPermission(
                QStringLiteral("android.permission.READ_EXTERNAL_STORAGE"));
    }
    if (apiLevel <= 29) {
        // Writing to shared storage is only possible with this permission on
        // legacy storage devices. A denial is not fatal, Mixxx then falls back
        // to its app-private directory.
        requestPermission(
                QStringLiteral("android.permission.WRITE_EXTERNAL_STORAGE"));
    }
    return granted;
}

bool hasFullExternalStorageAccess() {
    if (android_get_device_api_level() >= 30) {
        return QJniObject::callStaticMethod<jboolean>(
                       "android/os/Environment", "isExternalStorageManager") ==
                JNI_TRUE;
    }
    return QtAndroidPrivate::checkPermission(
                   QStringLiteral("android.permission.WRITE_EXTERNAL_STORAGE"))
                   .result() == QtAndroidPrivate::Authorized;
}

void requestFullExternalStorageAccess() {
    if (android_get_device_api_level() < 30) {
        requestPermission(
                QStringLiteral("android.permission.WRITE_EXTERNAL_STORAGE"));
        return;
    }
    if (hasFullExternalStorageAccess()) {
        return;
    }
    qDebug() << "requesting ACTION_MANAGE_APP_ALL_FILES_ACCESS_PERMISSION";
    const QJniObject action = QJniObject::getStaticObjectField(
            "android/provider/Settings",
            "ACTION_MANAGE_APP_ALL_FILES_ACCESS_PERMISSION",
            "Ljava/lang/String;");
    QJniObject intent("android/content/Intent",
            "(Ljava/lang/String;)V",
            action.object());
    const QJniObject jniPath = QJniObject::fromString(
            QStringLiteral("package:%1").arg(QStringLiteral(ANDROID_PACKAGE_NAME)));
    const QJniObject jniUri = QJniObject::callStaticObjectMethod(
            "android/net/Uri",
            "parse",
            "(Ljava/lang/String;)Landroid/net/Uri;",
            jniPath.object<jstring>());
    intent.callObjectMethod("setData",
            "(Landroid/net/Uri;)Landroid/content/Intent;",
            jniUri.object<jobject>());
    QtAndroidPrivate::startActivity(intent, 0);
}

QString defaultMusicDirectory() {
    // QStandardPaths::MusicLocation resolves to the shared music directory
    // (usually /storage/emulated/0/Music) when the app may access it and to
    // the app-private one otherwise, so it works on every API level and on
    // devices that mount the primary volume elsewhere.
    QString path = QStandardPaths::writableLocation(QStandardPaths::MusicLocation);
    if (path.isEmpty()) {
        path = QStringLiteral("/storage/emulated/0/Music");
    }
    QDir dir(path);
    if (!dir.exists() && !dir.mkpath(QStringLiteral("."))) {
        qWarning() << "Could not create music directory" << path
                   << "- falling back to the app private directory";
        const QStringList appPaths =
                QStandardPaths::standardLocations(QStandardPaths::AppDataLocation);
        VERIFY_OR_DEBUG_ASSERT(!appPaths.isEmpty()) {
            return path;
        }
        path = appPaths.first() + QStringLiteral("/Music");
        QDir().mkpath(path);
    }
    return path;
}

} // namespace android
} // namespace mixxx
