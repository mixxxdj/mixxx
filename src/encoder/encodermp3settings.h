#pragma once

#include "encoder/encoderrecordingsettings.h"
#include "encoder/encoder.h"
#include "recording/defs_recording.h"

// Storage of settings for MP3 encoder
class EncoderMp3Settings : public EncoderRecordingSettings {
  public:
    EncoderMp3Settings(UserSettingsPointer m_pConfig);
    ~EncoderMp3Settings() override = default;

    // Indicates that it uses the quality slider section of the preferences
    bool usesQualitySlider() const override {
        return true;
    }
    // Returns the list of quality values that it supports, to assign them to the slider
    QList<int> getQualityValues() const override;
    QList<int> getVBRQualityValues() const;
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
        return ENCODING_MP3;
    }

    static const int DEFAULT_BITRATE_INDEX;
    static const QString ENCODING_MODE_GROUP;
  private:
    QList<OptionsGroup> m_radioList;
    QList<int> m_qualList;
    QList<int> m_qualVBRList;
    UserSettingsPointer m_pConfig;
};
