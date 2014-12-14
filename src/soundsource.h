/***************************************************************************
 soundsource.h  -  description
 -------------------
 begin                : Wed Feb 20 2002
 copyright            : (C) 2002 by Tue and Ken Haste Andersen
 email                :
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

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
 7 - Mixxx 1.13.0 New AudioSource API
 */

#include "audiosource.h"

#include "util/defs.h"

#include <QImage>

/** Getter function to be declared by all SoundSource plugins */
namespace Mixxx {
class SoundSource;
class TrackMetadata;
}

typedef Mixxx::SoundSource* (*getSoundSourceFunc)(QString filename);
typedef char** (*getSupportedFileExtensionsFunc)();
typedef int (*getSoundSourceAPIVersionFunc)();
/* New in version 3 */
typedef void (*freeFileExtensionsFunc)(char** exts);

namespace Mixxx {

/*
 Base class for sound sources.
 */
class SoundSource: public AudioSource {
public:
    virtual ~SoundSource();

    inline const QString& getFilename() const {
        return m_sFilename;
    }
    inline const QString& getType() const {
        return m_sType;
    }

    // Parses metadata before opening the SoundSource for reading.
    //
    // Only metadata that is quickly readable should be read.
    // The implementation is free to set inaccurate estimated
    // values here that are overwritten when the AudioSource is
    // actually opened for reading.
    virtual Result parseMetadata(Mixxx::TrackMetadata* pMetadata) = 0;

    // Returns the first cover art image embedded within the file (if any).
    virtual QImage parseCoverArt() = 0;

    /**
     * Opens the SoundSource for reading audio data.
     *
     * When opening a SoundSource the corresponding
     * AudioSource must be initialized.
     */
    virtual Result open() = 0;

    // The bitrate in kbit/s (optional).
    // The actual bitrate might be determined and set when the file is opened.
    inline bool hasBitrate() const {
        return 0 < m_bitrate;
    }
    inline size_type getBitrate() const {
        return m_bitrate;
    }

    // The actual duration in seconds.
    // The actual duration can be calculated after the file has been opened.
    inline bool hasDuration() const {
        return !isFrameCountEmpty() && isFrameRateValid();
    }
    inline size_type getDuration() const {
        return getFrameCount() / getFrameRate();
    }

protected:
    explicit SoundSource(QString sFilename);
    SoundSource(QString sFilename, QString sType);

    inline void setBitrate(size_type bitrate) {
        m_bitrate = bitrate;
    }

private:
    const QString m_sFilename;
    const QString m_sType;

    size_type m_bitrate;
};

typedef QSharedPointer<SoundSource> SoundSourcePointer;

} //namespace Mixxx

#endif
