#include <QMap>

#include "encoder/encoderopussettings.h"
#include "recording/defs_recording.h"
#include "util/logger.h"
#include "util/compatibility.h"

namespace {
const int kDefaultQualityIndex = 6;
const char* kQualityKey = "Opus_Quality";
const mixxx::Logger kLogger("EncoderOpusSettings");
} // namespace

const QString EncoderOpusSettings::BITRATE_MODE_GROUP = "Opus_BitrateMode";

EncoderOpusSettings::EncoderOpusSettings(UserSettingsPointer pConfig)
    : m_pConfig(pConfig) {
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
    m_qualList.append(510); // Max Opus bitrate

    QMap<int, QString> modes;
    modes.insert(OPUS_BITRATE_CONSTRAINED_VBR,
            QObject::tr("Constrained VBR"));
    modes.insert(OPUS_BITRATE_CBR, QObject::tr("CBR"));
    modes.insert(OPUS_BITRATE_VBR, QObject::tr("Full VBR (bitrate ignored)"));

    // OptionsGroup needs a QList<string> for the option list. Using QMap::values()
    // ensures that the returned QList<string> will be sorted in ascending key order, which
    // is what we want to be able to match OPUS_BITRATE_* number defines to the right mode
    // in EncoderOpus.
    m_radioList.append(OptionsGroup(
            QObject::tr("Bitrate Mode"), BITRATE_MODE_GROUP, modes.values()));
}

QList<int> EncoderOpusSettings::getQualityValues() const {
    return m_qualList;
}

void EncoderOpusSettings::setQualityByIndex(int qualityIndex) {
    if (qualityIndex >= 0 && qualityIndex < m_qualList.size()) {
        m_pConfig->setValue<int>(ConfigKey(RECORDING_PREF_KEY, kQualityKey), qualityIndex);
    } else {
        kLogger.warning() << "Invalid qualityIndex given to EncoderVorbisSettings: "
                          << qualityIndex << ". Ignoring it";
    }
}
int EncoderOpusSettings::getQuality() const {
    int qualityIndex = m_pConfig->getValue(
            ConfigKey(RECORDING_PREF_KEY, kQualityKey), kDefaultQualityIndex);

    if (qualityIndex >= 0 && qualityIndex < m_qualList.size()) {
        return m_qualList.at(qualityIndex);
    } else {
        // Same as Vorbis: Opus does not have a fixed set of
        // bitrates, so we can accept any value.
        return qualityIndex;
    }
}

int EncoderOpusSettings::getQualityIndex() const {
    int qualityIndex = m_pConfig->getValue(
            ConfigKey(RECORDING_PREF_KEY, kQualityKey), kDefaultQualityIndex);

    if (qualityIndex >= 0 && qualityIndex < m_qualList.size()) {
        return qualityIndex;
    } else {
        kLogger.warning() << "Invalid qualityIndex in EncoderVorbisSettings "
                          << qualityIndex << "(Max is:"
                          << m_qualList.size() << ") . Ignoring it and"
                          << "returning default which is" << kDefaultQualityIndex;
        return kDefaultQualityIndex;
    }
}

QList<EncoderSettings::OptionsGroup> EncoderOpusSettings::getOptionGroups() const {
    return m_radioList;
}

void EncoderOpusSettings::setGroupOption(const QString& groupCode, int optionIndex) {
    bool found = false;
    for (const OptionsGroup& group : qAsConst(m_radioList)) {
        if (groupCode == group.groupCode) {
            found = true;
            if (optionIndex < group.controlNames.size() || optionIndex == 1) {
                m_pConfig->set(
                        ConfigKey(RECORDING_PREF_KEY, BITRATE_MODE_GROUP),
                        ConfigValue(optionIndex));
            } else {
                kLogger.warning()
                        << "Received an index out of range for:"
                        << groupCode << ", index:" << optionIndex;
            }
        }
    }

    if (!found) {
        kLogger.warning()
                << "Received an unknown groupCode on setGroupOption:" << groupCode;
    }
}

int EncoderOpusSettings::getSelectedOption(const QString& groupCode) const {
    int value = m_pConfig->getValue(
            ConfigKey(RECORDING_PREF_KEY, BITRATE_MODE_GROUP), 0);

    bool found = false;
    for (const OptionsGroup& group : qAsConst(m_radioList)) {
        if (groupCode == group.groupCode) {
            found = true;
            if (value >= group.controlNames.size() && value > 1) {
                kLogger.warning()
                        << "Value saved for" << groupCode
                        << "on preferences is out of range" << value << ". Returning 0";
                value = 0;
            }
        }
    }

    if (!found) {
        kLogger.warning()
                << "Received an unknown groupCode on getSelectedOption:" << groupCode;
    }

    return value;
}
