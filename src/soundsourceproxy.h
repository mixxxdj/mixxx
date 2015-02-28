#ifndef SOUNDSOURCEPROXY_H
#define SOUNDSOURCEPROXY_H

#include <QMap>
#include <QMutex>
#include <QString>
#include <QLibrary>
#include <QRegExp>

#include "trackinfoobject.h"
#include "sources/soundsourceplugin.h"
#include "util/sandbox.h"

/**
  *@author Tue Haste Andersen
  */


/**
 * Creates sound sources for filenames or tracks.
 */
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
    Mixxx::AudioSourcePointer openAudioSource();

    void closeAudioSource();

private:
    static QRegExp m_supportedFileRegex;
    static QMap<QString, QLibrary*> m_plugins;
    static QMap<QString, getSoundSourceFunc> m_extensionsSupportedByPlugins;
    static QMutex m_extensionsMutex;

    static QLibrary* getPlugin(QString lib_filename);

    static Mixxx::SoundSourcePointer initialize(const QString& qFilename);

    const TrackPointer m_pTrack;
    const SecurityTokenPointer m_pSecurityToken;

    const Mixxx::SoundSourcePointer m_pSoundSource;

    // Just an alias that keeps track of opening and closing
    // the corresponding SoundSource.
    Mixxx::AudioSourcePointer m_pAudioSource;
};

#endif
