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
}

QList<int> EncoderVorbisSettings::getQualityValues() const
{
    return m_qualList;
}

void EncoderVorbisSettings::setQualityByIndex(int qualityIndex)
{
    if (qualityIndex >= 0 && qualityIndex < m_qualList.size()) {
        m_pConfig->setValue<int>(ConfigKey(RECORDING_PREF_KEY, "OGG_Quality"), qualityIndex);
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
    if (qualityIndex >= 0 && qualityIndex < m_qualList.size()) {
        return qualityIndex;
    }
    else {
        qWarning() << "Invalid qualityIndex in EncoderVorbisSettings "
                   << qualityIndex << "(Max is:" << m_qualList.size() << ") . Ignoring it and"
                   << "returning default which is" << DEFAULT_BITRATE_INDEX;
        return DEFAULT_BITRATE_INDEX;
    }
}
