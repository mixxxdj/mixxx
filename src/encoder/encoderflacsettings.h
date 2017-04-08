/**
* @file encoderflacsettings.cpp
* @author Josep Maria Antolín
* @date Feb 27 2017
* @brief storage of setting for flac encoder
*/

#ifndef ENCODERFLACSETTINGS_H
#define ENCODERFLACSETTINGS_H

#include "encoder/encodersettings.h"
#include "encoder/encoder.h"

class EncoderFlacSettings : public EncoderSettings {
  public:
    EncoderFlacSettings(UserSettingsPointer pConfig);
    virtual ~EncoderFlacSettings();

    // Indicates that it uses the quality slider section of the preferences
    bool usesQualitySlider() const override;
    // Indicates that it uses the compression slider section of the preferences
    bool usesCompressionSlider() const override;
    // Indicates that it uses the radio button section of the preferences.
    bool usesOptionGroups() const override;

    // Returns the list of compression values supported, to assign them to the slider
    virtual QList<int> getCompressionValues() const override;
    // Sets the compression level
    virtual void setCompression(int compression) override;
    virtual int getCompression() const override;
    // Returns the list of radio options to show to the user
    QList<OptionsGroup> getOptionGroups() const override;
    // Selects the option by its index. If it is a single-element option, 
    // index 0 means disabled and 1 enabled.
    void setGroupOption(QString groupCode, int optionIndex) override;
    // Return the selected option of the group. If it is a single-element option, 
    // 0 means disabled and 1 enabled.
    int getSelectedOption(QString groupCode) const override;

    static const int DEFAULT_QUALITY_VALUE;
    static const QString BITS_GROUP;
    static const QString GROUP_COMPRESSION;
  private:
    QList<OptionsGroup> m_radioList;
    QList<int> m_qualList;
    UserSettingsPointer m_pConfig;
};


inline bool EncoderFlacSettings::usesQualitySlider() const
{
    return false;
}
inline bool EncoderFlacSettings::usesOptionGroups() const
{
    return true;
}

#endif // ENCODERFLACSETTINGS_H

