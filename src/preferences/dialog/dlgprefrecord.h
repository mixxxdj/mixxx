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
    void sampleRateChanged(int newRateIdx);
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
    void slotToggleCustomSampleRateIgnore(Qt::CheckState buttonState);
#else
    void slotToggleCustomSampleRateIgnore(int buttonState);
#endif

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
};
