#pragma once

#include <QButtonGroup>

#include "control/pollingcontrolproxy.h"
#include "encoder/encoder.h"
#include "preferences/dialog/dlgpreferencepage.h"
#include "preferences/dialog/ui_dlgprefrecorddlg.h"
#include "preferences/usersettings.h"

class ControlObject;
class ControlProxy;
class QRadioButton;
class QWidget;

class DlgPrefRecord : public DlgPreferencePage, public Ui::DlgPrefRecordDlg  {
    Q_OBJECT
  public:
    DlgPrefRecord(QWidget *parent, UserSettingsPointer _config);
    virtual ~DlgPrefRecord();

  protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

  public slots:
    // Apply changes to widget
    void slotApply() override;
    void slotUpdate() override;
    void slotResetToDefaults() override;

    // Dialog to browse for recordings directory
    void slotBrowseRecordingsDir();

    void slotFormatChanged();
    void slotSliderQuality();
    void slotSliderCompression();
    void slotGroupChanged();

    void slotDefaultSampleRateUpdated(mixxx::audio::SampleRate newRate);

  private slots:
    void slotToggleCueEnabled();
    void slotSampleRateChanged(int newRateIdx);
    void slotToggleCustomSampleRateIgnore(int buttonState);

  signals:
    void apply(const QString &);

  private:
    void setupEncoderUI();
    void loadMetaData();
    void updateTextQuality();
    void updateTextCompression();
    void saveRecordingFolder();
    void saveMetaData();
    void saveEncoding();
    void saveUseCueFile();
    void saveUseCueFileAnnotation();
    void saveSplitSize();
    void saveRecSampleRate();
    void updateSampleRates(const QList<mixxx::audio::SampleRate>& sampleRates);
    const QList<mixxx::audio::SampleRate>& createSampleRateGUIForFormat(const QString& recFormat);

    // Pointer to config object
    UserSettingsPointer m_pConfig;
    Encoder::Format m_selFormat;
    QButtonGroup encodersgroup;
    QButtonGroup optionsgroup;
    QList<QRadioButton*> m_formatButtons;
    QList<QAbstractButton*> m_optionWidgets;

    // to access the recsamplerate object.
    PollingControlProxy m_recSampleRate;
    PollingControlProxy m_useEngineSampleRate;

    double m_defaultSampleRate; // tracks the engine sample rate
    double m_oldRecSampleRate;
};
