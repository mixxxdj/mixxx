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
    void slotUpdate() override;
    void slotApply() override;
    void slotResetToDefaults() override;
    void slotCancel() override;

  private slots:
    void slotSetMinimumAvailable(int);
    void slotToggleRequeueIgnore(int);
    void slotSetRequeueIgnoreTime(const QTime& a_rTime);
    void slotSetRandomQueueMin(int);
    void slotConsiderRepeatPlaylistState(int);
    void slotToggleRandomQueue(int);

  private:
    UserSettingsPointer m_pConfig;
};

#endif /* DLGPREFAUTODJ_H */
