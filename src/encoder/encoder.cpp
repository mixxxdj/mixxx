/****************************************************************************
                   encoder.cpp  - encoder API for mixxx
                             -------------------
    copyright            : (C) 2009 by Phillip Whelan
    copyright            : (C) 2010 by Tobias Rafreider
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "encoder/encoder.h"
#include "preferences/usersettings.h"
#include "recording/defs_recording.h"
// TODO: Note: this is incomplete, since encoderffmpegmp3.cpp (and vorbis)
// are not compiled in any case.
#ifdef __FFMPEGFILE__
#include "encoder/encoderffmpegmp3.h"
#include "encoder/encoderffmpegvorbis.h"
#else
#include "encoder/encodermp3.h"
#include "encoder/encodervorbis.h"
#endif
#include "encoder/encoderwave.h"
#include "encoder/encodersndfileflac.h"
#include "encoder/encodermp3settings.h"
#include "encoder/encodervorbissettings.h"
#include "encoder/encoderwavesettings.h"
#include "encoder/encoderflacsettings.h"

#include <QList>

EncoderFactory EncoderFactory::factory;

const EncoderFactory& EncoderFactory::getFactory()
{
    return factory;
}

EncoderFactory::EncoderFactory() {
    // Add new supported formats here. Also modify the getNewEncoder/getEncoderSettings method.
    m_formats.append(Encoder::Format("Wave PCM",ENCODING_WAVE, true));
    m_formats.append(Encoder::Format("AIFF PCM",ENCODING_AIFF, true));
    m_formats.append(Encoder::Format("Flac", ENCODING_FLAC, true));
    m_formats.append(Encoder::Format("Mp3",ENCODING_MP3, false));
    m_formats.append(Encoder::Format("Ogg Vorbis",ENCODING_OGG, false));
}

const QList<Encoder::Format> EncoderFactory::getFormats() const
{
    return m_formats;
}

Encoder::Format EncoderFactory::getSelectedFormat(UserSettingsPointer pConfig) const
{
    QString encoding = pConfig->getValueString(ConfigKey(RECORDING_PREF_KEY, "Encoding"));
    foreach(Encoder::Format format, m_formats) {
        if (format.internalName == encoding) {
            return format;
        }
    }
    qWarning() << "Format: " << encoding << " not recognized! Returning format WAV";
    return m_formats.first();
}
Encoder::Format EncoderFactory::getFormatFor(QString formatText) const
{
    foreach(Encoder::Format format, m_formats) {
        if (format.internalName == formatText) {
            return format;
        }
    }
    qWarning() << "Format: " << formatText << " not recognized! Returning format WAV";
    return m_formats.first();
}

EncoderPointer EncoderFactory::getNewEncoder(Encoder::Format format,
    UserSettingsPointer pConfig, EncoderCallback* pCallback) const
{
    EncoderPointer pEncoder;
    if (format.internalName == ENCODING_WAVE) {
        pEncoder = std::make_shared<EncoderWave>(pCallback);
        pEncoder->setEncoderSettings(EncoderWaveSettings(pConfig, format));
    } else if (format.internalName == ENCODING_AIFF) {
        pEncoder = std::make_shared<EncoderWave>(pCallback);
        pEncoder->setEncoderSettings(EncoderWaveSettings(pConfig, format));
    } else if (format.internalName == ENCODING_FLAC) {
        pEncoder = std::make_shared<EncoderSndfileFlac>(pCallback);
        pEncoder->setEncoderSettings(EncoderFlacSettings(pConfig));
    } else if (format.internalName == ENCODING_MP3) {
        #ifdef __FFMPEGFILE__
        pEncoder = std::make_shared<EncoderFfmpegMp3>(pCallback);
        #else
        pEncoder = std::make_shared<EncoderMp3>(pCallback);
        #endif
        pEncoder->setEncoderSettings(EncoderMp3Settings(pConfig));
    } else if (format.internalName == ENCODING_OGG) {
        #ifdef __FFMPEGFILE__
        pEncoder = std::make_shared<EncoderFfmpegVorbis>(pCallback);
        #else
        pEncoder = std::make_shared<EncoderVorbis>(pCallback);
        #endif
        pEncoder->setEncoderSettings(EncoderVorbisSettings(pConfig));
    } else {
        qWarning() << "Unsuported format requested! " << format.internalName;
        // I am unsure what to do here. an assert(false) maybe? Just returning wav settings for now
        pEncoder = std::make_shared<EncoderWave>(pCallback);
        pEncoder->setEncoderSettings(EncoderWaveSettings(pConfig, format));
    }
    return pEncoder;
}

EncoderSettingsPointer EncoderFactory::getEncoderSettings(Encoder::Format format,
    UserSettingsPointer pConfig) const
{
    if (format.internalName == ENCODING_WAVE) {
        return std::make_shared<EncoderWaveSettings>(pConfig, format);
    } else if (format.internalName == ENCODING_AIFF) {
        return std::make_shared<EncoderWaveSettings>(pConfig, format);
    } else if (format.internalName == ENCODING_FLAC) {
        return std::make_shared<EncoderFlacSettings>(pConfig);
    } else if (format.internalName == ENCODING_MP3) {
        return std::make_shared<EncoderMp3Settings>(pConfig);
    } else if (format.internalName == ENCODING_OGG) {
        return std::make_shared<EncoderVorbisSettings>(pConfig);
    } else {
        qWarning() << "Unsuported format requested! " << format.internalName;
        // I am unsure what to do here. an assert(false) maybe? Just returning wav settings for now
        return std::make_shared<EncoderWaveSettings>(pConfig, format);
    }
}
