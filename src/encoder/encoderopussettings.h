#pragma once

#include "encoder/encoderrecordingsettings.h"
#include "encoder/encoder.h"
#include "recording/defs_recording.h"

#define OPUS_BITRATE_MODES_COUNT 3
#define OPUS_BITRATE_CONSTRAINED_VBR 0
#define OPUS_BITRATE_CBR 1
#define OPUS_BITRATE_VBR 2

class EncoderOpusSettings: public EncoderRecordingSettings {
  public:
    explicit EncoderOpusSettings(UserSettingsPointer pConfig);

    // Indicates that it uses the quality slider section of the preferences
    bool usesQualitySlider() const override {
        return true;
    }

    // Returns the list of quality values that it supports, to assign them to the slider
    QList<int> getQualityValues() const override;
    // Sets the quality value by its index
    void setQualityByIndex(int qualityIndex) override;
    // Returns the current quality value
    int getQuality() const override;
    int getQualityIndex() const override;

    // Returns the list of radio options to show to the user
    QList<OptionsGroup> getOptionGroups() const override;
    // Selects the option by its index. If it is a single-element option,
    // index 0 means disabled and 1 enabled.
    void setGroupOption(const QString& groupCode, int optionIndex) override;
    // Return the selected option of the group. If it is a single-element option,
    // 0 means disabled and 1 enabled.
    int getSelectedOption(const QString& groupCode) const override;

    // Returns the format of this encoder settings.
    QString getFormat() const override {
        return ENCODING_OPUS;
    }

    static const QString BITRATE_MODE_GROUP;

  private:
    UserSettingsPointer m_pConfig;
    QList<int> m_qualList;
    QList<OptionsGroup> m_radioList;
};
