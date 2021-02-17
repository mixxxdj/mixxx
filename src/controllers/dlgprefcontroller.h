#pragma once

#include <QHash>
#include <QSortFilterProxyModel>
#include <memory>

#include "controllers/controllerinputmappingtablemodel.h"
#include "controllers/controllermappinginfo.h"
#include "controllers/controlleroutputmappingtablemodel.h"
#include "controllers/dlgcontrollerlearning.h"
#include "controllers/legacycontrollermapping.h"
#include "controllers/ui_dlgprefcontrollerdlg.h"
#include "preferences/dialog/dlgpreferencepage.h"
#include "preferences/usersettings.h"

// Forward declarations
class Controller;
class ControllerManager;
class MappingInfoEnumerator;

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

  public slots:
    /// Called when the preference dialog (not this page) is shown to the user.
    void slotUpdate() override;
    /// Called when the user clicks the global "Apply" button.
    void slotApply() override;
    /// Called when the user clicks the global "Reset to Defaults" button.
    void slotResetToDefaults() override;

  signals:
    void applyMapping(Controller* pController,
            LegacyControllerMappingPointer pMapping,
            bool bEnabled);
    void mappingStarted();
    void mappingEnded();

  private slots:
    /// Called when the user selects another mapping in the combobox
    void slotMappingSelected(int index);
    /// Used to selected the current mapping in the combobox and display the
    /// mapping information.
    void slotShowMapping(LegacyControllerMappingPointer mapping);

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
    QString mappingShortName(const LegacyControllerMappingPointer pMapping) const;
    QString mappingName(const LegacyControllerMappingPointer pMapping) const;
    QString mappingAuthor(const LegacyControllerMappingPointer pMapping) const;
    QString mappingDescription(const LegacyControllerMappingPointer pMapping) const;
    QString mappingSupportLinks(const LegacyControllerMappingPointer pMapping) const;
    QString mappingFileLinks(const LegacyControllerMappingPointer pMapping) const;
    void applyMappingChanges();
    void saveMapping();
    void initTableView(QTableView* pTable);

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
    bool isDirty() {
        return m_bDirty;
    }

    /// Reload the mappings in the dropdown dialog
    void enumerateMappings(const QString& selectedMappingPath);
    MappingInfo enumerateMappingsFromEnumerator(
            QSharedPointer<MappingInfoEnumerator> pMappingEnumerator,
            const QIcon& icon = QIcon());

    void enableDevice();
    void disableDevice();

    Ui::DlgPrefControllerDlg m_ui;
    UserSettingsPointer m_pConfig;
    const QString m_pUserDir;
    std::shared_ptr<ControllerManager> m_pControllerManager;
    Controller* m_pController;
    DlgControllerLearning* m_pDlgControllerLearning;
    LegacyControllerMappingPointer m_pMapping;
    QMap<QString, bool> m_pOverwriteMappings;
    ControllerInputMappingTableModel* m_pInputTableModel;
    QSortFilterProxyModel* m_pInputProxyModel;
    ControllerOutputMappingTableModel* m_pOutputTableModel;
    QSortFilterProxyModel* m_pOutputProxyModel;
    bool m_bDirty;
};
