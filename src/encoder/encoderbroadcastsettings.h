/****************************************************************************
                   encoder.h  - encoder API for mixxx
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
