#include "soundsourceproxy.h"

#include "trackinfoobject.h"
#ifdef __MAD__
#include "sources/soundsourcemp3.h"
#endif
#include "sources/soundsourceoggvorbis.h"
#ifdef __OPUS__
#include "sources/soundsourceopus.h"
#endif
#ifdef __COREAUDIO__
#include "sources/soundsourcecoreaudio.h"
#endif
#ifdef __SNDFILE__
#include "sources/soundsourcesndfile.h"
#endif
#ifdef __FFMPEGFILE__
#include "sources/soundsourceffmpeg.h"
#endif
#ifdef __MODPLUG__
#include "sources/soundsourcemodplug.h"
#endif
#include "sources/soundsourceflac.h"
#include "util/cmdlineargs.h"

#include <QMutexLocker>
#include <QDir>
#include <QDesktopServices>
#include <QCoreApplication>
#include <QApplication>

//Static memory allocation
QMutex SoundSourceProxy::s_mutex;
QRegExp SoundSourceProxy::s_supportedFileRegex;
Mixxx::SoundSourceProviderRegistry SoundSourceProxy::s_soundSourceProviders;

namespace {

SecurityTokenPointer openSecurityToken(QString qFilename,
        SecurityTokenPointer pToken) {
    if (pToken.isNull()) {
        // Open a security token for the file if we are in a sandbox.
        QFileInfo info(qFilename);
        return Sandbox::openSecurityToken(info, true);
    } else {
        return pToken;
    }
}

}

//Constructor
SoundSourceProxy::SoundSourceProxy(QString qFilename,
        SecurityTokenPointer pToken)
        : m_pSecurityToken(openSecurityToken(qFilename, pToken))
                , m_pSoundSource(initialize(qFilename)) {
}

//Other constructor
SoundSourceProxy::SoundSourceProxy(TrackPointer pTrack)
        : m_pTrack(pTrack)
                , m_pSecurityToken(
                openSecurityToken(pTrack->getLocation(),
                        pTrack->getSecurityToken()))
                        , m_pSoundSource(initialize(pTrack->getLocation())) {
}

Mixxx::AudioSourcePointer SoundSourceProxy::openAudioSource(const Mixxx::AudioSourceConfig& audioSrcCfg) {
    if (m_pAudioSource) {
        qDebug() << "AudioSource is already open";
        return m_pAudioSource;
    }

    if (!m_pSoundSource) {
        qDebug() << "No SoundSource available";
        return m_pAudioSource;
    }

    if (OK != m_pSoundSource->open(audioSrcCfg)) {
        qWarning() << "Failed to open SoundSource";
        return m_pAudioSource;
    }

    if (!m_pSoundSource->isValid()) {
        qWarning() << "Invalid file:" << m_pSoundSource->getUrlString()
                << "channels" << m_pSoundSource->getChannelCount()
                << "frame rate" << m_pSoundSource->getChannelCount();
        return m_pAudioSource;
    }
    if (m_pSoundSource->isEmpty()) {
        qWarning() << "Empty file:" << m_pSoundSource->getUrlString();
        return m_pAudioSource;
    }

    // Overwrite metadata with actual audio properties
    if (m_pTrack) {
        m_pTrack->setChannels(m_pSoundSource->getChannelCount());
        m_pTrack->setSampleRate(m_pSoundSource->getFrameRate());
        if (m_pSoundSource->hasDuration()) {
            m_pTrack->setDuration(m_pSoundSource->getDuration());
        }
        if (m_pSoundSource->hasBitrate()) {
            m_pTrack->setBitrate(m_pSoundSource->getBitrate());
        }
    }

    m_pAudioSource = m_pSoundSource;

    return m_pAudioSource;
}

void SoundSourceProxy::closeAudioSource() {
    if (m_pAudioSource) {
        DEBUG_ASSERT(m_pSoundSource);
        m_pSoundSource->close();
        m_pAudioSource.clear();
    }
}

// static
void SoundSourceProxy::loadPlugins() {
    QMutexLocker locker(&s_mutex);

    // Initialize built-in file types (last provider wins)
#ifdef __SNDFILE__
    // libsndfile is just a fallback and will be overwritten by
    // specialized providers!
    s_soundSourceProviders.registerProvider(Mixxx::SoundSourceProviderPointer(
            new Mixxx::SoundSourceProviderSndFile));
#endif
    s_soundSourceProviders.registerProvider(Mixxx::SoundSourceProviderPointer(
            new Mixxx::SoundSourceProviderFLAC));
    s_soundSourceProviders.registerProvider(Mixxx::SoundSourceProviderPointer(
            new Mixxx::SoundSourceProviderOggVorbis));
#ifdef __OPUS__
    s_soundSourceProviders.registerProvider(Mixxx::SoundSourceProviderPointer(
            new Mixxx::SoundSourceProviderOpus));
#endif
#ifdef __MAD__
    s_soundSourceProviders.registerProvider(Mixxx::SoundSourceProviderPointer(
            new Mixxx::SoundSourceProviderMp3));
#endif
#ifdef __MODPLUG__
    s_soundSourceProviders.registerProvider(Mixxx::SoundSourceProviderPointer(
            new Mixxx::SoundSourceProviderModPlug));
#endif
#ifdef __COREAUDIO__
    s_soundSourceProviders.registerProvider(Mixxx::SoundSourceProviderPointer(
            new Mixxx::SoundSourceProviderCoreAudio));
#endif
#ifdef __FFMPEGFILE__
    // FFmpeg currently overrides all other built-in providers
    // if enabled
    s_soundSourceProviders.registerProvider(Mixxx::SoundSourceProviderPointer(
            new Mixxx::SoundSourceProviderFFmpeg));
#endif

    // Scan for and initialize all plugins.
    // Loaded plugins will replace any built-in providers
    // that have been registered before (see above)!
    QList<QDir> pluginDirs;
    QStringList nameFilters;

    const QString& pluginPath = CmdlineArgs::Instance().getPluginPath();
    if (!pluginPath.isEmpty()) {
        qDebug() << "Adding plugin path from commandline arg:" << pluginPath;
        pluginDirs << QDir(pluginPath);
    }

    const QString dataLocation = QDesktopServices::storageLocation(
            QDesktopServices::DataLocation);
    const QString applicationPath = QCoreApplication::applicationDirPath();

#ifdef __LINUX__
    // TODO(rryan): Why can't we use applicationDirPath() and assume it's in the
    // 'bin' folder of $PREFIX, so we just traverse
    // ../lib/mixxx/plugins/soundsource.
    QDir libPluginDir(UNIX_LIB_PATH);
    if (libPluginDir.cd("plugins") && libPluginDir.cd("soundsource")) {
        pluginDirs << libPluginDir;
    }

    QDir dataPluginDir(dataLocation);
    if (dataPluginDir.cd("plugins") && dataPluginDir.cd("soundsource")) {
        pluginDirs << dataPluginDir;
    }

    // For people who build from source.
    QDir developer32Root(applicationPath);
    if (developer32Root.cd("lin32_build") && developer32Root.cd("plugins")) {
        pluginDirs << developer32Root.absolutePath();
    }
    QDir developer64Root(applicationPath);
    if (developer64Root.cd("lin64_build") && developer64Root.cd("plugins")) {
        pluginDirs << developer64Root.absolutePath();
    }
#elif __WINDOWS__
    QDir appPluginDir(applicationPath);
    if (appPluginDir.cd("plugins") && appPluginDir.cd("soundsource")) {
        pluginDirs << appPluginDir;
    }
#elif __APPLE__
    // blah/Mixxx.app/Contents/MacOS/../PlugIns/
    // TODO(XXX): Our SCons bundle target doesn't handle plugin subdirectories
    // :( so we can't do:
    //blah/Mixxx.app/Contents/PlugIns/soundsource
    QDir bundlePluginDir(applicationPath);
    if (bundlePluginDir.cdUp() && bundlePluginDir.cd("PlugIns")) {
        pluginDirs << bundlePluginDir;
    }

    // For people who build from source.
    QDir developer32Root(applicationPath);
    if (developer32Root.cd("osx32_build") && developer32Root.cd("plugins")) {
        pluginDirs << developer32Root.absolutePath();
    }
    QDir developer64Root(applicationPath);
    if (developer64Root.cd("osx64_build") && developer64Root.cd("plugins")) {
        pluginDirs << developer64Root.absolutePath();
    }

    QDir dataPluginDir(dataLocation);
    if (dataPluginDir.cd("Plugins") && dataPluginDir.cd("soundsource")) {
        pluginDirs << dataPluginDir;
    }

    nameFilters << "libsoundsource*";
#endif

    foreach(QDir dir, pluginDirs) {
        QStringList files = dir.entryList(nameFilters, QDir::Files | QDir::NoDotAndDotDot);
        foreach (const QString& file, files) {
            Mixxx::SoundSourcePluginLibraryPointer pPluginLibrary(
                    Mixxx::SoundSourcePluginLibrary::load(dir.filePath(file)));
            if (pPluginLibrary) {
                const Mixxx::SoundSourceProviderPointer pSoundSourceProvider(
                        pPluginLibrary->getSoundSourceProvider());
                s_soundSourceProviders.registerProviderPlugin(
                        pSoundSourceProvider, pPluginLibrary);
            }
        }

        const QStringList supportedFileTypes(
                s_soundSourceProviders.getSupportedFileTypes());
        foreach(const QString &supportedFileType, supportedFileTypes) {
            const Mixxx::SoundSourceProviderPointer pProvider(
                    s_soundSourceProviders.getProviderForFileType(supportedFileType));
            const Mixxx::SoundSourcePluginLibraryPointer pPluginLibrary(
                    s_soundSourceProviders.getPluginLibraryForFileType(supportedFileType));
            if (pPluginLibrary) {
                qDebug() << "SoundSourceProvider for" << supportedFileType
                        << "is" << pProvider->getName()
                        << "from" << pPluginLibrary->getFileName();
            } else {
                qDebug() << "SoundSourceProvider for" << supportedFileType
                        << "is" << pProvider->getName();
            }
        }
    }
}

// static
QStringList SoundSourceProxy::supportedFileExtensions() {
    QMutexLocker locker(&s_mutex);
    return s_soundSourceProviders.getSupportedFileTypes();
}

// static
QStringList SoundSourceProxy::supportedFileExtensionsByPlugins() {
    QMutexLocker locker(&s_mutex);
    return s_soundSourceProviders.getSupportedPluginFileTypes();
}

// static
QStringList SoundSourceProxy::supportedFileNamePatterns() {
    QMutexLocker locker(&s_mutex);
    return s_soundSourceProviders.getSupportedFileNamePatterns();
}

//static
QRegExp SoundSourceProxy::supportedFileNameRegex() {
    QMutexLocker locker(&s_mutex);
    return s_soundSourceProviders.getSupportedFileNameRegex();
}

// static
bool SoundSourceProxy::isFilenameSupported(const QString& fileName) {
    QMutexLocker locker(&s_mutex);
    return s_soundSourceProviders.isSuppportedFileName(fileName);
}

// static
Mixxx::SoundSourcePointer SoundSourceProxy::initialize(
        const QString& qFilename) {
    const QUrl url(QUrl::fromLocalFile(qFilename));

    const QString fileType(Mixxx::SoundSource::getTypeFromUrl(url));
    if (fileType.isEmpty()) {
        qWarning() << "Unknown file type:" << qFilename;
        return Mixxx::SoundSourcePointer();
    }

    QMutexLocker locker(&s_mutex);

    Mixxx::SoundSourceProviderPointer pSoundSourceProvider(
            s_soundSourceProviders.getProviderForFileType(fileType));
    if (pSoundSourceProvider) {
        return pSoundSourceProvider->newSoundSource(url);
    } else {
        qWarning() << "Unsupported file type" << qFilename;
        return Mixxx::SoundSourcePointer();
    }
}
