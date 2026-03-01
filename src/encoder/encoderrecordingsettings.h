#pragma once

#include "encoder/encodersettings.h"
#include "preferences/usersettings.h"
#include "recording/defs_recording.h"
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
        DEBUG_ASSERT_UNREACHABLE(!"unimplemented");
    }

    // Sets the compression level
    void setCompression(int compression) override {
        Q_UNUSED(compression);
        DEBUG_ASSERT_UNREACHABLE(!"unimplemented");
    }

    // Selects the option by its index. If it is a single-element option,
    // index 0 means disabled and 1 enabled.
    virtual void setGroupOption(const QString& groupCode, int optionIndex) {
        Q_UNUSED(groupCode);
        Q_UNUSED(optionIndex);
        DEBUG_ASSERT_UNREACHABLE(!"unimplemented");
    }

    // Override getChannelMode to read from recording preferences but making it
    // so that it fallbacks to stereo if no config is available (e.g. in
    // broadcasting context)
    ChannelMode getChannelMode() const override {
        UserSettingsPointer config = getConfig();
        if (!config) {
            // Fallback if no config (shouldn't happen in recording context)
            return ChannelMode::STEREO;
        }

        int channelMode = config->getValue<int>(
                ConfigKey(RECORDING_PREF_KEY, "channel_mode"), 0);

        switch (channelMode) {
        case 1:
            return ChannelMode::MONO;
        case 0: // fallthrough
        default:
            return ChannelMode::STEREO;
        }
    }

  protected:
    // Subclasses override this to provide their m_pConfig
    // Base class returns null pointer
    virtual UserSettingsPointer getConfig() const {
        return UserSettingsPointer();
    }
};

typedef std::shared_ptr<EncoderRecordingSettings> EncoderRecordingSettingsPointer;
