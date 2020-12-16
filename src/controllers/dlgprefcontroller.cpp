#include "controllers/dlgprefcontroller.h"

#include <QDesktopServices>
#include <QFileDialog>
#include <QFileInfo>
#include <QInputDialog>
#include <QStandardPaths>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QtDebug>

#include "controllers/controller.h"
#include "controllers/controllerlearningeventfilter.h"
#include "controllers/controllermanager.h"
#include "controllers/defs_controllers.h"
#include "defs_urls.h"
#include "moc_dlgprefcontroller.cpp"
#include "preferences/usersettings.h"
#include "util/version.h"

namespace {
const QString kPresetExt(".midi.xml");
} // namespace

DlgPrefController::DlgPrefController(QWidget* parent,
        Controller* controller,
        ControllerManager* controllerManager,
        UserSettingsPointer pConfig)
        : DlgPreferencePage(parent),
          m_pConfig(pConfig),
          m_pUserDir(userPresetsPath(pConfig)),
          m_pControllerManager(controllerManager),
          m_pController(controller),
          m_pDlgControllerLearning(nullptr),
          m_pInputTableModel(nullptr),
          m_pInputProxyModel(nullptr),
          m_pOutputTableModel(nullptr),
          m_pOutputProxyModel(nullptr),
          m_bDirty(false) {
    m_ui.setupUi(this);

    initTableView(m_ui.m_pInputMappingTableView);
    initTableView(m_ui.m_pOutputMappingTableView);

    connect(m_pController, &Controller::presetLoaded, this, &DlgPrefController::slotShowPreset);
    // TODO(rryan): Eh, this really isn't thread safe but it's the way it's been
    // since 1.11.0. We shouldn't be calling Controller methods because it lives
    // in a different thread. Booleans (like isOpen()) are fine but a complex
    // object like a preset involves QHash's and other data structures that
    // really don't like concurrent access.
    ControllerPresetPointer pPreset = m_pController->getPreset();
    slotShowPreset(pPreset);

    m_ui.labelDeviceName->setText(m_pController->getName());
    QString category = m_pController->getCategory();
    if (!category.isEmpty()) {
        m_ui.labelDeviceCategory->setText(category);
    } else {
        m_ui.labelDeviceCategory->hide();
    }

    // When the user picks a preset, load it.
    connect(m_ui.comboBoxPreset,
            QOverload<int>::of(&QComboBox::activated),
            this,
            &DlgPrefController::slotPresetSelected);

    // When the user toggles the Enabled checkbox, mark as dirty
    connect(m_ui.chkEnabledDevice, &QCheckBox::clicked, this, [this] { setDirty(true); });

    // Connect our signals to controller manager.
    connect(this,
            &DlgPrefController::applyPreset,
            m_pControllerManager,
            &ControllerManager::slotApplyPreset);

    // Open script file links
    connect(m_ui.labelLoadedPresetScriptFileLinks,
            &QLabel::linkActivated,
            [](const QString& path) {
                QDesktopServices::openUrl(QUrl::fromLocalFile(path));
            });

    // Input mappings
    connect(m_ui.btnAddInputMapping,
            &QAbstractButton::clicked,
            this,
            &DlgPrefController::addInputMapping);
    connect(m_ui.btnRemoveInputMappings,
            &QAbstractButton::clicked,
            this,
            &DlgPrefController::removeInputMappings);
    connect(m_ui.btnLearningWizard,
            &QAbstractButton::clicked,
            this,
            &DlgPrefController::showLearningWizard);
    connect(m_ui.btnClearAllInputMappings,
            &QAbstractButton::clicked,
            this,
            &DlgPrefController::clearAllInputMappings);

    // Output mappings
    connect(m_ui.btnAddOutputMapping,
            &QAbstractButton::clicked,
            this,
            &DlgPrefController::addOutputMapping);
    connect(m_ui.btnRemoveOutputMappings,
            &QAbstractButton::clicked,
            this,
            &DlgPrefController::removeOutputMappings);
    connect(m_ui.btnClearAllOutputMappings,
            &QAbstractButton::clicked,
            this,
            &DlgPrefController::clearAllOutputMappings);
}

DlgPrefController::~DlgPrefController() {
}

void DlgPrefController::showLearningWizard() {
    // If the user has checked the "Enabled" checkbox but they haven't hit OK to
    // apply it yet, prompt them to apply the settings before we open the
    // learning dialog. If we don't apply the settings first and open the
    // device, the dialog won't react to controller messages.
    if (m_ui.chkEnabledDevice->isChecked() && !m_pController->isOpen()) {
        QMessageBox::StandardButton result = QMessageBox::question(this,
                tr("Apply device settings?"),
                tr("Your settings must be applied before starting the learning "
                   "wizard.\n"
                   "Apply settings and continue?"),
                QMessageBox::Ok |
                        QMessageBox::Cancel, // Buttons to be displayed
                QMessageBox::Ok);            // Default button
        // Stop if the user has not pressed the Ok button,
        // which could be the Cancel or the Close Button.
        if (result != QMessageBox::Ok) {
            return;
        }
    }
    slotApply();

    // Note that DlgControllerLearning is set to delete itself on close using
    // the Qt::WA_DeleteOnClose attribute (so this "new" doesn't leak memory)
    m_pDlgControllerLearning = new DlgControllerLearning(this, m_pController);
    m_pDlgControllerLearning->show();
    ControllerLearningEventFilter* pControllerLearning =
            m_pControllerManager->getControllerLearningEventFilter();
    pControllerLearning->startListening();
    connect(pControllerLearning,
            &ControllerLearningEventFilter::controlClicked,
            m_pDlgControllerLearning,
            &DlgControllerLearning::controlClicked);
    connect(m_pDlgControllerLearning,
            &DlgControllerLearning::listenForClicks,
            pControllerLearning,
            &ControllerLearningEventFilter::startListening);
    connect(m_pDlgControllerLearning,
            &DlgControllerLearning::stopListeningForClicks,
            pControllerLearning,
            &ControllerLearningEventFilter::stopListening);
    connect(m_pDlgControllerLearning,
            &DlgControllerLearning::stopLearning,
            this,
            &DlgPrefController::show);
    connect(m_pDlgControllerLearning,
            &DlgControllerLearning::inputMappingsLearned,
            this,
            &DlgPrefController::midiInputMappingsLearned);

    emit mappingStarted();
    connect(m_pDlgControllerLearning,
            &DlgControllerLearning::stopLearning,
            this,
            &DlgPrefController::mappingEnded);
}

void DlgPrefController::midiInputMappingsLearned(
        const MidiInputMappings& mappings) {
    // This is just a shortcut since doing a round-trip from Learning ->
    // Controller -> slotPresetLoaded -> setPreset is too heavyweight.
    if (m_pInputTableModel != nullptr) {
        m_pInputTableModel->addMappings(mappings);
    }
}

QString DlgPrefController::presetShortName(
        const ControllerPresetPointer pPreset) const {
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

QString DlgPrefController::presetName(
        const ControllerPresetPointer pPreset) const {
    if (pPreset) {
        QString name = pPreset->name();
        if (name.length() > 0) {
            return name;
        }
    }
    return tr("No Name");
}

QString DlgPrefController::presetDescription(
        const ControllerPresetPointer pPreset) const {
    if (pPreset) {
        QString description = pPreset->description();
        if (description.length() > 0) {
            return description;
        }
    }
    return tr("No Description");
}

QString DlgPrefController::presetAuthor(
        const ControllerPresetPointer pPreset) const {
    if (pPreset) {
        QString author = pPreset->author();
        if (author.length() > 0) {
            return author;
        }
    }
    return tr("No Author");
}

QString DlgPrefController::presetForumLink(
        const ControllerPresetPointer pPreset) const {
    QString url;
    if (pPreset) {
        QString link = pPreset->forumlink();
        if (link.length() > 0) {
            url = "<a href=\"" + link + "\">Mixxx Forums</a>";
        }
    }
    return url;
}

QString DlgPrefController::presetWikiLink(
        const ControllerPresetPointer pPreset) const {
    QString url;
    if (pPreset) {
        QString link = pPreset->wikilink();
        if (link.length() > 0) {
            url = "<a href=\"" + link + "\">Mixxx Wiki</a>";
        }
    }
    return url;
}

QString DlgPrefController::presetManualLink(
        const ControllerPresetPointer pPreset) const {
    QString url;
    if (pPreset) {
        QString link = pPreset->manualLink();
        if (!link.isEmpty()) {
            url = "<a href=\"" + link + "\">Manual</a>";
        }
    }
    return url;
}

QString DlgPrefController::presetFileLinks(
        const ControllerPresetPointer pPreset) const {
    if (!pPreset) {
        return QString();
    }

    const QString builtinFileSuffix = QStringLiteral(" (") + tr("built-in") + QStringLiteral(")");
    QString systemPresetPath = resourcePresetsPath(m_pConfig);
    QStringList linkList;
    QString xmlFileName = QFileInfo(pPreset->filePath()).fileName();
    QString xmlFileLink = QStringLiteral("<a href=\"") +
            pPreset->filePath() + QStringLiteral("\">") +
            xmlFileName + QStringLiteral("</a>");
    if (pPreset->filePath().startsWith(systemPresetPath)) {
        xmlFileLink += builtinFileSuffix;
    }
    linkList << xmlFileLink;

    for (const auto& script : pPreset->getScriptFiles()) {
        QString scriptFileLink = QStringLiteral("<a href=\"") +
                script.file.absoluteFilePath() + QStringLiteral("\">") +
                script.name + QStringLiteral("</a>");

        if (!script.file.exists()) {
            scriptFileLink +=
                    QStringLiteral(" (") + tr("missing") + QStringLiteral(")");
        } else if (script.file.absoluteFilePath().startsWith(
                           systemPresetPath)) {
            scriptFileLink += builtinFileSuffix;
        }

        linkList << scriptFileLink;
    }
    return linkList.join("<br/>");
}

void DlgPrefController::enumeratePresets(const QString& selectedPresetPath) {
    m_ui.comboBoxPreset->clear();

    // qDebug() << "Enumerating presets for controller" << m_pController->getName();

    // Insert a dummy item at the top to try to make it less confusing.
    // (We don't want the first found file showing up as the default item when a
    // user has their controller plugged in)
    QIcon noPresetIcon(":/images/ic_none.svg");
    m_ui.comboBoxPreset->addItem(noPresetIcon, "No Preset");

    PresetInfo match;
    // Enumerate user presets
    QIcon userPresetIcon(":/images/ic_custom.svg");

    // Reload user presets to detect added, changed or removed mappings
    m_pControllerManager->getMainThreadUserPresetEnumerator()->loadSupportedPresets();

    PresetInfo userPresetsMatch = enumeratePresetsFromEnumerator(
            m_pControllerManager->getMainThreadUserPresetEnumerator(),
            userPresetIcon);
    if (userPresetsMatch.isValid()) {
        match = userPresetsMatch;
    }

    // Insert a separator between user presets (+ dummy item) and system presets
    m_ui.comboBoxPreset->insertSeparator(m_ui.comboBoxPreset->count());

    // Enumerate system presets
    QIcon systemPresetIcon(":/images/ic_mixxx_symbolic.svg");
    PresetInfo systemPresetsMatch = enumeratePresetsFromEnumerator(
            m_pControllerManager->getMainThreadSystemPresetEnumerator(),
            systemPresetIcon);
    if (systemPresetsMatch.isValid()) {
        match = systemPresetsMatch;
    }

    // Preselect configured or matching preset
    int index = -1;
    if (!selectedPresetPath.isEmpty()) {
        index = m_ui.comboBoxPreset->findData(selectedPresetPath);
    } else if (match.isValid()) {
        index = m_ui.comboBoxPreset->findText(match.getName());
    }
    if (index == -1) {
        m_ui.chkEnabledDevice->setEnabled(false);
    } else {
        m_ui.comboBoxPreset->setCurrentIndex(index);
        m_ui.chkEnabledDevice->setEnabled(true);
    }
}

PresetInfo DlgPrefController::enumeratePresetsFromEnumerator(
        QSharedPointer<PresetInfoEnumerator> pPresetEnumerator, const QIcon& icon) {
    PresetInfo match;

    // Check if enumerator is ready. Should be rare that it isn't. We will
    // re-enumerate on the next open of the preferences.
    if (!pPresetEnumerator.isNull()) {
        // Get a list of presets in alphabetical order
        QList<PresetInfo> systemPresets =
                pPresetEnumerator->getPresetsByExtension(
                        m_pController->presetExtension());

        for (const PresetInfo& preset : systemPresets) {
            m_ui.comboBoxPreset->addItem(
                    icon, preset.getName(), preset.getPath());
            if (m_pController->matchPreset(preset)) {
                match = preset;
            }
        }
    }

    return match;
}

void DlgPrefController::slotUpdate() {
    enumeratePresets(m_pControllerManager->getConfiguredPresetFileForDevice(
            m_pController->getName()));

    // Check if the controller is open.
    bool deviceOpen = m_pController->isOpen();
    // Check/uncheck the "Enabled" box
    m_ui.chkEnabledDevice->setChecked(deviceOpen);

    // If the controller is not mappable, disable the input and output mapping
    // sections and the learning wizard button.
    bool isMappable = m_pController->isMappable();
    m_ui.btnLearningWizard->setEnabled(isMappable);
    m_ui.inputMappingsTab->setEnabled(isMappable);
    m_ui.outputMappingsTab->setEnabled(isMappable);
}

void DlgPrefController::slotResetToDefaults() {
    m_ui.chkEnabledDevice->setChecked(false);
    enumeratePresets(QString());
    slotPresetSelected(m_ui.comboBoxPreset->currentIndex());
}

void DlgPrefController::applyPresetChanges() {
    if (m_pInputTableModel) {
        m_pInputTableModel->apply();
    }

    if (m_pOutputTableModel) {
        m_pOutputTableModel->apply();
    }
}

void DlgPrefController::slotApply() {
    applyPresetChanges();

    // If no changes were made, do nothing
    if (!(isDirty() || (m_pPreset && m_pPreset->isDirty()))) {
        return;
    }

    bool bEnabled = false;
    if (m_pPreset) {
        bEnabled = m_ui.chkEnabledDevice->isChecked();

        if (m_pPreset->isDirty()) {
            savePreset();
        }
    }
    m_ui.chkEnabledDevice->setChecked(bEnabled);

    // The shouldn't be dirty at this pint because we already tried to save
    // it. If that failed, don't apply the preset.
    if (m_pPreset && m_pPreset->isDirty()) {
        return;
    }

    // Load the resulting preset (which has been mutated by the input/output
    // table models). The controller clones the preset so we aren't touching
    // the same preset.
    emit applyPreset(m_pController, m_pPreset, bEnabled);

    // Mark the dialog as not dirty
    setDirty(false);
}

QUrl DlgPrefController::helpUrl() const {
    return QUrl(MIXXX_MANUAL_CONTROLLERS_URL);
}

void DlgPrefController::slotPresetSelected(int chosenIndex) {
    QString presetPath;
    if (chosenIndex == 0) {
        // User picked "No Preset" item
        m_ui.chkEnabledDevice->setEnabled(false);

        if (m_ui.chkEnabledDevice->isChecked()) {
            m_ui.chkEnabledDevice->setChecked(false);
            setDirty(true);
        }
    } else {
        // User picked a preset
        m_ui.chkEnabledDevice->setEnabled(true);

        if (!m_ui.chkEnabledDevice->isChecked()) {
            m_ui.chkEnabledDevice->setChecked(true);
            setDirty(true);
        }

        presetPath = m_ui.comboBoxPreset->itemData(chosenIndex).toString();
    }

    // Check if the preset is different from the configured preset
    if (m_pControllerManager->getConfiguredPresetFileForDevice(
                m_pController->getName()) != presetPath) {
        setDirty(true);
    }

    applyPresetChanges();
    if (m_pPreset && m_pPreset->isDirty()) {
        if (QMessageBox::question(this,
                    tr("Mapping has been edited"),
                    tr("Do you want to save the changes?")) ==
                QMessageBox::Yes) {
            savePreset();
        }
    }

    ControllerPresetPointer pPreset = ControllerPresetFileHandler::loadPreset(
            presetPath, QDir(resourcePresetsPath(m_pConfig)));

    if (pPreset) {
        DEBUG_ASSERT(!pPreset->isDirty());
    }

    slotShowPreset(pPreset);
}

void DlgPrefController::savePreset() {
    VERIFY_OR_DEBUG_ASSERT(m_pPreset) {
        return;
    }

    if (!m_pPreset->isDirty()) {
        qDebug() << "Mapping is not dirty, no need to save it.";
        return;
    }

    QString oldFilePath = m_pPreset->filePath();
    QString newFilePath;
    QFileInfo fileInfo(oldFilePath);
    QString presetName = m_pPreset->name();

    bool isUserPreset = fileInfo.absoluteDir().absolutePath().append("/") == m_pUserDir;
    bool saveAsNew = true;
    if (m_pOverwritePresets.contains(oldFilePath) &&
            m_pOverwritePresets.value(oldFilePath) == true) {
        saveAsNew = false;
    }

    // If this is a user preset, ask whether to overwrite or save with new name.
    // Optionally, tick checkbox to always overwrite this preset in the current session.
    if (isUserPreset && saveAsNew) {
        QString overwriteTitle = tr("Preset already exists.");
        QString overwriteLabel = tr(
                "<b>%1</b> already exists in user preset folder.<br>"
                "Overwrite or save with a new name?");
        QString overwriteCheckLabel = tr("Always overwrite during this session");

        QMessageBox overwriteMsgBox;
        overwriteMsgBox.setIcon(QMessageBox::Question);
        overwriteMsgBox.setWindowTitle(overwriteTitle);
        overwriteMsgBox.setText(overwriteLabel.arg(presetName));
        QCheckBox overwriteCheckBox;
        overwriteCheckBox.setText(overwriteCheckLabel);
        overwriteCheckBox.blockSignals(true);
        overwriteCheckBox.setCheckState(Qt::Unchecked);
        overwriteMsgBox.addButton(&overwriteCheckBox, QMessageBox::ActionRole);
        QPushButton* pSaveAsNew = overwriteMsgBox.addButton(
                tr("Save As"), QMessageBox::AcceptRole);
        QPushButton* pOverwrite = overwriteMsgBox.addButton(
                tr("Overwrite"), QMessageBox::AcceptRole);
        overwriteMsgBox.setDefaultButton(pSaveAsNew);
        overwriteMsgBox.exec();

        if (overwriteMsgBox.clickedButton() == pOverwrite) {
            saveAsNew = false;
            if (overwriteCheckBox.checkState() == Qt::Checked) {
                m_pOverwritePresets.insert(m_pPreset->filePath(), true);
            }
        } else if (overwriteMsgBox.close()) {
            return;
        }
    }

    // Ask for a preset name when
    // * initially saving a modified Mixxx preset to the user folder
    // * saving a user preset with a new name.
    // The name will be used as display name and file name.
    if (!saveAsNew) {
        newFilePath = oldFilePath;
    } else {
        QString savePresetTitle = tr("Save user mapping");
        QString savePresetLabel = tr("Enter the name for saving the mapping to the user folder.");
        QString savingFailedTitle = tr("Saving mapping failed");
        QString invalidNameLabel =
                tr("A mapping cannot have a blank name and may not contain "
                   "special characters.");
        QString fileExistsLabel = tr("A mapping file with that name already exists.");
        // Only allow the name to contain letters, numbers, whitespaces and _-+()/
        const QRegExp rxRemove = QRegExp("[^[(a-zA-Z0-9\\_\\-\\+\\(\\)\\/|\\s]");

        // Choose a new file (base) name
        bool validPresetName = false;
        while (!validPresetName) {
            QString userDir = m_pUserDir;
            bool ok = false;
            presetName = QInputDialog::getText(nullptr,
                    savePresetTitle,
                    savePresetLabel,
                    QLineEdit::Normal,
                    presetName,
                    &ok)
                                 .remove(rxRemove)
                                 .trimmed();
            if (!ok) {
                return;
            }
            if (presetName.isEmpty()) {
                QMessageBox::warning(nullptr,
                        savingFailedTitle,
                        invalidNameLabel);
                continue;
            }
            // While / is allowed for the display name we can't use it for the file name.
            QString fileName = presetName.replace(QString("/"), QString("-"));
            newFilePath = userDir + fileName + kPresetExt;
            if (QFile::exists(newFilePath)) {
                QMessageBox::warning(nullptr,
                        savingFailedTitle,
                        fileExistsLabel);
                continue;
            }
            validPresetName = true;
        }
        m_pPreset->setName(presetName);
        qDebug() << "Mapping renamed to" << m_pPreset->name();
    }

    if (!m_pPreset->savePreset(newFilePath)) {
        qDebug() << "Failed to save mapping as" << newFilePath;
        return;
    }
    qDebug() << "Mapping saved as" << newFilePath;

    m_pPreset->setFilePath(newFilePath);
    m_pPreset->setDirty(false);

    enumeratePresets(m_pPreset->filePath());
}

void DlgPrefController::initTableView(QTableView* pTable) {
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

void DlgPrefController::slotShowPreset(ControllerPresetPointer preset) {
    m_ui.labelLoadedPreset->setText(presetName(preset));
    m_ui.labelLoadedPresetDescription->setText(presetDescription(preset));
    m_ui.labelLoadedPresetAuthor->setText(presetAuthor(preset));
    QStringList supportLinks;

    QString forumLink = presetForumLink(preset);
    if (forumLink.length() > 0) {
        supportLinks << forumLink;
    }

    QString manualLink = presetManualLink(preset);
    if (manualLink.length() > 0) {
        supportLinks << manualLink;
    }

    QString wikiLink = presetWikiLink(preset);
    if (wikiLink.length() > 0) {
        supportLinks << wikiLink;
    }

    // There is always at least one support link.
    // TODO(rryan): This is a horrible general support link for MIDI!
    QString troubleShooting = QString(
        "<a href=\"http://mixxx.org/wiki/doku.php/midi_scripting\">%1</a>")
            .arg(tr("Troubleshooting"));
    supportLinks << troubleShooting;

    QString support = supportLinks.join("&nbsp;&nbsp;");
    m_ui.labelLoadedPresetSupportLinks->setText(support);

    QString mappingFileLinks = presetFileLinks(preset);
    m_ui.labelLoadedPresetScriptFileLinks->setText(mappingFileLinks);

    // We mutate this preset so keep a reference to it while we are using it.
    // TODO(rryan): Clone it? Technically a waste since nothing else uses this
    // copy but if someone did they might not expect it to change.
    m_pPreset = preset;

    ControllerInputMappingTableModel* pInputModel =
            new ControllerInputMappingTableModel(this);
    pInputModel->setPreset(preset);

    QSortFilterProxyModel* pInputProxyModel = new QSortFilterProxyModel(this);
    pInputProxyModel->setSortRole(Qt::UserRole);
    pInputProxyModel->setSourceModel(pInputModel);
    m_ui.m_pInputMappingTableView->setModel(pInputProxyModel);

    for (int i = 0; i < pInputModel->columnCount(); ++i) {
        QAbstractItemDelegate* pDelegate = pInputModel->delegateForColumn(
            i, m_ui.m_pInputMappingTableView);
        if (pDelegate != nullptr) {
            qDebug() << "Setting input delegate for column" << i << pDelegate;
            m_ui.m_pInputMappingTableView->setItemDelegateForColumn(i, pDelegate);
        }
    }

    // Now that we have set the new model our old model can be deleted.
    delete m_pInputProxyModel;
    m_pInputProxyModel = pInputProxyModel;
    delete m_pInputTableModel;
    m_pInputTableModel = pInputModel;

    ControllerOutputMappingTableModel* pOutputModel =
            new ControllerOutputMappingTableModel(this);
    pOutputModel->setPreset(preset);

    QSortFilterProxyModel* pOutputProxyModel = new QSortFilterProxyModel(this);
    pOutputProxyModel->setSortRole(Qt::UserRole);
    pOutputProxyModel->setSourceModel(pOutputModel);
    m_ui.m_pOutputMappingTableView->setModel(pOutputProxyModel);

    for (int i = 0; i < pOutputModel->columnCount(); ++i) {
        QAbstractItemDelegate* pDelegate = pOutputModel->delegateForColumn(
            i, m_ui.m_pOutputMappingTableView);
        if (pDelegate != nullptr) {
            qDebug() << "Setting output delegate for column" << i << pDelegate;
            m_ui.m_pOutputMappingTableView->setItemDelegateForColumn(i, pDelegate);
        }
    }

    // Now that we have set the new model our old model can be deleted.
    delete m_pOutputProxyModel;
    m_pOutputProxyModel = pOutputProxyModel;
    delete m_pOutputTableModel;
    m_pOutputTableModel = pOutputModel;
}

void DlgPrefController::addInputMapping() {
    if (m_pInputTableModel) {
        m_pInputTableModel->addEmptyMapping();
        // Ensure the added row is visible.
        QModelIndex left = m_pInputProxyModel->mapFromSource(
            m_pInputTableModel->index(m_pInputTableModel->rowCount() - 1, 0));
        QModelIndex right = m_pInputProxyModel->mapFromSource(
            m_pInputTableModel->index(m_pInputTableModel->rowCount() - 1,
                                       m_pInputTableModel->columnCount() - 1));
        m_ui.m_pInputMappingTableView->selectionModel()->select(
            QItemSelection(left, right), QItemSelectionModel::Clear | QItemSelectionModel::Select);
        m_ui.m_pInputMappingTableView->scrollTo(left);
    }
}

void DlgPrefController::removeInputMappings() {
    if (m_pInputProxyModel) {
        QItemSelection selection = m_pInputProxyModel->mapSelectionToSource(
            m_ui.m_pInputMappingTableView->selectionModel()->selection());
        QModelIndexList selectedIndices = selection.indexes();
        if (selectedIndices.size() > 0 && m_pInputTableModel) {
            m_pInputTableModel->removeMappings(selectedIndices);
        }
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
    }
}

void DlgPrefController::addOutputMapping() {
    if (m_pOutputTableModel) {
        m_pOutputTableModel->addEmptyMapping();
        // Ensure the added row is visible.
        QModelIndex left = m_pOutputProxyModel->mapFromSource(
            m_pOutputTableModel->index(m_pOutputTableModel->rowCount() - 1, 0));
        QModelIndex right = m_pOutputProxyModel->mapFromSource(
            m_pOutputTableModel->index(m_pOutputTableModel->rowCount() - 1,
                                       m_pOutputTableModel->columnCount() - 1));
        m_ui.m_pOutputMappingTableView->selectionModel()->select(
            QItemSelection(left, right), QItemSelectionModel::Clear | QItemSelectionModel::Select);
        m_ui.m_pOutputMappingTableView->scrollTo(left);
    }
}

void DlgPrefController::removeOutputMappings() {
    if (m_pOutputProxyModel) {
        QItemSelection selection = m_pOutputProxyModel->mapSelectionToSource(
            m_ui.m_pOutputMappingTableView->selectionModel()->selection());
        QModelIndexList selectedIndices = selection.indexes();
        if (selectedIndices.size() > 0 && m_pOutputTableModel) {
            m_pOutputTableModel->removeMappings(selectedIndices);
        }
    }
}

void DlgPrefController::clearAllOutputMappings() {
    if (QMessageBox::warning(
            this, tr("Clear Output Mappings"),
            tr("Are you sure you want to clear all output mappings?"),
            QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel) != QMessageBox::Ok) {
        return;
    }
    if (m_pOutputTableModel) {
        m_pOutputTableModel->clear();
    }
}
