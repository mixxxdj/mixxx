#ifndef DLGPREFREPLAYGAIN_H
#define DLGPREFREPLAYGAIN_H

#include <QButtonGroup>
#include <QWidget>

#include "preferences/dialog/ui_dlgprefreplaygaindlg.h"
#include "preferences/replaygainsettings.h"
#include "control/controlproxy.h"
#include "preferences/dlgpreferencepage.h"

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
    void slotSetRGEnabled();
    void slotSetRGAnalyzerChanged();
    void slotSetReanalyze();

    void slotApply();
    void slotUpdate();
    void slotResetToDefaults();

  signals:
    void apply(const QString &);

  private:
    // Determines whether or not to gray out the preferences
    void loadSettings();
    void setLabelCurrentReplayGainBoost(int value);
    bool isReplayGainAnalyzerEnabled() const;
    int getReplayGainVersion() const;

    ReplayGainSettings m_rgSettings;
    ControlProxy m_replayGainBoost;
    ControlProxy m_defaultBoost;
    ControlProxy m_enabled;

    QButtonGroup m_analysisButtonGroup;
};

#endif /* DLGPREFREPLAYGAIN_H */
