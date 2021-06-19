#include "encoder/encoderflacsettings.h"
#include "recording/defs_recording.h"
#include <sndfile.h>

const int EncoderFlacSettings::DEFAULT_QUALITY_VALUE = 5;
const QString EncoderFlacSettings::BITS_GROUP = "FLAC_BITS";
const QString EncoderFlacSettings::GROUP_COMPRESSION = "FLAC_COMPRESSION";

EncoderFlacSettings::EncoderFlacSettings(UserSettingsPointer pConfig)
{
    m_pConfig = pConfig;

    QList<QString> names;
    names.append(QObject::tr("16 bits"));
    names.append(QObject::tr("24 bits"));
    m_radioList.append(OptionsGroup(QObject::tr("Bit depth"), BITS_GROUP, names));

    m_qualList.append(0);
    m_qualList.append(1);
    m_qualList.append(2);
    m_qualList.append(3);
    m_qualList.append(4);
    m_qualList.append(5);
    m_qualList.append(6);
    m_qualList.append(7);
    m_qualList.append(8);
}

bool EncoderFlacSettings::usesCompressionSlider() const
{
#if defined SFC_SUPPORTS_SET_COMPRESSION_LEVEL // Seems that this only exists since version 1.0.26
    return true;
#else
    return false;
#endif
}

// Returns the list of compression values supported, to assign them to the slider
QList<int> EncoderFlacSettings::getCompressionValues() const
{
    return m_qualList;
}
// Sets the compression level
void EncoderFlacSettings::setCompression(int compression) {
    if (m_qualList.contains(compression)) {
        m_pConfig->set(ConfigKey(RECORDING_PREF_KEY, GROUP_COMPRESSION),
            ConfigValue(compression));
    } else {
        qWarning() << "Received a compression value out of range: " << compression;
    }
}
int EncoderFlacSettings::getCompression() const
{
    int value = m_pConfig->getValue(ConfigKey(RECORDING_PREF_KEY, GROUP_COMPRESSION),
            DEFAULT_QUALITY_VALUE);
    if (!m_qualList.contains(value)) {
        qWarning() << "Value saved for compression on preferences is out of range "
                   << value << ". Returning default compression";
        value=DEFAULT_QUALITY_VALUE;
    }
    return value;
}


// Returns the list of radio options to show to the user
QList<EncoderSettings::OptionsGroup> EncoderFlacSettings::getOptionGroups() const
{
    return m_radioList;
}

// Selects the option by its index. If it is a single-element option,
// index 0 means disabled and 1 enabled.
void EncoderFlacSettings::setGroupOption(const QString& groupCode, int optionIndex) {
    bool found=false;
    for (const auto& group : qAsConst(m_radioList)) {
        if (groupCode == group.groupCode) {
            found=true;
            if (optionIndex < group.controlNames.size() || optionIndex == 1) {
                m_pConfig->set(ConfigKey(RECORDING_PREF_KEY, groupCode),
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
int EncoderFlacSettings::getSelectedOption(const QString& groupCode) const {
    bool found=false;
    int value = m_pConfig->getValue(ConfigKey(RECORDING_PREF_KEY, groupCode), 0);
    for (const auto&  group : m_radioList) {
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
