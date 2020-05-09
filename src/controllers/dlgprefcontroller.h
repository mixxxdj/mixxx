/// @file dlgprefcontroller.h
/// @author Sean M. Pappalardo  spappalardo@mixxx.org
/// @date Mon May 2 2011
/// @brief Configuration dialog for a single DJ controller

#ifndef DLGPREFCONTROLLER_H
#define DLGPREFCONTROLLER_H

#include <QHash>
#include <QSortFilterProxyModel>

#include "controllers/controllerinputmappingtablemodel.h"
#include "controllers/controlleroutputmappingtablemodel.h"
#include "controllers/controllerpreset.h"
#include "controllers/controllerpresetinfo.h"
#include "controllers/dlgcontrollerlearning.h"
#include "controllers/ui_dlgprefcontrollerdlg.h"
#include "preferences/usersettings.h"
#include "preferences/dlgpreferencepage.h"

// Forward declarations
class Controller;
class ControllerManager;
class PresetInfoEnumerator;

class DlgPrefController : public DlgPreferencePage {
    Q_OBJECT
  public:
    DlgPrefController(QWidget *parent, Controller* controller,
                      ControllerManager* controllerManager,
                      UserSettingsPointer pConfig);
    virtual ~DlgPrefController();

  public slots:
    /// Called when the preference dialog (not this page) is shown to the user.
    void slotUpdate();
    /// Called when the user clicks the global "Apply" button.
    void slotApply();
    /// Called when the user clicks the global "Reset to Defaults" button.
    void slotResetToDefaults();

    QUrl helpUrl() const override;

  signals:
    void applyPreset(Controller* pController, ControllerPresetPointer pPreset, bool bEnabled);
    void mappingStarted();
    void mappingEnded();

  private slots:
    /// Called when the user selects another preset in the combobox
    void slotPresetSelected(int index);
    /// Used to selected the current preset in the combobox and display the
    /// preset information.
    void slotShowPreset(ControllerPresetPointer preset);

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
    QString presetShortName(const ControllerPresetPointer pPreset) const;
    QString presetName(const ControllerPresetPointer pPreset) const;
    QString presetAuthor(const ControllerPresetPointer pPreset) const;
    QString presetDescription(const ControllerPresetPointer pPreset) const;
    QString presetForumLink(const ControllerPresetPointer pPreset) const;
    QString presetWikiLink(const ControllerPresetPointer pPreset) const;
    QString presetScriptFileLinks(const ControllerPresetPointer pPreset) const;
    void applyPresetChanges();
    void savePreset();
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
    void enumeratePresets(const QString& selectedPresetPath);
    PresetInfo enumeratePresetsFromEnumerator(
            QSharedPointer<PresetInfoEnumerator> pPresetEnumerator,
            QIcon icon = QIcon());

    void enableDevice();
    void disableDevice();

    Ui::DlgPrefControllerDlg m_ui;
    UserSettingsPointer m_pConfig;
    ControllerManager* m_pControllerManager;
    Controller* m_pController;
    DlgControllerLearning* m_pDlgControllerLearning;
    ControllerPresetPointer m_pPreset;
    ControllerInputMappingTableModel* m_pInputTableModel;
    QSortFilterProxyModel* m_pInputProxyModel;
    ControllerOutputMappingTableModel* m_pOutputTableModel;
    QSortFilterProxyModel* m_pOutputProxyModel;
    bool m_bDirty;
};

#endif /*DLGPREFCONTROLLER_H*/
