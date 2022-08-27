#pragma once

#include <QWidget>

#include "control/pollingcontrolproxy.h"
#include "preferences/dialog/dlgpreferencepage.h"
#include "preferences/dialog/ui_dlgprefcrossfaderdlg.h"
#include "preferences/usersettings.h"

class DlgPrefCrossfader : public DlgPreferencePage, public Ui::DlgPrefCrossfaderDlg  {
    Q_OBJECT
  public:
    DlgPrefCrossfader(QWidget* parent, UserSettingsPointer _config);
    virtual ~DlgPrefCrossfader();

  public slots:
    // Update X-Fader
    void slotUpdateXFader();
    // Apply changes to widget
    void slotApply();
    void slotUpdate();
    void slotResetToDefaults();

  signals:
    void apply(const QString &);

  private:
    void loadSettings();
    void drawXfaderDisplay();

    // Pointer to config object
    UserSettingsPointer m_config;

    QGraphicsScene* m_pxfScene;

    // X-fader values
    double m_xFaderMode, m_transform, m_cal;

    PollingControlProxy m_mode;
    PollingControlProxy m_curve;
    PollingControlProxy m_calibration;
    PollingControlProxy m_reverse;
    PollingControlProxy m_crossfader;

    bool m_xFaderReverse;
};
