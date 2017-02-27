/**
* @file encoderwavesettings.h
* @author Josep Maria Antolín
* @date Feb 27 2017
* @brief storage of setting for vorbis encoder
*/

#ifndef ENCODERVORBISSETTINGS_H
#define ENCODERVORBISSETTINGS_H

#include "encoder/encodersettings.h"
#include "encoder/encoder.h"

class EncoderVorbisSettings : public EncoderSettings {
    public:
    EncoderVorbisSettings(UserSettingsPointer pConfig);
    virtual ~EncoderVorbisSettings();

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
    virtual int getQuality() const override;
    virtual int getQualityIndex() const override;

  private:
    QList<int> m_qualList;
    UserSettingsPointer m_pConfig;
};


inline bool EncoderVorbisSettings::usesQualitySlider() const
{
    return true;
}
inline bool EncoderVorbisSettings::usesCompressionSlider() const
{
    return false;
}
inline bool EncoderVorbisSettings::usesOptionGroups() const
{
    return false;
}

#endif // ENCODERVORBISSETTINGS_H
