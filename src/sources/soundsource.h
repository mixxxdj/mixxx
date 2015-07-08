#ifndef MIXXX_SOUNDSOURCE_H
#define MIXXX_SOUNDSOURCE_H

#include "sources/metadatasource.h"
#include "sources/audiosource.h"

namespace Mixxx {

// Base class for sound sources.
class SoundSource: public MetadataSource, public AudioSource {
public:
    static QString getTypeFromUrl(const QUrl& url);

    const QString& getType() const {
        return m_type;
    }

    Result parseTrackMetadata(Mixxx::TrackMetadata* pMetadata) const /*override*/;

    QImage parseCoverArt() const /*override*/;

    // Opens the AudioSource for reading audio data.
    //
    // Since reopening is not supported close() will be called
    // implicitly before the AudioSource is actually opened.
    Result open(const AudioSourceConfig& audioSrcCfg = AudioSourceConfig());

    // Closes the AudioSource and frees all resources.
    //
    // Might be called even if the AudioSource has never been
    // opened, has already been closed, or if opening has failed.
    virtual void close() = 0;

protected:
    explicit SoundSource(const QUrl& url);
    SoundSource(const QUrl& url, const QString& type);

private:
    // Tries to open the AudioSource for reading audio data
    // according to the "Template Method" design pattern. If
    // tryOpen() fails all (partially) allocated resources
    // will be freed by close(). Implementing classes do not
    // need to free resources in tryOpen() themselves, but
    // should instead be prepared for the following invocation
    // of close().
    virtual Result tryOpen(const AudioSourceConfig& audioSrcCfg) = 0;

    const QString m_type;
};

typedef QSharedPointer<SoundSource> SoundSourcePointer;

} //namespace Mixxx

#endif // MIXXX_SOUNDSOURCE_H
