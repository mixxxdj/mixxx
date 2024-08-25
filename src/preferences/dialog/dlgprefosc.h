#ifndef DLGPREFOSC_H
#define DLGPREFOSC_H

#include <QRadioButton>
#include <QWidget>

#include "preferences/usersettings.h"
//#include "preferences/dialog/ui_dlgprefoscdlg.h"
//#include "preferences/dialog/dlgprefoscdlg.ui"
//#include "preferences/dlgpreferencespage.h"
#include "preferences/dlgpreferences.h"

class ControlObject;
class ControlObjectThread;

class DlgPrefOsc : public DlgPreferencePage, public Ui::DlgPrefOscDlg  {
    Q_OBJECT
  public:
    DlgPrefOsc(QWidget *parent, UserSettingsPointer& pConfig);
    virtual ~DlgPrefOsc();

  public slots:
    // Apply changes to widget
    void slotApply();
    void slotUpdate();
    void slotResetToDefaults();



  signals:
    void apply(const QString &);

  private:
    // Pointer to config object
    UserSettingsPointer m_pConfig;
};

#endif