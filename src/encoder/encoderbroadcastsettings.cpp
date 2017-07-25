/**
* @file encoderbroadcastsettings.cpp
* @author Josep Maria Antolín
* @date Feb 27 2017
* @brief storage of broadcast settings for the encoders.
*/

#include "encoder/encoderbroadcastsettings.h"
#include "broadcast/defs_broadcast.h"

#define DEFAULT_BITRATE 128

EncoderBroadcastSettings::EncoderBroadcastSettings(
        BroadcastSettingsPointer settings)
        : m_settings(settings) {
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
}
EncoderBroadcastSettings::~EncoderBroadcastSettings() {
}

QList<int> EncoderBroadcastSettings::getQualityValues() const {
    return m_qualList;
}

// Sets the value
void EncoderBroadcastSettings::setQualityByValue(int qualityValue) {
    const BroadcastProfilePtr& profile = m_settings->profileAt(0);

    if (m_qualList.contains(qualityValue)) {
        profile->setBitrate(qualityValue);
    } else {
        qWarning() << "Invalid qualityValue given to EncoderBroadcastSettings: " 
            << qualityValue << ". Ignoring it";
    }
}

void EncoderBroadcastSettings::setQualityByIndex(int qualityIndex) {
    const BroadcastProfilePtr& profile = m_settings->profileAt(0);

    if (qualityIndex >= 0 && qualityIndex < m_qualList.size()) {
        profile->setBitrate(m_qualList.at(qualityIndex));
    } else {
        qWarning() << "Invalid qualityIndex given to EncoderBroadcastSettings: " 
            << qualityIndex << ". Ignoring it";
    }
}

int EncoderBroadcastSettings::getQuality() const {
    const BroadcastProfilePtr& profile = m_settings->profileAt(0);

    int bitrate = profile->getBitrate();
    if (m_qualList.contains(bitrate)) {
        return bitrate;
    }
    else {
        qWarning() << "Invalid bitrate in EncoderBroadcastSettings " 
            << bitrate << ". Ignoring it and returning default";
    }
    return DEFAULT_BITRATE;
}

int EncoderBroadcastSettings::getQualityIndex() const {
    return m_qualList.indexOf(getQuality());
}

void EncoderBroadcastSettings::setChannelMode(EncoderSettings::ChannelMode mode)
{
    const BroadcastProfilePtr& profile = m_settings->profileAt(0);
    profile->setChannels(static_cast<int>(mode));
}

EncoderSettings::ChannelMode EncoderBroadcastSettings::getChannelMode() const {
    const BroadcastProfilePtr& profile = m_settings->profileAt(0);

    switch(profile->getChannels()) {
        case 1: return EncoderSettings::ChannelMode::MONO;
        case 2: return EncoderSettings::ChannelMode::STEREO;
        case 0: // fallthrough
        default: return EncoderSettings::ChannelMode::AUTOMATIC;
    }
}

