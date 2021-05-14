#include "encoder/encoderbroadcastsettings.h"
#include "broadcast/defs_broadcast.h"

#define DEFAULT_BITRATE 128

EncoderBroadcastSettings::EncoderBroadcastSettings(
        BroadcastProfilePtr profile)
        : m_pProfile(profile) {
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

QList<int> EncoderBroadcastSettings::getQualityValues() const {
    return m_qualList;
}

int EncoderBroadcastSettings::getQuality() const {
    int bitrate = m_pProfile->getBitrate();
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

EncoderSettings::ChannelMode EncoderBroadcastSettings::getChannelMode() const {
    switch(m_pProfile->getChannels()) {
        case 1: return EncoderSettings::ChannelMode::MONO;
        case 2: return EncoderSettings::ChannelMode::STEREO;
        case 0: // fallthrough
        default: return EncoderSettings::ChannelMode::AUTOMATIC;
    }
}

QString EncoderBroadcastSettings::getFormat() const {
    return m_pProfile->getFormat();
}
