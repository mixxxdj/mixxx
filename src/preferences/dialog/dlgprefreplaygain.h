#ifndef DLGPREFREPLAYGAIN_H
#define DLGPREFREPLAYGAIN_H

#include <QWidget>

#include "preferences/dialog/ui_dlgprefreplaygaindlg.h"
#include "preferences/usersettings.h"
#include "controlobjectslave.h"
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
    void slotSetRGAnalyzerEnabled();

    void slotApply();
    void slotUpdate();
    void slotResetToDefaults();

  signals:
    void apply(const QString &);

  private:
    // Determines whether or not to gray out the preferences
    void loadSettings();
    void setLabelCurrentReplayGainBoost(int value);

    // Pointer to config object
    UserSettingsPointer config;

    ControlObjectSlave m_replayGainBoost;
    ControlObjectSlave m_defaultBoost;
    ControlObjectSlave m_enabled;
};

#endif /* DLGPREFREPLAYGAIN_H */
