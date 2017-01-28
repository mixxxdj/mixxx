#ifndef DLGPREFBROADCAST_H
#define DLGPREFBROADCAST_H

#include <QWidget>

#include "preferences/dialog/ui_dlgprefbroadcastdlg.h"
#include "control/controlobject.h"
#include "preferences/usersettings.h"
#include "broadcast/defs_broadcast.h"
#include "preferences/dlgpreferencepage.h"

class ControlProxy;

class DlgPrefBroadcast : public DlgPreferencePage, public Ui::DlgPrefBroadcastDlg  {
    Q_OBJECT
  public:
    DlgPrefBroadcast(QWidget *parent, UserSettingsPointer _config);
    virtual ~DlgPrefBroadcast();

  public slots:
    /** Apply changes to widget */
    void slotApply();
    void slotUpdate();
    void slotResetToDefaults();
    void broadcastEnabledChanged(double value);

  signals:
    void apply(const QString &);

  private:
    UserSettingsPointer m_pConfig;
    ControlProxy* m_pBroadcastEnabled;
};

#endif
