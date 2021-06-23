#include "encoder/encoderwavesettings.h"

#include "recording/defs_recording.h"

const QString EncoderWaveSettings::BITS_GROUP = "BITS";

EncoderWaveSettings::EncoderWaveSettings(UserSettingsPointer pConfig,
        QString format)
        : m_pConfig(pConfig),
          m_format(std::move(format)) {
    VERIFY_OR_DEBUG_ASSERT(m_format == ENCODING_WAVE || m_format == ENCODING_AIFF) {
        m_format = ENCODING_WAVE;
        qWarning() << "EncoderWaveSettings setup with " << m_format
                   << ". This is an error! Changing it to " << m_format;
    }
    QList<QString> names;
    names.append(QObject::tr("16 bits"));
    names.append(QObject::tr("24 bits"));
    names.append(QObject::tr("32 bits float"));
    m_radioList.append(OptionsGroup(QObject::tr("Bit depth"), BITS_GROUP, names));
}

// Returns the list of radio options to show to the user
QList<EncoderSettings::OptionsGroup> EncoderWaveSettings::getOptionGroups() const
{
    return m_radioList;
}

// Selects the option by its index. If it is a single-element option,
// index 0 means disabled and 1 enabled.
void EncoderWaveSettings::setGroupOption(const QString& groupCode, int optionIndex) {
    bool found=false;
    for (const auto& group : qAsConst(m_radioList)) {
        if (groupCode == group.groupCode) {
            found=true;
            if (optionIndex < group.controlNames.size() || optionIndex == 1) {
                m_pConfig->set(
                        ConfigKey(RECORDING_PREF_KEY, m_format + "_" + groupCode),
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
int EncoderWaveSettings::getSelectedOption(const QString& groupCode) const {
    bool found=false;
    int value = m_pConfig->getValue(
            ConfigKey(RECORDING_PREF_KEY, m_format + "_" + groupCode), 0);
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
