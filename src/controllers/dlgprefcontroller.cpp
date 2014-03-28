/**
* @file dlgprefcontroller.cpp
* @author Sean M. Pappalardo  spappalardo@mixxx.org
* @date Mon May 2 2011
* @brief Configuration dialog for a DJ controller
*/

#include <QtDebug>
#include <QFileInfo>

#include "controllers/dlgprefcontroller.h"
#include "controllers/controllerlearningeventfilter.h"
#include "controllers/controller.h"
#include "controllers/controllermanager.h"
#include "controllers/defs_controllers.h"
#include "configobject.h"

DlgPrefController::DlgPrefController(QWidget* parent, Controller* controller,
                                     ControllerManager* controllerManager,
                                     ConfigObject<ConfigValue> *pConfig)
        : DlgPreferencePage(parent),
          m_pConfig(pConfig),
          m_pControllerManager(controllerManager),
          m_pController(controller),
          m_pDlgControllerLearning(NULL),
          m_pInputTableModel(NULL),
          m_bDirty(false) {
    m_ui.setupUi(this);

    initTableView(m_ui.m_pInputMappingTableView);
    connect(m_pController, SIGNAL(presetLoaded(ControllerPresetPointer)),
            this, SLOT(slotPresetLoaded(ControllerPresetPointer)));
    const ControllerPresetPointer pPreset = controller->getPreset();
    slotPresetLoaded(pPreset);

    m_ui.labelDeviceName->setText(m_pController->getName());
    QString category = m_pController->getCategory();
    if (!category.isEmpty()) {
        m_ui.labelDeviceCategory->setText(category);
    } else {
        m_ui.labelDeviceCategory->hide();
    }

    // When the user picks a preset, load it.
    connect(m_ui.comboBoxPreset, SIGNAL(activated(int)),
            this, SLOT(slotLoadPreset(int)));

    // When the user toggles the Enabled checkbox, toggle.
    connect(m_ui.chkEnabledDevice, SIGNAL(clicked(bool)),
            this, SLOT(slotEnableDevice(bool)));

    // Connect our signals to controller manager.
    connect(this, SIGNAL(openController(Controller*)),
            m_pControllerManager, SLOT(openController(Controller*)));
    connect(this, SIGNAL(closeController(Controller*)),
            m_pControllerManager, SLOT(closeController(Controller*)));
    connect(this, SIGNAL(loadPreset(Controller*, QString, bool)),
            m_pControllerManager, SLOT(loadPreset(Controller*, QString, bool)));

    // If the controller is not mappable, disable the input and output mapping
    // sections.
    bool isMappable = m_pController->isMappable();
    m_ui.inputMappingsPage->setEnabled(isMappable);
    m_ui.outputMappingsPage->setEnabled(isMappable);

    // Input mappings
    connect(m_ui.btnAddInputMapping, SIGNAL(clicked()),
            this, SLOT(addInputMapping()));
    connect(m_ui.btnRemoveInputMappings, SIGNAL(clicked()),
            this, SLOT(removeInputMappings()));
    connect(m_ui.btnLearningWizard, SIGNAL(clicked()),
            this, SLOT(showLearningWizard()));
    connect(m_ui.btnClearAllInputMappings, SIGNAL(clicked()),
            this, SLOT(clearAllInputMappings()));

    // Output mappings
    connect(m_ui.btnAddOutputMapping, SIGNAL(clicked()),
            this, SLOT(addOutputMapping()));
    connect(m_ui.btnRemoveOutputMappings, SIGNAL(clicked()),
            this, SLOT(removeOutputMappings()));
    connect(m_ui.btnClearAllOutputMappings, SIGNAL(clicked()),
            this, SLOT(clearAllOutputMappings()));
}

DlgPrefController::~DlgPrefController() {
}

void DlgPrefController::showLearningWizard() {
    // If the user has checked the "Enabled" checkbox but they haven't hit OK to
    // apply it yet, prompt them to apply the settings before we open the
    // learning dialog. If we don't apply the settings first and open the
    // device, the dialog won't react to controller messages.
    if (m_ui.chkEnabledDevice->isChecked() && !m_pController->isOpen()) {
        QMessageBox::StandardButton result = QMessageBox::question(
            this,
            tr("Apply device settings?"),
            tr("Your settings must be applied before starting the learning wizard.\n"
               "Apply settings and continue?"),
            QMessageBox::Ok | QMessageBox::Cancel,  // Buttons to be displayed
            QMessageBox::Ok);  // Default button
        // Stop if the user has not pressed the Ok button,
        // which could be the Cancel or the Close Button.
        if (result != QMessageBox::Ok) {
            return;
        }
    }
    slotApply();

    // After this point we consider the mapping wizard as dirtying the preset.
    slotDirty();

    // Note that DlgControllerLearning is set to delete itself on close using
    // the Qt::WA_DeleteOnClose attribute (so this "new" doesn't leak memory)
    m_pDlgControllerLearning = new DlgControllerLearning(this, m_pController);
    m_pDlgControllerLearning->show();
    ControllerLearningEventFilter* pControllerLearning =
            m_pControllerManager->getControllerLearningEventFilter();
    pControllerLearning->startListening();
    connect(pControllerLearning, SIGNAL(controlClicked(ControlObject*)),
            m_pDlgControllerLearning, SLOT(controlClicked(ControlObject*)));
    connect(m_pDlgControllerLearning, SIGNAL(listenForClicks()),
            pControllerLearning, SLOT(startListening()));
    connect(m_pDlgControllerLearning, SIGNAL(stopListeningForClicks()),
            pControllerLearning, SLOT(stopListening()));
    connect(m_pDlgControllerLearning, SIGNAL(stopLearning()),
            this, SLOT(show()));


    emit(mappingStarted());
    connect(m_pDlgControllerLearning, SIGNAL(stopLearning()),
            this, SIGNAL(mappingEnded()));
}

QString DlgPrefController::presetShortName(const ControllerPresetPointer pPreset) const {
    QString presetName = tr("None");
    if (pPreset) {
        QString name = pPreset->name();
        QString author = pPreset->author();
        if (name.length() > 0 && author.length() > 0) {
            presetName = tr("%1 by %2").arg(pPreset->name(), pPreset->author());
        } else if (name.length() > 0) {
            presetName = name;
        } else if (pPreset->filePath().length() > 0) {
            QFileInfo file(pPreset->filePath());
            presetName = file.baseName();
        }
    }
    return presetName;
}

QString DlgPrefController::presetDescription(const ControllerPresetPointer pPreset) const {
    QString description = tr("No Description");
    if (pPreset) {
        QString descr = pPreset->description();
        if (description.length() > 0)
            description = descr;
    }
    return description;
}

QString DlgPrefController::presetForumLink(const ControllerPresetPointer pPreset) const {
    QString url;
    if (pPreset) {
        QString link = pPreset->forumlink();
        if (link.length() > 0)
            url = "<a href=\"" + link + "\">Mixxx Forums</a>";
    }
    return url;
}

QString DlgPrefController::presetWikiLink(const ControllerPresetPointer pPreset) const {
    QString url;
    if (pPreset) {
        QString link = pPreset->wikilink();
        if (link.length() > 0)
            url = "<a href=\"" + link + "\">Mixxx Wiki</a>";
    }
    return url;
}

void DlgPrefController::slotDirty() {
    m_bDirty = true;
}

QString nameForPreset(const PresetInfo& preset) {
    QString name = preset.getName();
    if (name.length() == 0) {
        QFileInfo file(preset.getPath());
        name = file.baseName();
    }
    return name;
}

void DlgPrefController::enumeratePresets() {
    m_ui.comboBoxPreset->clear();

    // qDebug() << "Enumerating presets for controller" << m_pController->getName();

    // Insert a dummy "..." item at the top to try to make it less confusing.
    // (We don't want the first found file showing up as the default item when a
    // user has their controller plugged in)
    m_ui.comboBoxPreset->addItem("...");

    m_ui.comboBoxPreset->setInsertPolicy(QComboBox::InsertAlphabetically);
    // Ask the controller manager for a list of applicable presets
    PresetInfoEnumerator* pie =  m_pControllerManager->getPresetInfoManager();
    QList<PresetInfo> presets = pie->getPresets(m_pController->presetExtension());

    PresetInfo match;
    foreach (PresetInfo preset, presets) {
        m_ui.comboBoxPreset->addItem(nameForPreset(preset), preset.getPath());
        if (m_pController->matchPreset(preset)) {
            match = preset;
            break;
        }
    }

    // Jump to matching device in list if it was found.
    if (match.isValid()) {
        int index = m_ui.comboBoxPreset->findText(nameForPreset(match));
        if (index != -1) {
            m_ui.comboBoxPreset->setCurrentIndex(index);
        }
    }
}

void DlgPrefController::slotUpdate() {
    // Check if the device that this dialog is for is already enabled...
    bool deviceOpen = m_pController->isOpen();
    // Check/uncheck the "Enabled" box
    m_ui.chkEnabledDevice->setChecked(deviceOpen);
    // Enable/disable presets group box.
    m_ui.groupBoxPresets->setEnabled(deviceOpen);
    // Enable/disable learning wizard option.
    m_ui.btnLearningWizard->setEnabled(deviceOpen);
}

void DlgPrefController::slotApply() {
    /* User has pressed OK, so if anything has been changed, enable or disable
     * the device, write the controls to the DOM, and reload the presets.
     */
    if (m_bDirty) {
        if (m_ui.chkEnabledDevice->isChecked()) {
            enableDevice();
        } else {
            disableDevice();
        }

        //Select the "..." item again in the combobox.
        m_ui.comboBoxPreset->setCurrentIndex(0);
    }
    m_bDirty = false;
}

void DlgPrefController::slotLoadPreset(int chosenIndex) {
    if (chosenIndex == 0) {
        // User picked ...
        return;
    }

    QString presetPath = m_ui.comboBoxPreset->itemData(chosenIndex).toString();

    // Applied on prefs close
    emit(loadPreset(m_pController, presetPath, true));
    slotDirty();
}

void DlgPrefController::initTableView(QTableView* pTable) {
    // Editing starts when clicking on an already selected item.
    pTable->setEditTriggers(QAbstractItemView::SelectedClicked);

    // Enable selection by rows and extended selection (ctrl/shift click)
    pTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    pTable->setSelectionMode(QAbstractItemView::ExtendedSelection);

    pTable->setWordWrap(false);
    pTable->setShowGrid(false);
    pTable->setCornerButtonEnabled(false);
    pTable->setSortingEnabled(true);

    //Work around a Qt bug that lets you make your columns so wide you
    //can't reach the divider to make them small again.
    pTable->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);

    pTable->verticalHeader()->hide();
    pTable->verticalHeader()->setDefaultSectionSize(20);
    pTable->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    pTable->setAlternatingRowColors(true);
}

void DlgPrefController::slotPresetLoaded(ControllerPresetPointer preset) {
    m_ui.labelLoadedPreset->setText(presetShortName(preset));
    m_ui.labelLoadedPresetDescription->setText(presetDescription(preset));
    QStringList supportLinks;
    QString forumLink = presetForumLink(preset);
    if (forumLink.length() > 0) {
        supportLinks << forumLink;
    }
    QString wikiLink = presetWikiLink(preset);
    if (wikiLink.length() > 0) {
        supportLinks << wikiLink;
    }
    QString support = supportLinks.join("&nbsp;");
    if (support.length() == 0) {
        //: Shown when a MIDI controller has no links to support pages (e.g. Mixxx wiki or forums).
        support = tr("No support available.");
    }
    m_ui.labelLoadedPresetSupportLinks->setText(support);

    ControllerInputMappingTableModel* pInputModel =
            new ControllerInputMappingTableModel(this);
    pInputModel->setPreset(preset);
    m_ui.m_pInputMappingTableView->setModel(pInputModel);

    for (int i = 0; i < pInputModel->columnCount(); ++i) {
        QAbstractItemDelegate* pDelegate = pInputModel->delegateForColumn(
            i, m_ui.m_pInputMappingTableView);
        if (pDelegate != NULL) {
            m_ui.m_pInputMappingTableView->setItemDelegateForColumn(i, pDelegate);
        }
    }

    // Now that we have set the new model our old model can be deleted.
    if (m_pInputTableModel) {
        delete m_pInputTableModel;
    }
    m_pInputTableModel = pInputModel;
}

void DlgPrefController::slotEnableDevice(bool enable) {
    // Enable/disable presets group box and learning wizard button.
    m_ui.groupBoxPresets->setEnabled(enable);
    m_ui.btnLearningWizard->setEnabled(enable);
    slotDirty();

    // Set tree item text to normal/bold.
    emit(controllerEnabled(this, enable));
}

void DlgPrefController::enableDevice() {
    emit(openController(m_pController));
    //TODO: Should probably check if open() actually succeeded.
}

void DlgPrefController::disableDevice() {
    emit(closeController(m_pController));
    //TODO: Should probably check if close() actually succeeded.
}

void DlgPrefController::addInputMapping() {
    if (m_pInputTableModel) {
        m_pInputTableModel->addEmptyInputMapping();
        slotDirty();
    }
}

void DlgPrefController::removeInputMappings() {
    QModelIndexList selectedIndices =
            m_ui.m_pInputMappingTableView->selectionModel()->selectedRows();
    if (selectedIndices.size() > 0 && m_pInputTableModel) {
        m_pInputTableModel->removeInputMappings(selectedIndices);
        slotDirty();
    }
}

void DlgPrefController::clearAllInputMappings() {
    if (QMessageBox::warning(
            this, tr("Clear Input Mappings"),
            tr("Are you sure you want to clear all input mappings?"),
            QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel) != QMessageBox::Ok) {
        return;
    }
    if (m_pInputTableModel) {
        m_pInputTableModel->clear();
        slotDirty();
    }
}

void DlgPrefController::addOutputMapping() {
}

void DlgPrefController::removeOutputMappings() {
    QModelIndexList selectedIndices =
            m_ui.m_pOutputMappingTableView->selectionModel()->selectedRows();
}

void DlgPrefController::clearAllOutputMappings() {
    if (QMessageBox::warning(
            this, tr("Clear Output Mappings"),
            tr("Are you sure you want to clear all output mappings?"),
            QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel) != QMessageBox::Ok) {
        return;
    }
}
