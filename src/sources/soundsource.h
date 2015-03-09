#ifndef MIXXX_SOUNDSOURCE_H
#define MIXXX_SOUNDSOURCE_H

#define MIXXX_SOUNDSOURCE_API_VERSION 7
/** @note SoundSource API Version history:
 1 - Mixxx 1.8.0 Beta 2
 2 - Mixxx 1.9.0 Pre (added key code)
 3 - Mixxx 1.10.0 Pre (added freeing function for extensions)
 4 - Mixxx 1.11.0 Pre (added composer field to SoundSource)
 5 - Mixxx 1.12.0 Pre (added album artist and grouping fields to SoundSource)
 6 - Mixxx 1.12.0 Pre (added cover art suppport)
 7 - Mixxx 1.13.0 New SoundSource/AudioSource API
 */

#include "sources/metadatasource.h"
#include "sources/audiosource.h"

namespace Mixxx {

// Base class for sound sources.
class SoundSource: public MetadataSource, public AudioSource {
public:
    static QString getTypeFromUrl(QUrl url);

    const QString& getType() const {
        return m_type;
    }

    Result parseTrackMetadata(Mixxx::TrackMetadata* pMetadata) const /*override*/;

    QImage parseCoverArt() const /*override*/;

    // Opens the AudioSource for reading audio data.
    //
    // Since reopening is not supported close() will be called
    // implicitly before the AudioSource is actually opened.
    Result open(SINT channelCountHint = kChannelCountDefault);

    // Closes the AudioSource and frees all resources.
    //
    // Might be called even if the AudioSource has never been
    // opened, has already been closed, or if opening has failed.
    virtual void close() = 0;

protected:
    explicit SoundSource(QUrl url);
    SoundSource(QUrl url, QString type);

private:
    // Tries to open the AudioSource for reading audio data
    // according to the "Template Method" design pattern. If
    // tryOpen() fails all (partially) allocated resources
    // will be freed by close(). Implementing classes do not
    // need to free resources in tryOpen() themselves, but
    // should instead be prepared for the following invocation
    // of close().
    virtual Result tryOpen(SINT channelCountHint) = 0;

    const QString m_type;
};

typedef QSharedPointer<SoundSource> SoundSourcePointer;

} //namespace Mixxx

#endif // MIXXX_SOUNDSOURCE_H
