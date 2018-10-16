/**
* @file encodersettings.cpp
* @author Josep Maria Antolín
* @date Feb 27 2017
* @brief Encoder settings interface for encoders.
*/

#ifndef ENCODERSETTINGS_H
#define ENCODERSETTINGS_H

#include <QList>
#include <QObject>
#include "util/memory.h"

class EncoderSettings {
    public:
        class OptionsGroup {
            public:
            OptionsGroup(QString gName, QString gCode, QList<QString> conNames) :
                groupName(gName), groupCode(gCode), controlNames(conNames) {}
            QString groupName;
            QString groupCode;
            QList<QString> controlNames;
        };
        enum class ChannelMode {
            AUTOMATIC=0,
            MONO=1,
            STEREO=2
        };

    EncoderSettings() {}
    virtual ~EncoderSettings() {}

    // Indicates that it uses the quality slider section of the preferences
    virtual bool usesQualitySlider() const = 0;
    // Indicates that it uses the compression slider section of the preferences
    virtual bool usesCompressionSlider() const = 0;
    // Indicates that it uses the radio button section of the preferences.
    virtual bool usesOptionGroups() const = 0;

    // Returns the list of quality values supported, to assign them to the slider
    virtual QList<int> getQualityValues() const { return QList<int>(); }
    // Sets the quality value by its value
    virtual void setQualityByValue(int qualityValue) { Q_UNUSED(qualityValue); }
    // Sets the quality value by its index
    virtual void setQualityByIndex(int qualityIndex) { Q_UNUSED(qualityIndex); }
    virtual int getQuality() const { return 0; }
    virtual int getQualityIndex() const { return 0; }
    // Returns the list of compression values supported, to assign them to the slider
    virtual QList<int> getCompressionValues() const { return QList<int>(); }
    // Sets the compression level
    virtual void setCompression(int compression) {  Q_UNUSED(compression); }
    virtual int getCompression() const { return 0; }
    // Returns the list of radio options to show to the user
    virtual QList<OptionsGroup> getOptionGroups() const { return QList<OptionsGroup>(); }
    // Selects the option by its index. If it is a single-element option, 
    // index 0 means disabled and 1 enabled.
    virtual void setGroupOption(QString groupCode, int optionIndex) { 
            Q_UNUSED(groupCode); Q_UNUSED(optionIndex); }
    // Return the selected option of the group. If it is a single-element option, 
    // 0 means disabled and 1 enabled.
    virtual int getSelectedOption(QString groupCode) const { Q_UNUSED(groupCode); return 0; }
    
    virtual void setChannelMode(ChannelMode mode) { Q_UNUSED(mode); }
    virtual ChannelMode getChannelMode() const { return ChannelMode::AUTOMATIC; }
};

typedef std::shared_ptr<EncoderSettings> EncoderSettingsPointer;

#endif // ENCODERSETTINGS_H
