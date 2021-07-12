#pragma once

#include "encoder/encodersettings.h"
#include "util/assert.h"

class EncoderRecordingSettings : public EncoderSettings {
  public:
    // Indicates that it uses the quality slider section of the preferences
    virtual bool usesQualitySlider() const {
        return false;
    }

    // Indicates that it uses the compression slider section of the preferences
    virtual bool usesCompressionSlider() const {
        return false;
    }

    virtual void setQualityByIndex(int qualityIndex) {
        Q_UNUSED(qualityIndex);
        DEBUG_ASSERT(!"unimplemented");
    }

    // Sets the compression level
    void setCompression(int compression) override {
        Q_UNUSED(compression);
        DEBUG_ASSERT(!"unimplemented");
    }

    // Selects the option by its index. If it is a single-element option,
    // index 0 means disabled and 1 enabled.
    virtual void setGroupOption(const QString& groupCode, int optionIndex) {
        Q_UNUSED(groupCode);
        Q_UNUSED(optionIndex);
        DEBUG_ASSERT(!"unimplemented");
    }
};

typedef std::shared_ptr<EncoderRecordingSettings> EncoderRecordingSettingsPointer;
