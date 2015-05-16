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
#include "util/regex.h"

#include <QMutexLocker>
#include <QDir>
#include <QDesktopServices>
#include <QCoreApplication>
#include <QApplication>

//Static memory allocation
QMutex SoundSourceProxy::s_mutex;
QRegExp SoundSourceProxy::s_supportedFileRegex;
QMap<QString, Mixxx::SoundSourcePluginLibraryPointer> SoundSourceProxy::s_soundSourcePluginLibraries;
QMap<QString, Mixxx::SoundSourceProviderPointer> SoundSourceProxy::s_soundSourceProviders;
QSet<QString> SoundSourceProxy::s_supportedFileExtensionsByPlugins;

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

// static
void SoundSourceProxy::loadPlugins() {
    // Initialize built-in file types (last provider wins)
#ifdef __SNDFILE__
    // libsndfile is just a fallback and will be overwritten by
    // specialized providers!
    addSoundSourceProvider(Mixxx::SoundSourceProviderPointer(
            new Mixxx::SoundSourceProviderSndFile));
#endif
    addSoundSourceProvider(Mixxx::SoundSourceProviderPointer(
            new Mixxx::SoundSourceProviderFLAC));
    addSoundSourceProvider(Mixxx::SoundSourceProviderPointer(
            new Mixxx::SoundSourceProviderOggVorbis));
#ifdef __OPUS__
    addSoundSourceProvider(Mixxx::SoundSourceProviderPointer(
            new Mixxx::SoundSourceProviderOpus));
#endif
#ifdef __MAD__
    addSoundSourceProvider(Mixxx::SoundSourceProviderPointer(
            new Mixxx::SoundSourceProviderMp3));
#endif
#ifdef __MODPLUG__
    addSoundSourceProvider(Mixxx::SoundSourceProviderPointer(
            new Mixxx::SoundSourceProviderModPlug));
#endif
#ifdef __COREAUDIO__
    addSoundSourceProvider(Mixxx::SoundSourceProviderPointer(
            new Mixxx::SoundSourceProviderCoreAudio));
#endif
#ifdef __FFMPEGFILE__
    // FFmpeg currently overrides all other built-in providers
    // if enabled
    return Mixxx::SoundSourcePointer(new Mixxx::SoundSourceFFmpeg(url));
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

    foreach(QDir dir, pluginDirs){
    QStringList files = dir.entryList(nameFilters, QDir::Files | QDir::NoDotAndDotDot);
    foreach (const QString& file, files) {
        Mixxx::SoundSourcePluginLibraryPointer pPluginLibrary(
                Mixxx::SoundSourcePluginLibrary::load(dir.filePath(file)));
        if (pPluginLibrary) {
            Mixxx::SoundSourceProviderPointer pSoundSourceProvider(
                    pPluginLibrary->getSoundSourceProvider());
            const QStringList supportedFileTypes(pSoundSourceProvider->getSupportedFileTypes());
            if (supportedFileTypes.isEmpty()) {
                qWarning() << "SoundSource plugin does not support any file types"
                        << pPluginLibrary->getFileName();
            } else {
                s_soundSourcePluginLibraries.insert(
                        pPluginLibrary->getFileName(),
                        pPluginLibrary);
                s_supportedFileExtensionsByPlugins +=
                        QSet<QString>::fromList(supportedFileTypes);
                addSoundSourceProvider(
                        pSoundSourceProvider,
                        supportedFileTypes);
            }
        }
    }
}
}

//static
void SoundSourceProxy::addSoundSourceProvider(
        Mixxx::SoundSourceProviderPointer pSoundSourceProvider) {
    addSoundSourceProvider(pSoundSourceProvider,
            pSoundSourceProvider->getSupportedFileTypes());
}

//static
void SoundSourceProxy::addSoundSourceProvider(
        Mixxx::SoundSourceProviderPointer pSoundSourceProvider,
        const QStringList& supportedFileTypes) {
    DEBUG_ASSERT(!supportedFileTypes.isEmpty()); // wouldn't make any sense
    for (int i = 0; i < supportedFileTypes.size(); ++i) {
        s_soundSourceProviders.insert(
                supportedFileTypes[i],
                pSoundSourceProvider);
    }
}

// static
Mixxx::SoundSourcePointer SoundSourceProxy::initialize(
        const QString& qFilename) {
    const QUrl url(QUrl::fromLocalFile(qFilename));

    const QString type(Mixxx::SoundSource::getTypeFromUrl(url));
    if (type.isEmpty()) {
        qWarning() << "Unknown file type:" << qFilename;
        return Mixxx::SoundSourcePointer();
    }

    if (s_soundSourceProviders.contains(type)) {
        Mixxx::SoundSourceProviderPointer pSoundSourceProvider(
                s_soundSourceProviders.value(type));
        DEBUG_ASSERT(pSoundSourceProvider);
        return pSoundSourceProvider->newSoundSource(url);
    } else { //Unsupported filetype
        return Mixxx::SoundSourcePointer();
    }
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
QStringList SoundSourceProxy::supportedFileExtensions() {
    QMutexLocker locker(&s_mutex);
    return s_soundSourceProviders.keys();
}

// static
QStringList SoundSourceProxy::supportedFileExtensionsByPlugins() {
    QMutexLocker locker(&s_mutex);
    return s_supportedFileExtensionsByPlugins.toList();
}

// static
QString SoundSourceProxy::supportedFileExtensionsString() {
    QStringList supportedFileExtList =
            SoundSourceProxy::supportedFileExtensions();
    // Turn the list into a "*.mp3 *.wav *.etc" style string
    for (int i = 0; i < supportedFileExtList.size(); ++i) {
        supportedFileExtList[i] = QString("*.%1").arg(supportedFileExtList[i]);
    }
    return supportedFileExtList.join(" ");
}

// static
QString SoundSourceProxy::supportedFileExtensionsRegex() {
    QStringList supportedFileExtList =
            SoundSourceProxy::supportedFileExtensions();
    return RegexUtils::fileExtensionsRegex(supportedFileExtList);
}

// static
bool SoundSourceProxy::isFilenameSupported(QString fileName) {
    if (s_supportedFileRegex.isEmpty()) {
        QString regex = SoundSourceProxy::supportedFileExtensionsRegex();
        s_supportedFileRegex = QRegExp(regex, Qt::CaseInsensitive);
    }
    return fileName.contains(s_supportedFileRegex);
}
