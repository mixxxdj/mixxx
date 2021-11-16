#pragma once

#include <QModelIndex>
#include <QWidget>

#include "broadcast/defs_broadcast.h"
#include "control/controlobject.h"
#include "preferences/broadcastsettings.h"
#include "preferences/broadcastsettingsmodel.h"
#include "preferences/dialog/dlgpreferencepage.h"
#include "preferences/dialog/ui_dlgprefbroadcastdlg.h"
#include "preferences/usersettings.h"

class ControlProxy;

class DlgPrefBroadcast : public DlgPreferencePage, public Ui::DlgPrefBroadcastDlg  {
    Q_OBJECT
  public:
    DlgPrefBroadcast(QWidget *parent,
                     BroadcastSettingsPointer pBroadcastSettings);
    virtual ~DlgPrefBroadcast();

    QUrl helpUrl() const override;

  public slots:
    /** Apply changes to widget */
    void slotApply() override;
    void slotUpdate() override;
    void slotResetToDefaults() override;
    void broadcastEnabledChanged(double value);
    void checkBoxEnableReconnectChanged(int value);
    void checkBoxLimitReconnectsChanged(int value);
    void enableCustomMetadataChanged(int value);
    void connectionListItemSelected(const QModelIndex& selected);

  signals:
    void apply(const QString &);

  private slots:
    void btnCreateConnectionClicked();
    void btnRenameConnectionClicked();
    void btnRemoveConnectionClicked();
    void btnDisconnectAllClicked();
    void onSectionResized();

  private:
    void applyModel();
    void updateModel();
    void selectConnectionRow(int row);
    void selectConnectionRowByName(const QString& rowName);
    void getValuesFromProfile(BroadcastProfilePtr profile);
    void setValuesToProfile(BroadcastProfilePtr profile);

    BroadcastSettingsPointer m_pBroadcastSettings;
    BroadcastSettingsModel* m_pSettingsModel;
    ControlProxy* m_pBroadcastEnabled;
    BroadcastProfilePtr m_pProfileListSelection;
};
