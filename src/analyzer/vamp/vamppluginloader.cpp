#include "analyzer/vamp/vamppluginloader.h"

#include <mutex>

#include <QCoreApplication>
#include <QDesktopServices>
#include <QDir>
#include <QStringBuilder>

#include "util/logger.h"

#ifdef __WINDOWS__
    #include <windows.h>
    #define ENV_PATH_LIST_SEPARATOR ";"
#else
    #define ENV_PATH_LIST_SEPARATOR ":"
#endif


namespace mixxx {

namespace {

Logger kLogger("VampPluginLoader");

Vamp::HostExt::PluginLoader* s_pPluginLoader = nullptr;

std::once_flag s_initPluginLoaderOnceFlag;

inline
QString toNativeEnvPath(const QDir& dir) {
    return QDir::toNativeSeparators(dir.absolutePath());
}

inline
void logIgnoringNonExistentPath(const QDir& dir) {
    kLogger.debug() << "Ignoring non-existent path:" << toNativeEnvPath(dir);
}

QString composeEnvPathList(
        const QString& envPathList,
        const QDir& appendDir) {
    QString nativePath = toNativeEnvPath(appendDir);
    if (envPathList.isEmpty()) {
        return nativePath;
    } else {
        return envPathList % ENV_PATH_LIST_SEPARATOR % nativePath;
    }
}

// Initialize the VAMP_PATH environment variable to point to the default
// places that Mixxx VAMP plugins are deployed on installation. If a
// VAMP_PATH environment variable is already set by the user, then this
// method appends to that.
void initPluginPaths() {
    QString envPathList =
            QString::fromLocal8Bit(qgetenv("VAMP_PATH").constData());
    kLogger.info() << "Current VAMP_PATH is:" << envPathList;

    const QString dataLocation = QDesktopServices::storageLocation(
            QDesktopServices::DataLocation);
    const QString applicationPath = QCoreApplication::applicationDirPath();

#ifdef __WINDOWS__
    QDir appDir(applicationPath);
    if (appDir.cd("plugins") && appDir.cd("vamp")) {
        envPathList = composeEnvPathList(envPathList, appDir);
    } else {
        logIgnoringNonExistentPath(appDir);
    }
#elif __APPLE__
    // Location within the OS X bundle that we store plugins.
    // blah/Mixxx.app/Contents/MacOS/
    QDir bundlePluginDir(applicationPath);
    if (bundlePluginDir.cdUp() && bundlePluginDir.cd("PlugIns")) {
        envPathList = composeEnvPathList(envPathList, bundlePluginDir);
    } else {
        logIgnoringNonExistentPath(bundlePluginDir);
    }

    // For people who build from source.
    QDir developer32Root(applicationPath);
    if (developer32Root.cd("osx32_build") && developer32Root.cd("vamp-plugins")) {
        envPathList = composeEnvPathList(envPathList, developer32Root);
    } else {
        logIgnoringNonExistentPath(developer32Root);
    }
    QDir developer64Root(applicationPath);
    if (developer64Root.cd("osx64_build") && developer64Root.cd("vamp-plugins")) {
        envPathList = composeEnvPathList(envPathList, developer64Root);
    } else {
        logIgnoringNonExistentPath(developer64Root);
    }

    QDir dataPluginDir(dataLocation);
    if (dataPluginDir.cd("Plugins") && dataPluginDir.cd("vamp")) {
        envPathList = composeEnvPathList(envPathList, dataPluginDir);
    } else {
        logIgnoringNonExistentPath(dataPluginDir);
    }
#elif __LINUX__
    QDir libPath(UNIX_LIB_PATH);
    if (libPath.cd("plugins") && libPath.cd("vamp")) {
        envPathList = composeEnvPathList(envPathList, libPath);
    } else {
        logIgnoringNonExistentPath(libPath);
    }

    QDir dataPluginDir(dataLocation);
    if (dataPluginDir.cd("plugins") && dataPluginDir.cd("vamp")) {
        envPathList = composeEnvPathList(envPathList, dataPluginDir);
    } else {
        logIgnoringNonExistentPath(dataPluginDir);
    }

    // For people who build from source.
    QDir developer32Root(applicationPath);
    if (developer32Root.cd("lin32_build") && developer32Root.cd("vamp-plugins")) {
        envPathList = composeEnvPathList(envPathList, developer32Root);
    } else {
        logIgnoringNonExistentPath(developer32Root);
    }
    QDir developer64Root(applicationPath);
    if (developer64Root.cd("lin64_build") && developer64Root.cd("vamp-plugins")) {
        envPathList = composeEnvPathList(envPathList, developer64Root);
    } else {
        logIgnoringNonExistentPath(developer64Root);
    }
#endif

    kLogger.info() << "Setting VAMP_PATH to:" << envPathList;
#ifdef __WINDOWS__
    envPathList = "VAMP_PATH=" % envPathList;
    putenv(envPathList.toLocal8Bit().constData());
#else
    setenv("VAMP_PATH", envPathList.toLocal8Bit().constData(), 1);
#endif
}

void initPluginLoader() {
    initPluginPaths();
    s_pPluginLoader = Vamp::HostExt::PluginLoader::getInstance();
}

} // anonymous namespace

VampPluginLoader::VampPluginLoader() {
    std::call_once(s_initPluginLoaderOnceFlag, initPluginLoader);
}

Vamp::HostExt::PluginLoader::PluginKey VampPluginLoader::composePluginKey(
    std::string libraryName, std::string identifier) {
    return s_pPluginLoader->composePluginKey(
        libraryName, identifier);
}

Vamp::HostExt::PluginLoader::PluginCategoryHierarchy VampPluginLoader::getPluginCategory(
    Vamp::HostExt::PluginLoader::PluginKey plugin) {
    return s_pPluginLoader->getPluginCategory(plugin);
}

Vamp::HostExt::PluginLoader::PluginKeyList VampPluginLoader::listPlugins() {
    return s_pPluginLoader->listPlugins();
}

Vamp::Plugin* VampPluginLoader::loadPlugin(
    Vamp::HostExt::PluginLoader::PluginKey key,
    float inputSampleRate, int adapterFlags) {
    return s_pPluginLoader->loadPlugin(
        key, inputSampleRate, adapterFlags);
}

} // namespace mixxx
