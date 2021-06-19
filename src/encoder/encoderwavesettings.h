#pragma once

#include "encoder/encoderrecordingsettings.h"
#include "encoder/encoder.h"
#include <QList>

/// Storage of settings for WAVE/AIFF encoder
class EncoderWaveSettings : public EncoderRecordingSettings {
  public:
    EncoderWaveSettings(UserSettingsPointer pConfig, QString format);
    ~EncoderWaveSettings() override = default;

    /// Returns the list of radio options to show to the user
    QList<OptionsGroup> getOptionGroups() const override;
    /// Selects the option by its index. If it is a single-element option,
    /// index 0 means disabled and 1 enabled.
    void setGroupOption(const QString& groupCode, int optionIndex) override;
    /// Return the selected option of the group. If it is a single-element option,
    /// 0 means disabled and 1 enabled.
    int getSelectedOption(const QString& groupCode) const override;

    /// Returns the format subtype of this encoder settings.
    QString getFormat() const override{
        return m_format;
    }

    static const QString BITS_GROUP;
  private:
    QList<OptionsGroup> m_radioList;
    UserSettingsPointer m_pConfig;
    QString m_format;
};
