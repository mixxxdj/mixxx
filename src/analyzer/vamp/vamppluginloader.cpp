/*
 * vamppluginloader.cpp
 *
 *  Created on: 23/jan/2012
 *      Author: Tobias Rafreider
 *
 * This is a thread-safe wrapper class around Vamp's
 * PluginLoader class.
 */
#include "analyzer/vamp/vamppluginloader.h"

#include <QCoreApplication>
#include <QDesktopServices>
#include <QDir>
#include <QMutex>
#include <QStringList>

#include <QtDebug>

#ifdef __WINDOWS__
    #include <windows.h>
    #define PATH_SEPARATOR ";"
#else
    #define PATH_SEPARATOR ":"
#endif

using Vamp::Plugin;
using Vamp::PluginHostAdapter;

namespace {

QMutex singletonMutex;

VampPluginLoader* pSingletonInstance = nullptr;

// Initialize the VAMP_PATH environment variable to point to the default
// places that Mixxx VAMP plugins are deployed on installation. If a
// VAMP_PATH environment variable is already set by the user, then this
// method appends to that.
void initializePluginPaths() {
    const char* pVampPath = getenv("VAMP_PATH");
    QString vampPath = "";
    if (pVampPath) {
        vampPath = QString(pVampPath);
    }

    // TODO(XXX) use correct split separator here.
    QStringList pathElements = vampPath.length() > 0 ? vampPath.split(PATH_SEPARATOR)
            : QStringList();

    const QString dataLocation = QDesktopServices::storageLocation(
            QDesktopServices::DataLocation);
    const QString applicationPath = QCoreApplication::applicationDirPath();

#ifdef __WINDOWS__
    QDir winVampPath(applicationPath);
    if (winVampPath.cd("plugins") && winVampPath.cd("vamp")) {
        pathElements << winVampPath.absolutePath().replace("/","\\");
    } else {
        qDebug() << winVampPath.absolutePath() << "does not exist!";
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
    qDebug() << "Setting VAMP_PATH to: " << newPath;
    QByteArray newPathBA = newPath.toLocal8Bit();
#ifndef __WINDOWS__
    setenv("VAMP_PATH", newPathBA.constData(), 1);
#else
    QString winpath = "VAMP_PATH=" + newPath;
    QByteArray winpathBA = winpath.toLocal8Bit();
    putenv(winpathBA.constData());
#endif
}

} // anonymous namespace

VampPluginLoader* VampPluginLoader::getInstance() {
    QMutexLocker lock(&singletonMutex);
    if (pSingletonInstance == nullptr) {
        initializePluginPaths();
        pSingletonInstance = new VampPluginLoader();
    }
    return pSingletonInstance;
}

VampPluginLoader::VampPluginLoader() {
    m_pVampPluginLoader = Vamp::HostExt::PluginLoader::getInstance();
}

PluginLoader::PluginKey VampPluginLoader::composePluginKey(
    std::string libraryName, std::string identifier) {
    QMutexLocker lock(&singletonMutex);
    PluginLoader::PluginKey key = m_pVampPluginLoader->composePluginKey(
        libraryName, identifier);
    return key;
}

PluginLoader::PluginCategoryHierarchy VampPluginLoader::getPluginCategory(
    Vamp::HostExt::PluginLoader::PluginKey plugin) {
    QMutexLocker lock(&singletonMutex);
    return m_pVampPluginLoader->getPluginCategory(plugin);
}

PluginLoader::PluginKeyList VampPluginLoader::listPlugins() {
    QMutexLocker lock(&singletonMutex);
    return m_pVampPluginLoader->listPlugins();
}

Vamp::Plugin* VampPluginLoader::loadPlugin(
    Vamp::HostExt::PluginLoader::PluginKey key,
    float inputSampleRate, int adapterFlags) {
    QMutexLocker lock(&singletonMutex);
    return m_pVampPluginLoader->loadPlugin(
        key, inputSampleRate, adapterFlags);
}
