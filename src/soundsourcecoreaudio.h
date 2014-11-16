/**
 * \file soundsourcecoreaudio.h
 * \class SoundSourceCoreAudio
 * \brief Decodes M4As (etc) using the AudioToolbox framework included as
 *        part of Core Audio on OS X (and iOS).
 * \author Albert Santoni <alberts at mixxx dot org>
 * \date Dec 12, 2010
 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef SOUNDSOURCECOREAUDIO_H
#define SOUNDSOURCECOREAUDIO_H

#include <QString>
#include <AudioToolbox/AudioToolbox.h>
//In our tree at lib/apple/
#include "CAStreamBasicDescription.h"

#if !defined(__COREAUDIO_USE_FLAT_INCLUDES__)
#include <CoreServices/CoreServices.h>
#include <CoreAudio/CoreAudioTypes.h>
#include <AudioToolbox/AudioFile.h>
#include <AudioToolbox/AudioFormat.h>
#else
#include "CoreAudioTypes.h"
#include "AudioFile.h"
#include "AudioFormat.h"
#endif

#include <fcntl.h>

#include "util/types.h"
#include "util/defs.h"
#include "soundsource.h"

class SoundSourceCoreAudio : public Mixxx::SoundSource {
public:
    explicit SoundSourceCoreAudio(QString filename);
    ~SoundSourceCoreAudio();
    Result open();
    Result parseHeader();
    QImage parseCoverArt();
    static QList<QString> supportedFileExtensions();

    diff_type seekFrame(diff_type frameIndex);

protected:
    unsigned read(unsigned long size, SAMPLE* buffer);
private:
    ExtAudioFileRef m_audioFile;
    CAStreamBasicDescription m_inputFormat;
    CAStreamBasicDescription m_outputFormat;
    SInt64 m_headerFrames;
};


#endif // ifndef SOUNDSOURCECOREAUDIO_H
