// storage of setting for mp3 encoder

#include "encoder/encodermp3settings.h"
#include "recording/defs_recording.h"

const int EncoderMp3Settings::DEFAULT_BITRATE_INDEX = 6;
const QString EncoderMp3Settings::ENCODING_MODE_GROUP = "MP3_VBR_MODE";

EncoderMp3Settings::EncoderMp3Settings(UserSettingsPointer pConfig) :
    m_pConfig(pConfig)
{
    // Added "32" because older settings started at index 1 with 48.
    m_qualList.append(32); // mono
    m_qualList.append(48); // mono
    m_qualList.append(64); // mono
    m_qualList.append(80); // mono
    m_qualList.append(96); // mono
    m_qualList.append(112); // stereo
    m_qualList.append(128); // stereo
    m_qualList.append(160); // stereo
    m_qualList.append(192); // stereo
    m_qualList.append(224); // stereo
    m_qualList.append(256); // stereo
    m_qualList.append(320); // stereo

    m_qualVBRList.append(55);// V7 mono
    m_qualVBRList.append(65);// V6 mono
    m_qualVBRList.append(75);// V5 mono
    m_qualVBRList.append(85);// V4 mono
    m_qualVBRList.append(110);// V7
    m_qualVBRList.append(120);// V6
    m_qualVBRList.append(140);// V5
    m_qualVBRList.append(160);// V4
    m_qualVBRList.append(190);// V3
    m_qualVBRList.append(200);// V2
    m_qualVBRList.append(240);// V1
    m_qualVBRList.append(260);// V0

    QList<QString> vbrmodes;
    vbrmodes.append("CBR");
    vbrmodes.append("ABR");
    vbrmodes.append("VBR");
    m_radioList.append(OptionsGroup(QObject::tr("Bitrate Mode"), ENCODING_MODE_GROUP, vbrmodes));
}

QList<int> EncoderMp3Settings::getQualityValues() const
{
    return m_qualList;
}

QList<int> EncoderMp3Settings::getVBRQualityValues() const
{
    return m_qualVBRList;
}

void EncoderMp3Settings::setQualityByIndex(int qualityIndex)
{
    if (qualityIndex >= 0 && qualityIndex < m_qualList.size()) {
        m_pConfig->set(ConfigKey(RECORDING_PREF_KEY, "MP3_Quality"), ConfigValue(qualityIndex));
    } else {
        qWarning() << "Invalid qualityIndex given to EncoderMp3Settings: "
                   << qualityIndex << ". Ignoring it";
    }
}

int EncoderMp3Settings::getQuality() const
{
    return m_qualList.at(getQualityIndex());
}
int EncoderMp3Settings::getQualityIndex() const
{
    int qualityIndex = m_pConfig->getValue(
            ConfigKey(RECORDING_PREF_KEY, "MP3_Quality"), DEFAULT_BITRATE_INDEX);
    if (qualityIndex >= 0 && qualityIndex < m_qualList.size()) {
        return qualityIndex;
    }
    else {
        qWarning() << "Invalid qualityIndex in EncoderMp3Settings"
                   << qualityIndex << "(Max is:" << m_qualList.size() << "). Ignoring it"
                   << "and returning default, which is:" << DEFAULT_BITRATE_INDEX;
    }
    return DEFAULT_BITRATE_INDEX;
}

// Returns the list of radio options to show to the user
QList<EncoderSettings::OptionsGroup> EncoderMp3Settings::getOptionGroups() const
{
    return m_radioList;
}
// Selects the option by its index. If it is a single-element option,
// index 0 means disabled and 1 enabled.
void EncoderMp3Settings::setGroupOption(const QString& groupCode, int optionIndex) {
    bool found=false;
    for (const auto& group : qAsConst(m_radioList)) {
        if (groupCode == group.groupCode) {
            found=true;
            if (optionIndex < group.controlNames.size() || optionIndex == 1) {
                m_pConfig->set(ConfigKey(RECORDING_PREF_KEY, ENCODING_MODE_GROUP),
                    ConfigValue(optionIndex));
            } else {
                qWarning() << "Received an index out of range for: "
                           << groupCode << ", index: " << optionIndex;
            }
        }
    }
    if (!found) {
        qWarning() << "Received an unknown groupCode on setGroupOption: " << groupCode;
    }
}
// Return the selected option of the group. If it is a single-element option,
// 0 means disabled and 1 enabled.
int EncoderMp3Settings::getSelectedOption(const QString& groupCode) const {
    bool found=false;
    int value = m_pConfig->getValue(
        ConfigKey(RECORDING_PREF_KEY, ENCODING_MODE_GROUP), 0);
    for (const auto& group : m_radioList) {
        if (groupCode == group.groupCode) {
            found=true;
            if (value >= group.controlNames.size() && value > 1) {
                qWarning() << "Value saved for " << groupCode
                           << " on preferences is out of range " << value
                           << ". Returning 0";
                value=0;
            }
        }
    }
    if (!found) {
        qWarning() << "Received an unknown groupCode on getSelectedOption: " << groupCode;
    }
    return value;
}
