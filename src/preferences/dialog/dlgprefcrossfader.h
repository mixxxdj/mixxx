#pragma once

#include <QWidget>

#include "control/controlproxy.h"
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

    ControlProxy m_mode;
    ControlProxy m_curve;
    ControlProxy m_calibration;
    ControlProxy m_reverse;
    ControlProxy m_crossfader;

    bool m_xFaderReverse;
};
