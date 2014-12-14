/***************************************************************************
 soundsourceoggvorbis.h  -  ogg vorbis decoder
 -------------------
 copyright            : (C) 2003 by Svein Magne Bang
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

#ifndef SOUNDSOURCEOGGVORBIS_H
#define SOUNDSOURCEOGGVORBIS_H

#include "soundsource.h"

#define OV_EXCLUDE_STATIC_CALLBACKS
#include <vorbis/vorbisfile.h>

class SoundSourceOggVorbis: public Mixxx::SoundSource {
    typedef SoundSource Super;

public:
    static QList<QString> supportedFileExtensions();

    explicit SoundSourceOggVorbis(QString qFilename);
    ~SoundSourceOggVorbis();

    Result parseMetadata(Mixxx::TrackMetadata* pMetadata) /*override*/;
    QImage parseCoverArt() /*override*/;

    Result open() /*override*/;

    diff_type seekFrame(diff_type frameIndex) /*override*/;
    size_type readFrameSamplesInterleaved(size_type frameCount,
            sample_type* sampleBuffer) /*override*/;
    size_type readStereoFrameSamplesInterleaved(size_type frameCount,
            sample_type* sampleBuffer) /*override*/;

private:
    void close();

    size_type readFrameSamplesInterleaved(size_type frameCount,
            sample_type* sampleBuffer, bool readStereoSamples);

    OggVorbis_File m_vf;
};

#endif
