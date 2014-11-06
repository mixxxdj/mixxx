/***************************************************************************
                          soundsource.cpp  -  description
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

#include <QtDebug>

#include "soundsource.h"
#include "util/math.h"


namespace Mixxx
{

namespace
{
    const bool s_bDebugMetadata = false;

    const float BPM_ZERO = 0.0f;
    const float BPM_MIN = 60.0f;
    const float BPM_MAX = 300.0f;

    inline
    float parseBpmString(const QString& sBpm) {
        float bpm = sBpm.toFloat();
        if (bpm < BPM_MIN) {
            bpm = BPM_ZERO;
        } else {
            while(bpm > BPM_MAX) {
                bpm /= 10.0f;
            }
        }
        return bpm;
    }

    float parseReplayGainString (QString sReplayGain) {
        QString ReplayGainstring = sReplayGain.remove( " dB" );
        float fReplayGain = db2ratio(ReplayGainstring.toFloat());
        // I found some mp3s of mine with replaygain tag set to 0dB even if not normalized.
        // This is because of Rapid Evolution 3, I suppose. I prefer to rescan them by setting value to 0 (i.e. rescan via analyserrg)
        if (fReplayGain == 1.0f) {
            fReplayGain = 0.0f;
        }
        return fReplayGain;
    }

}

/*
   SoundSource is an Uber-class for the reading and decoding of audio-files.
   Each class must have the following member functions:
   initializer with a filename
   seek()
   read()
   length()
   In addition there must be a static member:
   int ParseHeader(TrackInfoObject *Track)
   which is used for parsing header information, like trackname,length etc. The
   return type is int: 0 for OK, -1 for an error.
 */
SoundSource::SoundSource(QString qFilename)
        : m_qFilename(qFilename),
          m_fReplayGain(0.0f),
          m_fBpm(0.0f),
          m_iChannels(0),
          m_iBitrate(0),
          m_iSampleRate(0),
          m_iDuration(0) {
}

SoundSource::~SoundSource() {
}

void SoundSource::setBpmString(const QString& sBpm) {
    if (!sBpm.isEmpty()) {
        float fBpm = parseBpmString(sBpm);
        if (BPM_ZERO < fBpm) {
            setBpm(fBpm);
        }
    }
}

void SoundSource::setReplayGainString (QString sReplayGain) {
    setReplayGain(parseReplayGainString(sReplayGain));
}

} //namespace Mixxx
