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
    #define PATH_SEPARATOR ";"
#else
    #define PATH_SEPARATOR ":"
#endif


namespace mixxx {

namespace {

Logger kLogger("VampPluginLoader");

Vamp::HostExt::PluginLoader* pPluginLoader = nullptr;

std::once_flag initPluginLoaderOnceFlag;

// Initialize the VAMP_PATH environment variable to point to the default
// places that Mixxx VAMP plugins are deployed on installation. If a
// VAMP_PATH environment variable is already set by the user, then this
// method appends to that.
void initPluginPaths() {
    const QLatin1String pathEnv(getenv("VAMP_PATH"));
    QStringList pathElements = QString(pathEnv).split(PATH_SEPARATOR, QString::SkipEmptyParts);

    const QString dataLocation = QDesktopServices::storageLocation(
            QDesktopServices::DataLocation);
    const QString applicationPath = QCoreApplication::applicationDirPath();

#ifdef __WINDOWS__
    QDir winPath(applicationPath);
    if (winPath.cd("plugins") && winPath.cd("vamp")) {
        pathElements << winPath.absolutePath().replace("/","\\");
    } else {
        kLogger.debug() << winPath.absolutePath() << "does not exist!";
    }
#elif __APPLE__
    // Location within the OS X bundle that we store plugins.
    // blah/Mixxx.app/Contents/MacOS/
    QDir bundlePluginDir(applicationPath);
    if (bundlePluginDir.cdUp() && bundlePluginDir.cd("PlugIns")) {
        pathElements << bundlePluginDir.absolutePath();
    }

    // For people who build from source.
    QDir developer32Root(applicationPath);
    if (developer32Root.cd("osx32_build") && developer32Root.cd("vamp-plugins")) {
        pathElements << developer32Root.absolutePath();
    }
    QDir developer64Root(applicationPath);
    if (developer64Root.cd("osx64_build") && developer64Root.cd("vamp-plugins")) {
        pathElements << developer64Root.absolutePath();
    }

    QDir dataPluginDir(dataLocation);
    if (dataPluginDir.cd("Plugins") && dataPluginDir.cd("vamp")) {
        pathElements << dataPluginDir.absolutePath();
    }
#elif __LINUX__
    QDir libPath(UNIX_LIB_PATH);
    if (libPath.cd("plugins") && libPath.cd("vamp")) {
        pathElements << libPath.absolutePath();
    }

    QDir dataPluginDir(dataLocation);
    if (dataPluginDir.cd("plugins") && dataPluginDir.cd("vamp")) {
        pathElements << dataPluginDir.absolutePath();
    }

    // For people who build from source.
    QDir developer32Root(applicationPath);
    if (developer32Root.cd("lin32_build") && developer32Root.cd("vamp-plugins")) {
        pathElements << developer32Root.absolutePath();
    }
    QDir developer64Root(applicationPath);
    if (developer64Root.cd("lin64_build") && developer64Root.cd("vamp-plugins")) {
        pathElements << developer64Root.absolutePath();
    }
#endif

    QString newPath = pathElements.join(PATH_SEPARATOR);
    kLogger.info() << "Setting VAMP_PATH to:" << newPath;
#ifdef __WINDOWS__
    QString winPathEnv = "VAMP_PATH=" % newPath;
    putenv(winPathEnv.toLocal8Bit().constData());
#else
    setenv("VAMP_PATH", newPath.toLocal8Bit().constData(), 1);
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
