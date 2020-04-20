#pragma once

#include <QTreeWidgetItem>

#include "preferences/usersettings.h"
#include "controllers/ui_dlgprefcontrollersdlg.h"
#include "preferences/dlgpreferencepage.h"

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
                       ControllerManager* pControllerManager,
                       QTreeWidgetItem* pControllerTreeItem);
    virtual ~DlgPrefControllers();

    bool handleTreeItemClick(QTreeWidgetItem* clickedItem);

  public slots:
    /// Called when the preference dialog (not this page) is shown to the user.
    void slotUpdate();
    /// Called when the user clicks the global "Apply" button.
    void slotApply();
    /// Called when the user clicks the global "Cancel" button.
    void slotCancel();
    /// Called when the user clicks the global "Reset to Defaults" button.
    void slotResetToDefaults();

  private slots:
    void rescanControllers();
    void slotHighlightDevice(DlgPrefController* dialog, bool enabled);
    void slotOpenLocalFile(const QString& file);

  private:
    void destroyControllerWidgets();
    void setupControllerWidgets();

    DlgPreferences* m_pDlgPreferences;
    UserSettingsPointer m_pConfig;
    ControllerManager* m_pControllerManager;
    QTreeWidgetItem* m_pControllerTreeItem;
    QList<DlgPrefController*> m_controllerWindows;
    QList<QTreeWidgetItem*> m_controllerTreeItems;
};
