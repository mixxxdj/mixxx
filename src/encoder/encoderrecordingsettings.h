#pragma once

#include "encoder/encodersettings.h"

class EncoderRecordingSettings : public EncoderSettings {
  public:
    ~EncoderRecordingSettings() override = default;

    // Indicates that it uses the quality slider section of the preferences
    virtual bool usesQualitySlider() const = 0;
    // Indicates that it uses the compression slider section of the preferences
    virtual bool usesCompressionSlider() const = 0;
    // Indicates that it uses the radio button section of the preferences.
    virtual bool usesOptionGroups() const = 0;

    virtual void setQualityByIndex(int qualityIndex) {
        Q_UNUSED(qualityIndex);
    }

    // Sets the compression level
    virtual void setCompression(int compression) {
        Q_UNUSED(compression);
    }

    // Selects the option by its index. If it is a single-element option, 
    // index 0 means disabled and 1 enabled.
    virtual void setGroupOption(QString groupCode, int optionIndex) {
        Q_UNUSED(groupCode);
        Q_UNUSED(optionIndex);
    }
};

typedef std::shared_ptr<EncoderRecordingSettings> EncoderRecordingSettingsPointer;

