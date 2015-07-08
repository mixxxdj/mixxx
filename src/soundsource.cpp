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

#include "soundsource.h"

namespace Mixxx {

namespace {
const float BPM_ZERO = 0.0f;
const float BPM_MAX = 300.0f;

float parseBpmString(const QString& sBpm) {
    float bpm = sBpm.toFloat();
    while (bpm > BPM_MAX) {
        bpm /= 10.0f;
    }
    return bpm;
}

float parseReplayGainString(QString sReplayGain) {
    QString ReplayGainstring = sReplayGain.remove(" dB");
    float fReplayGain = db2ratio(ReplayGainstring.toFloat());
    // I found some mp3s of mine with replaygain tag set to 0dB even if not normalized.
    // This is because of Rapid Evolution 3, I suppose. I prefer to rescan them by setting value to 0 (i.e. rescan via analyserrg)
    if (fReplayGain == 1.0f) {
        fReplayGain = 0.0f;
    }
    return fReplayGain;
}

}

SoundSource::SoundSource(QString sFilename)
        : m_sFilename(sFilename), m_sType(
                m_sFilename.section(".", -1).toLower()), m_fReplayGain(0.0f), m_fBpm(
                BPM_ZERO), m_iChannels(0), m_iSampleRate(0), m_iBitrate(0), m_iDuration(0) {
}

SoundSource::SoundSource(QString sFilename, QString sType)
        : m_sFilename(sFilename), m_sType(sType), m_fReplayGain(0.0f), m_fBpm(
                BPM_ZERO), m_iChannels(0), m_iSampleRate(0), m_iBitrate(0), m_iDuration(0) {
}

SoundSource::~SoundSource() {
}

void SoundSource::setBpmString(QString sBpm) {
    if (!sBpm.isEmpty()) {
        float fBpm = parseBpmString(sBpm);
        if (BPM_ZERO < fBpm) {
            setBpm(fBpm);
        }
    }
}

void SoundSource::setReplayGainString(QString sReplayGain) {
    setReplayGain(parseReplayGainString(sReplayGain));
}

int SoundSource::getChannels() const {
    if (isChannelCountValid()) {
        // from AudioSource
        return getChannelCount();
    } else {
        // from metadata
        return m_iChannels;
    }
}

int SoundSource::getSampleRate() const {
    if (isFrameRateValid()) {
        // from AudioSource
        return getFrameRate();
    } else {
        // from metadata
        return m_iSampleRate;
    }
}

int SoundSource::getDuration() const {
    if (isFrameRateValid() && !isFrameCountEmpty()) {
        // from AudioSource
        return getFrameCount() / getFrameRate();
    } else {
        // from metadata
        return m_iDuration;
    }
}

} //namespace Mixxx
