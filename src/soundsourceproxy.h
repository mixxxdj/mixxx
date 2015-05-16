#ifndef SOUNDSOURCEPROXY_H
#define SOUNDSOURCEPROXY_H

#include "trackinfoobject.h"

#include "sources/soundsourcepluginlibrary.h"

#include "util/sandbox.h"

#include <QRegExp>
#include <QSet>

// Creates sound sources for filenames or tracks
class SoundSourceProxy: public Mixxx::MetadataSource {
public:
    static void loadPlugins();

    static QStringList supportedFileExtensions();
    static QStringList supportedFileExtensionsByPlugins();
    static QString supportedFileExtensionsString();
    static QString supportedFileExtensionsRegex();
    static bool isFilenameSupported(QString fileName);

    explicit SoundSourceProxy(QString qFilename, SecurityTokenPointer pToken = SecurityTokenPointer());
    explicit SoundSourceProxy(TrackPointer pTrack);

    QString getType() const {
        if (m_pSoundSource) {
            return m_pSoundSource->getType();
        } else {
            return QString();
        }
    }

    Result parseTrackMetadata(Mixxx::TrackMetadata* pMetadata) const /*override*/ {
        if (m_pSoundSource) {
            return m_pSoundSource->parseTrackMetadata(pMetadata);
        } else {
            return ERR;
        }
    }

    QImage parseCoverArt() const /*override*/ {
        if (m_pSoundSource) {
            return m_pSoundSource->parseCoverArt();
        } else {
            return QImage();
        }
    }

    // Opening the audio data through the proxy will
    // update the some metadata of the track object.
    // Returns a null pointer on failure.
    Mixxx::AudioSourcePointer openAudioSource(const Mixxx::AudioSourceConfig& audioSrcCfg = Mixxx::AudioSourceConfig());

    void closeAudioSource();

private:
    static QMutex s_mutex;
    static QRegExp s_supportedFileRegex;
    static QMap<QString, Mixxx::SoundSourcePluginLibraryPointer> s_soundSourcePluginLibraries;
    static QMap<QString, Mixxx::SoundSourceProviderPointer> s_soundSourceProviders;
    static QSet<QString> s_supportedFileExtensionsByPlugins;

    static void addSoundSourceProvider(
            Mixxx::SoundSourceProviderPointer pProvider);
    static void addSoundSourceProvider(
            Mixxx::SoundSourceProviderPointer pProvider,
            const QStringList& supportedFileTypes);

    static Mixxx::SoundSourcePointer initialize(const QString& qFilename);

    const TrackPointer m_pTrack;
    const SecurityTokenPointer m_pSecurityToken;

    const Mixxx::SoundSourcePointer m_pSoundSource;

    // Just an alias that keeps track of opening and closing
    // the corresponding SoundSource.
    Mixxx::AudioSourcePointer m_pAudioSource;
};

#endif
