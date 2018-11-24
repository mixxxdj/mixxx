#include "analyzer/vamp/vamppluginadapter.h"

#include <mutex>

#include <QCoreApplication>
#include <QStandardPaths>
#include <QDir>
#include <QStringBuilder>
#include <QStringList>

#include "util/logger.h"

#ifdef __WINDOWS__
    #include <windows.h>
    #define ENV_PATH_LIST_SEPARATOR ";"
#else
    #define ENV_PATH_LIST_SEPARATOR ":"
#endif


namespace mixxx {

namespace {

const Logger kLogger("VampPluginAdapter");

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

    const QString dataLocation = QStandardPaths::writableLocation(
            QStandardPaths::DataLocation);
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
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    QString vampDir = "vampqt5";
#else
    QString vampDir = "vamp";
#endif
    QDir libPath(UNIX_LIB_PATH);
    if (libPath.cd("plugins") && libPath.cd(vampDir)) {
        envPathList = composeEnvPathList(envPathList, libPath);
    } else {
        logIgnoringNonExistentPath(libPath);
    }

    QDir dataPluginDir(dataLocation);
    if (dataPluginDir.cd("plugins") && dataPluginDir.cd(vampDir)) {
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

std::mutex s_mutex;

Vamp::HostExt::PluginLoader* s_pluginLoader = nullptr;

Vamp::HostExt::PluginLoader* getPluginLoaderLocked() {
    if (!s_pluginLoader) {
        initPluginPaths();
        s_pluginLoader = Vamp::HostExt::PluginLoader::getInstance();
        VERIFY_OR_DEBUG_ASSERT(s_pluginLoader) {
            kLogger.critical()
                    << "Failed to get Vamp::HostExt::PluginLoader instance";
        }
    }
    return s_pluginLoader;
}

Vamp::Plugin* loadPluginLocked(
        Vamp::HostExt::PluginLoader::PluginKey key,
        float inputSampleRate,
        int adapterFlags) {
    const auto pluginLoader = getPluginLoaderLocked();
    if (pluginLoader) {
        const auto plugin = pluginLoader->loadPlugin(
            key, inputSampleRate, adapterFlags);
        if (plugin) {
            return plugin;
        }
    }
    kLogger.warning()
            << "Failed to load plugin"
            << key.c_str()
            << inputSampleRate
            << adapterFlags;
    return nullptr;
}

} // anonymous namespace

Vamp::HostExt::PluginLoader::PluginKeyList VampPluginAdapter::listPlugins() {
    const std::lock_guard<std::mutex> locked(s_mutex);
    const auto pluginLoader = getPluginLoaderLocked();
    if (pluginLoader) {
        return pluginLoader->listPlugins();
    } else {
        return Vamp::HostExt::PluginLoader::PluginKeyList();
    }
}

Vamp::HostExt::PluginLoader::PluginKey VampPluginAdapter::composePluginKey(
        std::string libraryName,
        std::string identifier) {
    const std::lock_guard<std::mutex> locked(s_mutex);
    const auto pluginLoader = getPluginLoaderLocked();
    if (pluginLoader) {
        return pluginLoader->composePluginKey(
                std::move(libraryName),
                std::move(identifier));
    } else {
        return Vamp::HostExt::PluginLoader::PluginKey();
    }
}

VampPluginAdapter::VampPluginAdapter()
        : m_plugin(nullptr),
          m_preferredBlockSize(0),
          m_preferredStepSize(0) {
}

VampPluginAdapter::VampPluginAdapter(
        Vamp::HostExt::PluginLoader::PluginKey key,
        float inputSampleRate,
        int adapterFlags)
        : m_plugin(nullptr),
          m_preferredBlockSize(0),
          m_preferredStepSize(0) {
    loadPlugin(key, inputSampleRate, adapterFlags);
}

VampPluginAdapter::~VampPluginAdapter() {
    std::lock_guard<std::mutex> locked(s_mutex);
    m_plugin.reset();
}

void VampPluginAdapter::loadPlugin(
        Vamp::HostExt::PluginLoader::PluginKey key,
        float inputSampleRate,
        int adapterFlags) {
    std::lock_guard<std::mutex> locked(s_mutex);
    m_plugin.reset();
    m_plugin.reset(loadPluginLocked(key, inputSampleRate, adapterFlags));
    if (m_plugin) {
        m_identifier = m_plugin->getIdentifier();
        m_name = m_plugin->getName();
        m_outputDescriptors = m_plugin->getOutputDescriptors();
        m_preferredBlockSize = m_plugin->getPreferredBlockSize();
        m_preferredStepSize = m_plugin->getPreferredStepSize();
    }
}

bool VampPluginAdapter::initialise(
            size_t inputChannels,
            size_t stepSize,
            size_t blockSize) {
    DEBUG_ASSERT(m_plugin);
    std::lock_guard<std::mutex> locked(s_mutex);
    return m_plugin->initialise(
            inputChannels,
            stepSize,
            blockSize);
}

Vamp::Plugin::FeatureSet VampPluginAdapter::process(
        const float* const* inputBuffers,
        Vamp::RealTime timestamp) {
    DEBUG_ASSERT(m_plugin);
    std::lock_guard<std::mutex> locked(s_mutex);
    return m_plugin->process(
            inputBuffers,
            timestamp);
}

Vamp::Plugin::FeatureSet VampPluginAdapter::getRemainingFeatures() {
    DEBUG_ASSERT(m_plugin);
    std::lock_guard<std::mutex> locked(s_mutex);
    return m_plugin->getRemainingFeatures();
}

} // namespace mixxx
