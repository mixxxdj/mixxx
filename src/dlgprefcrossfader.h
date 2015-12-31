#ifndef DLGPREFCROSSFADER_H
#define DLGPREFCROSSFADER_H

#include <QWidget>

#include "ui_dlgprefcrossfaderdlg.h"
#include "configobject.h"
#include "controlobjectslave.h"
#include "preferences/dlgpreferencepage.h"


class DlgPrefCrossfader : public DlgPreferencePage, public Ui::DlgPrefCrossfaderDlg  {
    Q_OBJECT
  public:
    DlgPrefCrossfader(QWidget* parent, ConfigObject<ConfigValue>* _config);
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
    ConfigObject<ConfigValue>* m_config;

    QGraphicsScene* m_pxfScene;

    // X-fader values
    double m_xFaderMode, m_transform, m_cal;

    ControlObjectSlave m_mode;
    ControlObjectSlave m_curve;
    ControlObjectSlave m_calibration;
    ControlObjectSlave m_reverse;
    ControlObjectSlave m_crossfader;

    bool m_xFaderReverse;
};

#endif
