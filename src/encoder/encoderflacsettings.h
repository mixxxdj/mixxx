#pragma once

#include "encoder/encoderrecordingsettings.h"
#include "encoder/encoder.h"
#include "recording/defs_recording.h"

/// Storage of settings for FLAC encoder
class EncoderFlacSettings : public EncoderRecordingSettings {
  public:
    EncoderFlacSettings(UserSettingsPointer pConfig);
    ~EncoderFlacSettings() override = default;

    // Indicates that it uses the compression slider section of the preferences
    bool usesCompressionSlider() const override;

    // Returns the list of compression values supported, to assign them to the slider
    virtual QList<int> getCompressionValues() const override;
    // Sets the compression level
    virtual void setCompression(int compression) override;
    virtual int getCompression() const override;
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
        return ENCODING_FLAC;
    }

    static const int DEFAULT_QUALITY_VALUE;
    static const QString BITS_GROUP;
    static const QString GROUP_COMPRESSION;
  private:
    QList<OptionsGroup> m_radioList;
    QList<int> m_qualList;
    UserSettingsPointer m_pConfig;
};
