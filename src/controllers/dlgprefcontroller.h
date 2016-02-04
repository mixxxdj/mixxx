/**
* @file dlgprefcontroller.h
* @author Sean M. Pappalardo  spappalardo@mixxx.org
* @date Mon May 2 2011
* @brief Configuration dialog for a DJ controller
*/

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

class DlgPrefController : public DlgPreferencePage {
    Q_OBJECT
  public:
    DlgPrefController(QWidget *parent, Controller* controller,
                      ControllerManager* controllerManager,
                      UserSettingsPointer pConfig);
    virtual ~DlgPrefController();

  public slots:
    // Called when we should apply / save our changes.
    void slotApply();
    // Called when we should cancel the changes made.
    void slotCancel();
    // Called when preference dialog (not this dialog) is displayed.
    void slotUpdate();
    // Called when the user toggles the enabled checkbox.
    void slotEnableDevice(bool enable);
    // Called when the user selects a preset from the combobox.
    void slotLoadPreset(int index);
    // Mark that we need to apply the settings.
    void slotDirty();
    // Reload the mappings in the dropdown dialog
    void enumeratePresets();

  signals:
    void controllerEnabled(DlgPrefController*, bool);
    void openController(Controller* pController);
    void closeController(Controller* pController);
    void loadPreset(Controller* pController, QString controllerName);
    void loadPreset(Controller* pController, ControllerPresetPointer pPreset);
    void mappingStarted();
    void mappingEnded();

  private slots:
    void slotPresetLoaded(ControllerPresetPointer preset);

    // Input mappings
    void addInputMapping();
    void showLearningWizard();
    void removeInputMappings();
    void clearAllInputMappings();

    // Output mappings
    void addOutputMapping();
    void removeOutputMappings();
    void clearAllOutputMappings();

    // Scripts
    void addScript();
    void removeScript();
    void openScript();

    void midiInputMappingsLearned(const MidiInputMappings& mappings);

  private:
    QString presetShortName(const ControllerPresetPointer pPreset) const;
    QString presetName(const ControllerPresetPointer pPreset) const;
    QString presetAuthor(const ControllerPresetPointer pPreset) const;
    QString presetDescription(const ControllerPresetPointer pPreset) const;
    QString presetForumLink(const ControllerPresetPointer pPreset) const;
    QString presetWikiLink(const ControllerPresetPointer pPreset) const;
    void savePreset(QString path);
    void initTableView(QTableView* pTable);

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
