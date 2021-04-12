#pragma once

#include <QList>

#include "encoder/encoder.h"
#include "encoder/encodersettings.h"

class EncoderFdkAacSettings : public EncoderRecordingSettings {
  public:
    EncoderFdkAacSettings(UserSettingsPointer pConfig, QString format);
    virtual ~EncoderFdkAacSettings();

    // Indicates that it uses the quality slider section of the preferences
    bool usesQualitySlider() const override {
        return true;
    }

    // Returns the list of quality values that it supports, to assign them to the slider
    QList<int> getQualityValues() const override;

    void setQualityByIndex(int qualityIndex) override;

    // Returns the current quality value
    int getQuality() const override;
    int getQualityIndex() const override;

    // Returns the format of this encoder settings.
    QString getFormat() const override {
        return m_format;
    }

  private:
    QList<int> m_qualList;
    UserSettingsPointer m_pConfig;
    QString m_format;
};
