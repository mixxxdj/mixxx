#ifndef SOUNDSOURCE_H
#define SOUNDSOURCE_H

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

#include "sources/audiosource.h"

#include <QImage>

/** Getter function to be declared by all SoundSource plugins */
namespace Mixxx {
class SoundSource;
class TrackMetadata;
}

typedef Mixxx::SoundSource* (*getSoundSourceFunc)(QString fileName);
typedef char** (*getSupportedFileExtensionsFunc)();
typedef int (*getSoundSourceAPIVersionFunc)();
/* New in version 3 */
typedef void (*freeFileExtensionsFunc)(char** fileExts);

namespace Mixxx {

/*
 Base class for sound sources.
 */
class SoundSource: public UrlResource {
public:
    static QString getTypeFromUrl(QUrl url);

    inline const QString& getType() const {
        return m_type;
    }

    // Parses metadata before opening the SoundSource for reading.
    //
    // Only metadata that is quickly readable should be read.
    // The implementation is free to set inaccurate estimated
    // values here that are overwritten when the AudioSource is
    // actually opened for reading.
    virtual Result parseMetadata(Mixxx::TrackMetadata* pMetadata) const;

    // Returns the first cover art image embedded within the file (if any).
    virtual QImage parseCoverArt() const;

    // Opens the SoundSource for reading audio data.
    virtual AudioSourcePointer open() const = 0;

protected:
    explicit SoundSource(QUrl url);
    SoundSource(QUrl url, QString type);

private:
    const QString m_type;
};

typedef QSharedPointer<SoundSource> SoundSourcePointer;

} //namespace Mixxx

#endif
