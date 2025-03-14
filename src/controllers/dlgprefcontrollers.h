#pragma once

#include <memory>

#include "controllers/ui_dlgprefcontrollersdlg.h"
#include "preferences/dialog/dlgpreferencepage.h"
#include "preferences/usersettings.h"
#include "util/parented_ptr.h"

class ControlProxy;
class DlgPreferences;
class DlgPrefController;
class ControllerManager;
class QTreeWidgetItem;

/// Controllers Overview in the preferences
///
/// This dialog allows selecting controllers for configuration.

class DlgPrefControllers : public DlgPreferencePage, public Ui::DlgPrefControllersDlg {
    Q_OBJECT
  public:
    DlgPrefControllers(DlgPreferences* pDlgPreferences,
            UserSettingsPointer pConfig,
            std::shared_ptr<ControllerManager> pControllerManager,
            QTreeWidgetItem* pControllersRootItem);
    virtual ~DlgPrefControllers();

    bool handleTreeItemClick(QTreeWidgetItem* clickedItem);
    QUrl helpUrl() const override;

    void updateMappingIconsAndColoredLinkTexts();
    void updateColoredLinkTexts() override;

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
#ifdef __PORTMIDI__
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
    void slotMidiThroughChanged(Qt::CheckState state);
#else
    void slotMidiThroughChanged(bool checked);
#endif
#endif
    void slotHighlightDevice(DlgPrefController* dialog, bool enabled);

  private:
    void destroyControllerWidgets();
    void setupControllerWidgets();
    void openLocalFile(const QString& file);

    DlgPreferences* m_pDlgPreferences;
    UserSettingsPointer m_pConfig;
    std::shared_ptr<ControllerManager> m_pControllerManager;
    QTreeWidgetItem* m_pControllersRootItem;
    QList<DlgPrefController*> m_controllerPages;
    QList<QTreeWidgetItem*> m_controllerTreeItems;

    const parented_ptr<ControlProxy> m_pNumDecks;
    const parented_ptr<ControlProxy> m_pNumSamplers;
};
