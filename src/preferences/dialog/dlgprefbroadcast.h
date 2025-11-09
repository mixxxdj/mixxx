#pragma once

#include "preferences/broadcastsettings.h"
#include "preferences/dialog/dlgpreferencepage.h"
#include "preferences/dialog/ui_dlgprefbroadcastdlg.h"

class ControlProxy;
class QWidget;
class QModelIndex;

class DlgPrefBroadcast : public DlgPreferencePage, public Ui::DlgPrefBroadcastDlg  {
    Q_OBJECT
  public:
    DlgPrefBroadcast(QWidget *parent,
                     BroadcastSettingsPointer pBroadcastSettings);
    virtual ~DlgPrefBroadcast();

    /// False if at least one connection has passwords with invalid
    /// characters or two Icecast connections have identical mount point
    bool okayToClose() const override;

    QUrl helpUrl() const override;

  public slots:
    /** Apply changes to widget */
    void slotApply() override;
    void slotUpdate() override;
    void slotResetToDefaults() override;
    void broadcastEnabledChanged(double value);
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
    void checkBoxEnableReconnectChanged(Qt::CheckState state);
    void checkBoxLimitReconnectsChanged(Qt::CheckState state);
    void enableCustomMetadataChanged(Qt::CheckState state);
#else
    void checkBoxEnableReconnectChanged(int value);
    void checkBoxLimitReconnectsChanged(int value);
    void enableCustomMetadataChanged(int value);
#endif
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
    bool m_allProfilesValid;
};
