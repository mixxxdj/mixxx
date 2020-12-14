#pragma once

#include <QTreeWidgetItem>
#include <memory>

#include "controllers/ui_dlgprefcontrollersdlg.h"
#include "preferences/dlgpreferencepage.h"
#include "preferences/usersettings.h"

class DlgPreferences;
class DlgPrefController;
class ControllerManager;

/// Controllers Overview in the preferences
///
/// This dialog allows selecting controllers for configuration.

class DlgPrefControllers : public DlgPreferencePage, public Ui::DlgPrefControllersDlg {
    Q_OBJECT
  public:
    DlgPrefControllers(DlgPreferences* pDlgPreferences,
            UserSettingsPointer pConfig,
            std::shared_ptr<ControllerManager> pControllerManager,
            QTreeWidgetItem* pControllerTreeItem);
    virtual ~DlgPrefControllers();

    bool handleTreeItemClick(QTreeWidgetItem* clickedItem);
    QUrl helpUrl() const override;

  public slots:
    /// Called when the preference dialog (not this page) is shown to the user.
    void slotUpdate() override;
    /// Called when the user clicks the global "Apply" button.
    void slotApply() override;
    /// Called when the user clicks the global "Cancel" button.
    void slotCancel() override;
    /// Called when the user clicks the global "Reset to Defaults" button.
    void slotResetToDefaults() override;

  private slots:
    void rescanControllers();
    void slotHighlightDevice(DlgPrefController* dialog, bool enabled);

  private:
    void destroyControllerWidgets();
    void setupControllerWidgets();
    void openLocalFile(const QString& file);

    DlgPreferences* m_pDlgPreferences;
    UserSettingsPointer m_pConfig;
    std::shared_ptr<ControllerManager> m_pControllerManager;
    QTreeWidgetItem* m_pControllerTreeItem;
    QList<DlgPrefController*> m_controllerWindows;
    QList<QTreeWidgetItem*> m_controllerTreeItems;
};
