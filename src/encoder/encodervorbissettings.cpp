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

#include "encoder/encodervorbissettings.h"
#include "recording/defs_recording.h"

#define DEFAULT_BITRATE_INDEX 6

EncoderVorbisSettings::EncoderVorbisSettings(UserSettingsPointer pConfig) :
    m_pConfig(pConfig)
{
    // Added "32" because older settings started at index 1 with 48.
    m_qualList.append(32);
    m_qualList.append(48);
    m_qualList.append(64);
    m_qualList.append(80);
    m_qualList.append(96);
    m_qualList.append(112);
    m_qualList.append(128);
    m_qualList.append(160);
    m_qualList.append(192);
    m_qualList.append(224);
    m_qualList.append(256);
    m_qualList.append(320);
    m_pConfig = m_pConfig;
}
EncoderVorbisSettings::~EncoderVorbisSettings()
{
    
}


QList<int> EncoderVorbisSettings::getQualityValues() const
{
    return m_qualList;
}
// Sets the value
void EncoderVorbisSettings::setQualityByValue(int qualityValue) 
{
    // Vorbis does not have a fixed set of bitrates, so we can accept any value.
    int indexValue;
    if (m_qualList.contains(qualityValue)) {
        indexValue = m_qualList.indexOf(qualityValue);
    } else {
    // If we let the user write a bitrate value, this would allow to save such value.
        indexValue = qualityValue;
    }
    m_pConfig->set(ConfigKey(RECORDING_PREF_KEY, "OGG_Quality"), ConfigValue(indexValue));
}

void EncoderVorbisSettings::setQualityByIndex(int qualityIndex)
{
    if (qualityIndex >= 0 && qualityIndex < m_qualList.size()) {
        m_pConfig->set(ConfigKey(RECORDING_PREF_KEY, "OGG_Quality"), ConfigValue(qualityIndex));
    } else {
        qWarning() << "Invalid qualityIndex given to EncoderVorbisSettings: " 
            << qualityIndex << ". Ignoring it";
    }
}
int EncoderVorbisSettings::getQuality() const
{
    int qualityIndex = m_pConfig->getValue(
            ConfigKey(RECORDING_PREF_KEY, "OGG_Quality"), DEFAULT_BITRATE_INDEX);
    if (qualityIndex >= 0 && qualityIndex < m_qualList.size()) {
        return m_qualList.at(qualityIndex);
    }
    else {
    // Vorbis does not have a fixed set of bitrates, so we can accept any value.
        return qualityIndex;
    }
}

int EncoderVorbisSettings::getQualityIndex() const
{
    int qualityIndex = m_pConfig->getValue(
            ConfigKey(RECORDING_PREF_KEY, "OGG_Quality"), DEFAULT_BITRATE_INDEX);
    if (qualityIndex < 0 || qualityIndex > m_qualList.size()) {
        return m_qualList.at(qualityIndex);
    }
    else {
        qWarning() << "Invalid qualityIndex in EncoderMp3Settings " 
            << qualityIndex << ". Ignoring it and returning default";
        return DEFAULT_BITRATE_INDEX;
    }
}