#include "analyzer/vamp/vamppluginloader.h"

#include <mutex>

#include <QCoreApplication>
#include <QDesktopServices>
#include <QDir>
#include <QStringList>
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

Vamp::HostExt::PluginLoader* pPluginLoader = nullptr;

std::once_flag initPluginLoaderOnceFlag;

QString composeEnvPathList(
        const QString& envPathList,
        const QDir& appendDir) {
    QString nativePath = QDir::toNativeSeparators(appendDir.absolutePath());
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

    const QString dataLocation = QDesktopServices::storageLocation(
            QDesktopServices::DataLocation);
    const QString applicationPath = QCoreApplication::applicationDirPath();

#ifdef __WINDOWS__
    QDir appDir(applicationPath);
    if (appDir.cd("plugins") && appDir.cd("vamp")) {
        envPathList = composeEnvPathList(envPathList, appDir);
    } else {
        kLogger.debug() << "Ignoring non-existent path:" << appDir;
    }
#elif __APPLE__
    // Location within the OS X bundle that we store plugins.
    // blah/Mixxx.app/Contents/MacOS/
    QDir bundlePluginDir(applicationPath);
    if (bundlePluginDir.cdUp() && bundlePluginDir.cd("PlugIns")) {
        envPathList = composeEnvPathList(envPathList, bundlePluginDir);
    } else {
        kLogger.debug() << "Ignoring non-existent path:" << bundlePluginDir;
    }

    // For people who build from source.
    QDir developer32Root(applicationPath);
    if (developer32Root.cd("osx32_build") && developer32Root.cd("vamp-plugins")) {
        envPathList = composeEnvPathList(envPathList, developer32Root);
    } else {
        kLogger.debug() << "Ignoring non-existent path:" << developer32Root;
    }
    QDir developer64Root(applicationPath);
    if (developer64Root.cd("osx64_build") && developer64Root.cd("vamp-plugins")) {
        envPathList = composeEnvPathList(envPathList, developer64Root);
    } else {
        kLogger.debug() << "Ignoring non-existent path:" << developer64Root;
    }

    QDir dataPluginDir(dataLocation);
    if (dataPluginDir.cd("Plugins") && dataPluginDir.cd("vamp")) {
        envPathList = composeEnvPathList(envPathList, dataPluginDir);
    } else {
        kLogger.debug() << "Ignoring non-existent path:" << dataPluginDir;
    }
#elif __LINUX__
    QDir libPath(UNIX_LIB_PATH);
    if (libPath.cd("plugins") && libPath.cd("vamp")) {
        envPathList = composeEnvPathList(envPathList, libPath);
    } else {
        kLogger.debug() << "Ignoring non-existent path:" << libPath;
    }

    QDir dataPluginDir(dataLocation);
    if (dataPluginDir.cd("plugins") && dataPluginDir.cd("vamp")) {
        envPathList = composeEnvPathList(envPathList, dataPluginDir);
    } else {
        kLogger.debug() << "Ignoring non-existent path:" << dataPluginDir;
    }

    // For people who build from source.
    QDir developer32Root(applicationPath);
    if (developer32Root.cd("lin32_build") && developer32Root.cd("vamp-plugins")) {
        envPathList = composeEnvPathList(envPathList, developer32Root);
    } else {
        kLogger.debug() << "Ignoring non-existent path:" << developer32Root;
    }
    QDir developer64Root(applicationPath);
    if (developer64Root.cd("lin64_build") && developer64Root.cd("vamp-plugins")) {
        envPathList = composeEnvPathList(envPathList, developer64Root);
    } else {
        kLogger.debug() << "Ignoring non-existent path:" << developer64Root;
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
    pPluginLoader = Vamp::HostExt::PluginLoader::getInstance();
}

} // anonymous namespace

VampPluginLoader::VampPluginLoader() {
    std::call_once(initPluginLoaderOnceFlag, initPluginLoader);
}

Vamp::HostExt::PluginLoader::PluginKey VampPluginLoader::composePluginKey(
    std::string libraryName, std::string identifier) {
    return pPluginLoader->composePluginKey(
        libraryName, identifier);
}

Vamp::HostExt::PluginLoader::PluginCategoryHierarchy VampPluginLoader::getPluginCategory(
    Vamp::HostExt::PluginLoader::PluginKey plugin) {
    return pPluginLoader->getPluginCategory(plugin);
}

Vamp::HostExt::PluginLoader::PluginKeyList VampPluginLoader::listPlugins() {
    return pPluginLoader->listPlugins();
}

Vamp::Plugin* VampPluginLoader::loadPlugin(
    Vamp::HostExt::PluginLoader::PluginKey key,
    float inputSampleRate, int adapterFlags) {
    return pPluginLoader->loadPlugin(
        key, inputSampleRate, adapterFlags);
}

} // namespace mixxx
