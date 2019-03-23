/**
* @file encodermp3settings.cpp
* @author Josep Maria Antolín
* @date Feb 27 2017
* @brief storage of setting for mp3 encoder
*/

#ifndef ENCODERMP3SETTINGS_H
#define ENCODERMP3SETTINGS_H

#include "encoder/encodersettings.h"
#include "encoder/encoder.h"

class EncoderMp3Settings : public EncoderSettings {
  public:
    EncoderMp3Settings(UserSettingsPointer m_pConfig);
    virtual ~EncoderMp3Settings();

    // Indicates that it uses the quality slider section of the preferences
    bool usesQualitySlider() const override;
    // Indicates that it uses the compression slider section of the preferences
    bool usesCompressionSlider() const override;
    // Indicates that it uses the radio button section of the preferences.
    bool usesOptionGroups() const override;

    // Returns the list of quality values that it supports, to assign them to the slider
    QList<int> getQualityValues() const override;
    QList<int> getVBRQualityValues() const;
    // Sets the quality value by its value
    void setQualityByValue(int qualityValue) override;
    // Sets the quality value by its index
    void setQualityByIndex(int qualityIndex) override;
    // Returns the current quality value
    int getQuality() const override;
    int getQualityIndex() const override;
    // Returns the list of radio options to show to the user
    QList<OptionsGroup> getOptionGroups() const override;
    // Selects the option by its index. If it is a single-element option, 
    // index 0 means disabled and 1 enabled.
    void setGroupOption(QString groupCode, int optionIndex) override;
    // Return the selected option of the group. If it is a single-element option, 
    // 0 means disabled and 1 enabled.
    int getSelectedOption(QString groupCode) const override;

    static const int DEFAULT_BITRATE_INDEX;
    static const QString ENCODING_MODE_GROUP;
  private:
    QList<OptionsGroup> m_radioList;
    QList<int> m_qualList;
    QList<int> m_qualVBRList;
    UserSettingsPointer m_pConfig;
};


inline bool EncoderMp3Settings::usesQualitySlider() const
{
    return true;
}
inline bool EncoderMp3Settings::usesCompressionSlider() const
{
    return false;
}
inline bool EncoderMp3Settings::usesOptionGroups() const
{
    return true;
}

#endif // ENCODERMP3SETTINGS_H
