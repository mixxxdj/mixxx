// encoderfdkaacsettings.h
// Created on Aug 15 2017 by Palakis

#ifndef ENCODER_ENCODERFDKAACSETTINGS_H
#define ENCODER_ENCODERFDKAACSETTINGS_H

#include <QList>

#include "encoder/encodersettings.h"
#include "encoder/encoder.h"

class EncoderFdkAacSettings: public EncoderSettings {
  public:
    EncoderFdkAacSettings(UserSettingsPointer pConfig);
    virtual ~EncoderFdkAacSettings();

    // Indicates that it uses the quality slider section of the preferences
    bool usesQualitySlider() const override {
        return true;
    }
    // Indicates that it uses the compression slider section of the preferences
    bool usesCompressionSlider() const override {
        return false;
    }
    // Indicates that it uses the radio button section of the preferences.
    bool usesOptionGroups() const override {
        return false;
    }

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
    UserSettingsPointer m_pConfig;
};

#endif // ENCODER_ENCODERFDKAACSETTINGS_H
