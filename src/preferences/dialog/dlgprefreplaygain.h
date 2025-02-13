#pragma once

#include <QButtonGroup>

#include "control/pollingcontrolproxy.h"
#include "preferences/dialog/dlgpreferencepage.h"
#include "preferences/dialog/ui_dlgprefreplaygaindlg.h"
#include "preferences/replaygainsettings.h"

class QWidget;

class DlgPrefReplayGain: public DlgPreferencePage,
                         public Ui::DlgPrefReplayGainDlg {
    Q_OBJECT
  public:
    DlgPrefReplayGain(QWidget *parent, UserSettingsPointer _config);
    virtual ~DlgPrefReplayGain();

  public slots:
    // Update initial gain increment
    void slotUpdateReplayGainBoost();
    void slotUpdateDefaultBoost();
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
    void slotSetRGEnabled(Qt::CheckState);
#else
    void slotSetRGEnabled(int);
#endif
    void slotSetRGAnalyzerChanged();
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
    void slotSetReanalyze(Qt::CheckState);
#else
    void slotSetReanalyze(int);
#endif

    void slotApply() override;
    void slotUpdate() override;
    void slotResetToDefaults() override;

  signals:
    void apply(const QString &);

  private:
    // Determines whether or not to gray out the preferences
    void loadSettings();
    void setLabelCurrentReplayGainBoost(int value);
    bool isReplayGainAnalyzerEnabled() const;
    int getReplayGainVersion() const;

    ReplayGainSettings m_rgSettings;
    PollingControlProxy m_replayGainBoost;
    PollingControlProxy m_defaultBoost;
    PollingControlProxy m_enabled;
};
