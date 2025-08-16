#pragma once

#include <memory>

#include "controllers/controllermappinginfo.h"
#include "controllers/midi/midimessage.h"
#include "controllers/ui_dlgprefcontrollerdlg.h"
#include "preferences/dialog/dlgpreferencepage.h"
#include "preferences/usersettings.h"
#include "util/parented_ptr.h"

// Forward declarations
class Controller;
class ControllerInputMappingTableModel;
class ControllerMappingTableProxyModel;
class ControllerManager;
class ControllerOutputMappingTableModel;
class ControlPickerMenu;
class DlgControllerLearning;
class MappingInfoEnumerator;
#ifdef MIXXX_USE_QML
class ControllerScriptEngineLegacy;
#endif

/// Configuration dialog for a single DJ controller
class DlgPrefController : public DlgPreferencePage {
    Q_OBJECT
  public:
    DlgPrefController(QWidget* parent,
            Controller* controller,
            std::shared_ptr<ControllerManager> controllerManager,
            UserSettingsPointer pConfig);
    virtual ~DlgPrefController();

    QUrl helpUrl() const override;
    void keyPressEvent(QKeyEvent* pEvent) override;

  public slots:
    /// Called when the preference dialog (not this page) is shown to the user.
    void slotUpdate() override;
    /// Called when the user clicks the global "Apply" button.
    void slotApply() override;
    /// Called when the preferences are hidden, e.g. when closing the window
    /// with the [X] button or keyboard shortcut
    void slotHide() override;
    /// Called when the user clicks the global "Reset to Defaults" button.
    void slotResetToDefaults() override;

    void slotRecreateControlPickerMenu();

  signals:
    void applyMapping(Controller* pController,
            std::shared_ptr<LegacyControllerMapping> pMapping,
            bool bEnabled);
    void mappingStarted();
    void mappingEnded();

  private slots:
    /// Called when the user selects another mapping in the combobox
    void slotMappingSelected(int index);
    void slotInputControlSearch();
    void slotOutputControlSearch();
    /// Called when the Controller Learning Wizard is closed.
    void slotStopLearning();
    void enableWizardAndIOTabs(bool enable);

#ifdef MIXXX_USE_QML
    // Onboard screen controller.
    void slotShowPreviewScreens(const ControllerScriptEngineLegacy* scriptEngine);
    // Wrapper used on shutdown.
    void slotClearPreviewScreens() {
        slotShowPreviewScreens(nullptr);
    }
#endif

    // Input mappings
    void addInputMapping();
    void showLearningWizard();
    void removeInputMappings();
    void clearAllInputMappings();

    // Output mappings
    void addOutputMapping();
    void removeOutputMappings();
    void clearAllOutputMappings();

    void midiInputMappingsLearned(const MidiInputMappings& mappings);

  private:
    /// Used to selected the current mapping in the combobox and display the
    /// mapping information.
    void showMapping(std::shared_ptr<LegacyControllerMapping> mapping);
    QString mappingShortName(const std::shared_ptr<LegacyControllerMapping> pMapping) const;
    QString mappingSupportLinks(const std::shared_ptr<LegacyControllerMapping> pMapping) const;
    QString mappingFileLinks(const std::shared_ptr<LegacyControllerMapping> pMapping) const;
    QString mappingFilePathFromIndex(int index) const;
    QString askForMappingName(const QString& prefilledName = QString()) const;
    void applyMappingChanges();
    bool saveMapping();
    void initTableView(QTableView* pTable);
    unsigned int getNumberOfVisibleTabs();
    int getIndexOfFirstVisibleTab();

    /// Set dirty state (i.e. changes have been made).
    ///
    /// When this preferences page is marked as "dirty", changes have occurred
    /// that can be applied or discarded.
    ///
    /// @param bDirty The new dialog's dirty state.
    void setDirty(bool bDirty) {
        m_bDirty = bDirty;
    }

    /// Set dirty state (i.e. changes have been made).
    ///
    /// When this preferences page is marked as "dirty", changes have occurred
    /// that can be applied or discarded.
    ///
    /// @param bDirty The new dialog's dirty state.
    bool isDirty() const {
        return m_bDirty;
    }

    /// Reload the mappings in the dropdown dialog
    void enumerateMappings(const QString& selectedMappingPath);
    MappingInfo enumerateMappingsFromEnumerator(
            QSharedPointer<MappingInfoEnumerator> pMappingEnumerator,
            const QIcon& icon = QIcon());

    Ui::DlgPrefControllerDlg m_ui;
    UserSettingsPointer m_pConfig;
    const QString m_pUserDir;
    std::shared_ptr<ControllerManager> m_pControllerManager;
    Controller* m_pController;
    parented_ptr<ControlPickerMenu> m_pControlPickerMenu;
    DlgControllerLearning* m_pDlgControllerLearning;
    std::shared_ptr<LegacyControllerMapping> m_pMapping;
    QMap<QString, bool> m_pOverwriteMappings;
    ControllerInputMappingTableModel* m_pInputTableModel;
    ControllerMappingTableProxyModel* m_pInputProxyModel;
    ControllerOutputMappingTableModel* m_pOutputTableModel;
    ControllerMappingTableProxyModel* m_pOutputProxyModel;
    bool m_GuiInitialized;
    bool m_bDirty;
    int m_inputMappingsTabIndex;  // Index of the input mappings tab
    int m_outputMappingsTabIndex; // Index of the output mappings tab
    int m_settingsTabIndex;       // Index of the settings tab
    int m_screensTabIndex;        // Index of the screens tab
    QHash<QString, bool> m_settingsCollapsedStates;
};
