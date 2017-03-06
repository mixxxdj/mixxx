/**
* @file encodermp3settings.cpp
* @author Josep Maria Antolín
* @date Feb 27 2017
* @brief storage of setting for mp3 encoder
*/

#include "encoder/encodermp3settings.h"
#include "recording/defs_recording.h"

const int EncoderMp3Settings::DEFAULT_BITRATE_INDEX = 6;
const QString EncoderMp3Settings::ENCODING_MODE_GROUP = "MP3_VBR_MODE";

EncoderMp3Settings::EncoderMp3Settings(UserSettingsPointer pConfig) :
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
    
    QList<QString> vbrmodes;
    vbrmodes.append("CBR");
    vbrmodes.append("ABR");
    vbrmodes.append("VBR");
    m_radioList.append(OptionsGroup(QObject::tr("Bitrate Mode"), ENCODING_MODE_GROUP, vbrmodes));
}
EncoderMp3Settings::~EncoderMp3Settings()
{
    
}


QList<int> EncoderMp3Settings::getQualityValues() const
{
    return m_qualList;
}

// Sets the value
void EncoderMp3Settings::setQualityByValue(int qualityValue) 
{
    if (m_qualList.contains(qualityValue)) {
        m_pConfig->set(ConfigKey(RECORDING_PREF_KEY, "MP3_Quality"), 
                ConfigValue(m_qualList.indexOf(qualityValue)));
    } else {
        qWarning() << "Invalid qualityValue given to EncoderMp3Settings: " 
            << qualityValue << ". Ignoring it";
    }
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
        qWarning() << "Invalid qualityIndex in EncoderMp3Settings " 
            << qualityIndex << ". Ignoring it and returning default";
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
void EncoderMp3Settings::setGroupOption(QString groupCode, int optionIndex)
{
    bool found=false;
    for (const auto& group : m_radioList) {
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
int EncoderMp3Settings::getSelectedOption(QString groupCode) const
{
    bool found=false;
    int value = m_pConfig->getValue(
        ConfigKey(RECORDING_PREF_KEY, ENCODING_MODE_GROUP), 0);
    for (const auto& group : m_radioList) {
        if (groupCode == group.groupCode) {
            found=true;
            if (value >= group.controlNames.size() && value > 1) {
                qWarning() << "Value saved for " << groupCode << 
                    " on preferences is out of range " << value << ". Returning 0";
                value=0;
            }
        }
    }
    if (!found) {
        qWarning() << "Received an unknown groupCode on getSelectedOption: " << groupCode;
    }
    return value;
}
