#ifndef DLGPREFBROADCAST_H
#define DLGPREFBROADCAST_H

#include <QWidget>
#include <QGroupBox>

#include "preferences/dialog/ui_dlgprefbroadcastdlg.h"
#include "control/controlobject.h"
#include "preferences/usersettings.h"
#include "broadcast/defs_broadcast.h"
#include "preferences/dlgpreferencepage.h"
#include "preferences/broadcastsettings.h"

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
    void checkBoxEnableReconnectChanged(int value);
    void checkBoxLimitReconnectsChanged(int value);
    void enableCustomMetadataChanged(int value);
    void btnCreateProfileClicked(bool checked);
    void profileListItemSelected(const QModelIndex& index);

  signals:
    void apply(const QString &);

  private slots:
    void formValueChanged();

  private:
    BroadcastProfile* selectedProfile();
    void getValuesFromProfile(BroadcastProfile* profile);
    void setValuesToProfile(BroadcastProfile* profile);
    void enableValueSignals(bool enable = true);

    BroadcastSettings m_settings;
    ControlProxy* m_pBroadcastEnabled;
    BroadcastProfile* m_pProfileListSelection;
    bool m_valuesChanged;
};

#endif
