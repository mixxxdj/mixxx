#ifndef DLGPREFAUTODJ_H
#define DLGPREFAUTODJ_H

#include <QWidget>

#include "preferences/dialog/ui_dlgprefautodjdlg.h"
#include "preferences/usersettings.h"
#include "preferences/dlgpreferencepage.h"

class DlgPrefAutoDJ : public DlgPreferencePage, public Ui::DlgPrefAutoDJDlg {
    Q_OBJECT
  public:
    DlgPrefAutoDJ(QWidget* pParent, UserSettingsPointer pConfig);
    virtual ~DlgPrefAutoDJ();

  public slots:
    void slotUpdate();
    void slotApply();
    void slotResetToDefaults();
    void slotCancel() ;

  private slots:
    void slotSetAutoDjRequeue(int);
    void slotSetAutoDjMinimumAvailable(int);
    void slotSetAutoDjUseIgnoreTime(int);
    void slotSetAutoDjIgnoreTime(const QTime &a_rTime);
    void slotSetAutoDJRandomQueueMin(int);
    void slotEnableAutoDJRandomQueueComboBox(int);
    void slotEnableAutoDJRandomQueue(int);

  private:
    UserSettingsPointer m_pConfig;
};

#endif /* DLGPREFAUTODJ_H */
