#include "analyzer/vamp/vamppluginloader.h"

#include <mutex>

#include <QCoreApplication>
#include <QDesktopServices>
#include <QDir>
#include <QStringBuilder>
#include <QStringList>

#include "analyzer/vamp/vampanalyzer.h"
#include "util/logger.h"

#ifdef __WINDOWS__
    #include <windows.h>
    #define ENV_PATH_LIST_SEPARATOR ";"
#else
    #define ENV_PATH_LIST_SEPARATOR ":"
#endif


namespace mixxx {

namespace {

const Logger kLogger("VampPluginLoader");

Vamp::HostExt::PluginLoader* s_pPluginLoader = nullptr;

std::mutex s_mutex;

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

} // anonymous namespace

VampPluginLoader::VampPluginLoader() {
    std::lock_guard<std::mutex> locked(s_mutex);
    if (!s_pPluginLoader) {
        initPluginPaths();
        s_pPluginLoader = Vamp::HostExt::PluginLoader::getInstance();
        VERIFY_OR_DEBUG_ASSERT(s_pPluginLoader) {
            kLogger.critical()
                    << "Failed to get instance of Vamp::HostExt::PluginLoader";
        }
    }
}

Vamp::HostExt::PluginLoader::PluginKeyList VampPluginLoader::listPlugins() const {
    std::lock_guard<std::mutex> locked(s_mutex);
    return s_pPluginLoader->listPlugins();
}

Vamp::Plugin* VampPluginLoader::loadPlugin(
    Vamp::HostExt::PluginLoader::PluginKey key,
    float inputSampleRate, int adapterFlags) const {
    std::lock_guard<std::mutex> locked(s_mutex);
    return s_pPluginLoader->loadPlugin(
        key, inputSampleRate, adapterFlags);
}

void VampPluginLoader::unloadPlugin(Vamp::Plugin** ppPlugin) const {
    DEBUG_ASSERT(ppPlugin);
    std::lock_guard<std::mutex> locked(s_mutex);
    delete *ppPlugin;
    *ppPlugin = nullptr;
}

bool VampPluginLoader::loadAnalyzerPlugin(
        VampAnalyzer* pAnalyzer,
        const QString& pluginLib,
        const QString& pluginId,
        SINT inputChannels,
        SINT inputSampleRate) const {
    QStringList pluginList = pluginId.split(":");
    if (pluginList.size() != 2) {
        qDebug() << "VampAnalyzer: got malformed pluginId: " << pluginId;
        return false;
    }

    bool isOutputNumber = false;
    int outputNumber = pluginList.at(1).toInt(&isOutputNumber);
    if (!isOutputNumber) {
        qDebug() << "VampAnalyzer: got malformed pluginId: " << pluginId;
        return false;
    }

    std::lock_guard<std::mutex> locked(s_mutex);

    Vamp::HostExt::PluginLoader::PluginKey pluginKey =
            s_pPluginLoader->composePluginKey(
                    pluginLib.toStdString(),
                    pluginList.at(0).toStdString());

    if (pAnalyzer->m_plugin) {
        qDebug() << "VampAnalyzer: kill plugin";
        delete pAnalyzer->m_plugin;
        pAnalyzer->m_plugin = nullptr;
    }
    pAnalyzer->m_plugin = s_pPluginLoader->loadPlugin(
            pluginKey,
            inputSampleRate,
            Vamp::HostExt::PluginLoader::ADAPT_ALL_SAFE);
    if (!pAnalyzer->m_plugin) {
        qDebug() << "VampAnalyzer: Cannot load Vamp Plug-in.";
        qDebug() << "Please copy libmixxxminimal.so from build dir to one of the following:";
        std::vector<std::string> path = Vamp::PluginHostAdapter::getPluginPath();
        for (unsigned int i = 0; i < path.size(); i++) {
            qDebug() << QString::fromStdString(path[i]);
        }
        return false;
    }

    Vamp::Plugin::OutputList outputs = pAnalyzer->m_plugin->getOutputDescriptors();
    if (outputs.empty()) {
        qDebug() << "VampAnalyzer: Plugin has no outputs!";
        return false;
    }
    if (outputNumber >= 0 && outputNumber < int(outputs.size())) {
        pAnalyzer->m_iOutput = outputNumber;
    } else {
        qDebug() << "VampAnalyzer: Invalid output number!";
        return false;
    }

    pAnalyzer->m_iBlockSize = pAnalyzer->m_plugin->getPreferredBlockSize();
    qDebug() << "Vampanalyzer BlockSize: " << pAnalyzer->m_iBlockSize;
    if (pAnalyzer->m_iBlockSize == 0) {
        // A plugin that can handle any block size may return 0. The final block
        // size will be set in the initialize() call. Since 0 means it is
        // accepting any size, 1024 should be good
        pAnalyzer->m_iBlockSize = 1024;
        qDebug() << "Vampanalyzer: setting pAnalyzer->m_iBlockSize to 1024";
    }

    pAnalyzer->m_iStepSize = pAnalyzer->m_plugin->getPreferredStepSize();
    qDebug() << "Vampanalyzer StepSize: " << pAnalyzer->m_iStepSize;
    if (pAnalyzer->m_iStepSize == 0 || pAnalyzer->m_iStepSize > pAnalyzer->m_iBlockSize) {
        // A plugin may return 0 if it has no particular interest in the step
        // size. In this case, the host should make the step size equal to the
        // block size if the plugin is accepting input in the time domain. If
        // the plugin is accepting input in the frequency domain, the host may
        // use any step size. The final step size will be set in the
        // initialize() call.
        pAnalyzer->m_iStepSize = pAnalyzer->m_iBlockSize;
        qDebug() << "Vampanalyzer: setting pAnalyzer->m_iStepSize to" << pAnalyzer->m_iStepSize;
    }

    if (!pAnalyzer->m_plugin->initialise(inputChannels, pAnalyzer->m_iStepSize, pAnalyzer->m_iBlockSize)) {
        qDebug() << "VampAnalyzer: Cannot initialize plugin";
        return false;
    }

    return true;
}

void VampPluginLoader::unloadAnalyzerPlugin(
        VampAnalyzer* pAnalyzer) const {
    DEBUG_ASSERT(pAnalyzer);
    unloadPlugin(&pAnalyzer->m_plugin);
}

Vamp::Plugin::FeatureSet VampPluginLoader::process(
        Vamp::Plugin* pPlugin,
        const float *const *inputBuffers,
        Vamp::RealTime timestamp) const {
    DEBUG_ASSERT(pPlugin);
    std::lock_guard<std::mutex> locked(s_mutex);
    return pPlugin->process(inputBuffers, timestamp);
}

Vamp::Plugin::FeatureSet VampPluginLoader::getRemainingFeatures(
        Vamp::Plugin* pPlugin) const {
    DEBUG_ASSERT(pPlugin);
    std::lock_guard<std::mutex> locked(s_mutex);
    return pPlugin->getRemainingFeatures();
}

} // namespace mixxx
