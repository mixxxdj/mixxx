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

#include "encoder/encoderbroadcastsettings.h"
#include "broadcast/defs_broadcast.h"

#define DEFAULT_BITRATE 128

EncoderBroadcastSettings::EncoderBroadcastSettings(UserSettingsPointer pConfig)
{
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
EncoderBroadcastSettings::~EncoderBroadcastSettings()
{
    
}


QList<int> EncoderBroadcastSettings::getQualityValues() const
{
    return m_qualList;
}

// Sets the value
void EncoderBroadcastSettings::setQualityByValue(int qualityValue) 
{
    if (m_qualList.contains(qualityValue)) {
        m_pConfig->set(ConfigKey(BROADCAST_PREF_KEY, "bitrate"), 
                ConfigValue(qualityValue));
    } else {
        qWarning() << "Invalid qualityValue given to EncoderBroadcastSettings: " 
            << qualityValue << ". Ignoring it";
    }
}

void EncoderBroadcastSettings::setQualityByIndex(int qualityIndex)
{
    if (qualityIndex >= 0 && qualityIndex < m_qualList.size()) {
        m_pConfig->set(ConfigKey(BROADCAST_PREF_KEY, "bitrate"), 
            ConfigValue(m_qualList.at(qualityIndex)));
    } else {
        qWarning() << "Invalid qualityIndex given to EncoderBroadcastSettings: " 
            << qualityIndex << ". Ignoring it";
    }
}

int EncoderBroadcastSettings::getQuality() const
{
    int bitrate = m_pConfig->getValue(
            ConfigKey(BROADCAST_PREF_KEY, "bitrate"), DEFAULT_BITRATE);
    if (m_qualList.contains(bitrate)) {
        return bitrate;
    }
    else {
        qWarning() << "Invalid bitrate in EncoderBroadcastSettings " 
            << bitrate << ". Ignoring it and returning default";
    }
    return DEFAULT_BITRATE;
}
int EncoderBroadcastSettings::getQualityIndex() const
{
    return m_qualList.indexOf(getQuality());
}
