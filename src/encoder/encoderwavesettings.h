/**
* @file encoderwavesettings.h
* @author Josep Maria Antolín
* @date Feb 27 2017
* @brief storage of setting for wave/aiff encoder
*/

#ifndef ENCODERWAVESETTINGS_H
#define ENCODERWAVESETTINGS_H

#include "encoder/encodersettings.h"
#include "encoder/encoder.h"
#include <QList>

class EncoderWaveSettings : public EncoderSettings {
  public:
    EncoderWaveSettings(UserSettingsPointer pConfig, Encoder::Format format);
    virtual ~EncoderWaveSettings();

    // Indicates that it uses the quality slider section of the preferences
    bool usesQualitySlider() const override;
    // Indicates that it uses the compression slider section of the preferences
    bool usesCompressionSlider() const override;
    // Indicates that it uses the radio button section of the preferences.
    bool usesOptionGroups() const override;

    // Returns the list of radio options to show to the user
    QList<OptionsGroup> getOptionGroups() const override;
    // Selects the option by its index. If it is a single-element option, 
    // index 0 means disabled and 1 enabled.
    void setGroupOption(QString groupCode, int optionIndex) override;
    // Return the selected option of the group. If it is a single-element option, 
    // 0 means disabled and 1 enabled.
    int getSelectedOption(QString groupCode) const override;

    // Returns the format subtype of this encoder settings.
    Encoder::Format getFormat() const;
    
    static const QString BITS_GROUP;
  private:
    QList<OptionsGroup> m_radioList;
    UserSettingsPointer m_pConfig;
    Encoder::Format m_format;
};


inline bool EncoderWaveSettings::usesQualitySlider() const
{
    return false;
}
inline bool EncoderWaveSettings::usesCompressionSlider() const
{
    return false;
}
inline bool EncoderWaveSettings::usesOptionGroups() const
{
    return true;
}

inline Encoder::Format EncoderWaveSettings::getFormat() const
{
    return m_format;
}

#endif // ENCODERWAVESETTINGS_H
