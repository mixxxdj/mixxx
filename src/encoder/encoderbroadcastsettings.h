/**
* @file encoderbroadcastsettings.h
* @author Josep Maria Antolín
* @date Feb 27 2017
* @brief storage of broadcast settings for the encoders.
*/


#ifndef ENCODERBROADCASTSETTINGS_H
#define ENCODERBROADCASTSETTINGS_H

#include "encoder/encodersettings.h"
#include "encoder/encoder.h"
#include "preferences/broadcastsettings.h"

class EncoderBroadcastSettings : public EncoderSettings {
  public:
    EncoderBroadcastSettings(BroadcastSettings settings);
    virtual ~EncoderBroadcastSettings();

    // Indicates that it uses the quality slider section of the preferences
    bool usesQualitySlider() const override;
    // Indicates that it uses the compression slider section of the preferences
    bool usesCompressionSlider() const override;
    // Indicates that it uses the radio button section of the preferences.
    bool usesOptionGroups() const override;

    // Returns the list of quality values that it supports, to assign them to the slider
    QList<int> getQualityValues() const override;
    // Sets the quality value by its value
    void setQualityByValue(int qualityValue) override;
    // Sets the quality value by its index
    void setQualityByIndex(int qualityIndex) override;
    // Returns the current quality value
    int getQuality() const override;
    int getQualityIndex() const override;
    void setChannelMode(ChannelMode mode) override;
    ChannelMode getChannelMode() const override;

  private:
    QList<int> m_qualList;
    BroadcastSettings m_settings;
};


inline bool EncoderBroadcastSettings::usesQualitySlider() const
{
    return true;
}
inline bool EncoderBroadcastSettings::usesCompressionSlider() const
{
    return false;
}
inline bool EncoderBroadcastSettings::usesOptionGroups() const
{
    return false;
}

#endif // ENCODERBROADCASTSETTINGS_H
