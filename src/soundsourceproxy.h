#ifndef SOUNDSOURCEPROXY_H
#define SOUNDSOURCEPROXY_H

#include "trackinfoobject.h"

#include "sources/soundsourceproviderregistry.h"

// Creates sound sources for filenames or tracks
class SoundSourceProxy: public Mixxx::MetadataSource {
public:
    static void loadPlugins(); // not thread-safe

    static QStringList getSupportedFileExtensions();
    static QStringList getSupportedFileExtensionsByPlugins();
    static QStringList getSupportedFileNamePatterns();
    static QRegExp getSupportedFileNameRegex();

    static bool isUrlSupported(const QUrl& url);
    static bool isFileSupported(const QFileInfo& fileInfo);
    static bool isFileNameSupported(const QString& fileName);
    static bool isFileExtensionSupported(const QString& fileExtension);

    explicit SoundSourceProxy(
            const TrackPointer& pTrack);

    const QString& getFilePath() const {
        return m_filePath;
    }

    const QUrl& getUrl() const {
        return m_url;
    }

    QString getType() const {
        if (m_pSoundSource) {
            return m_pSoundSource->getType();
        } else {
            return QString();
        }
    }

    Result parseTrackMetadataAndCoverArt(
            Mixxx::TrackMetadata* pTrackMetadata,
            QImage* pCoverArt) const override {
        if (m_pSoundSource) {
            return m_pSoundSource->parseTrackMetadataAndCoverArt(
                    pTrackMetadata, pCoverArt);
        } else {
            return ERR;
        }
    }

    // Only for  backward compatibility.
    // Should be removed when no longer needed.
    Result parseTrackMetadata(Mixxx::TrackMetadata* pTrackMetadata) {
        return parseTrackMetadataAndCoverArt(pTrackMetadata, NULL);
    }

    // Only for  backward compatibility.
    // Should be removed when no longer needed.
    QImage parseCoverArt() const {
        QImage coverArt;
        const Result result = parseTrackMetadataAndCoverArt(NULL, &coverArt);
        return (result == OK) ? coverArt : QImage();
    }

    // Opening the audio data through the proxy will
    // update the some metadata of the track object.
    // Returns a null pointer on failure.
    Mixxx::AudioSourcePointer openAudioSource(const Mixxx::AudioSourceConfig& audioSrcCfg = Mixxx::AudioSourceConfig());

    void closeAudioSource();

private:
    static Mixxx::SoundSourceProviderRegistry s_soundSourceProviders;

    static Mixxx::SoundSourcePointer initialize(const QString& qFilename);

    const TrackPointer m_pTrack;

    const QString m_filePath;
    const QUrl m_url;

    const Mixxx::SoundSourcePointer m_pSoundSource;

    // Just an alias that keeps track of opening and closing
    // the corresponding SoundSource.
    Mixxx::AudioSourcePointer m_pAudioSource;
};

#endif // SOUNDSOURCEPROXY_H
