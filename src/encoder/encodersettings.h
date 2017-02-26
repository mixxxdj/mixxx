/****************************************************************************
                   encodersettings.h  - encoder API for mixxx
                             -------------------
    copyright            : (C) 2017 by Josep Maria Antolín
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

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
    virtual void setQualityByValue(int qualityValue) {}
    // Sets the quality value by its index
    virtual void setQualityByIndex(int qualityIndex) {}
    virtual int getQuality() const { return 0; }
    virtual int getQualityIndex() const { return 0; }
    // Returns the list of compression values supported, to assign them to the slider
    virtual QList<int> getCompressionValues() const { return QList<int>(); }
    // Sets the compression level
    virtual void setCompression(int compression) {}
    virtual int getCompression() const { return 0; }
    // Returns the list of radio options to show to the user
    virtual QList<OptionsGroup> getOptionGroups() const { return QList<OptionsGroup>(); }
    // Selects the option by its index. If it is a single-element option, 
    // index 0 means disabled and 1 enabled.
    virtual void setGroupOption(QString groupCode, int optionIndex) {}
    // Return the selected option of the group. If it is a single-element option, 
    // 0 means disabled and 1 enabled.
    virtual int getSelectedOption(QString groupCode) const { return 0; }
    
};

typedef std::shared_ptr<EncoderSettings> EncoderSettingsPointer;

#endif // ENCODERSETTINGS_H
