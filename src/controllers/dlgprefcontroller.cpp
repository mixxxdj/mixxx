#include "controllers/dlgprefcontroller.h"

#include <QDesktopServices>
#include <QDir>
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
#include "controllers/midi/legacymidicontrollermapping.h"
#include "defs_urls.h"
#include "moc_dlgprefcontroller.cpp"
#include "preferences/usersettings.h"
#include "util/version.h"

namespace {
const QString kMappingExt(".midi.xml");

QString mappingNameToPath(const QString& directory, const QString& mappingName) {
    // While / is allowed for the display name we can't use it for the file name.
    QString fileName = QString(mappingName).replace(QChar('/'), QChar('-'));
    return directory + fileName + kMappingExt;
}

} // namespace

DlgPrefController::DlgPrefController(
        QWidget* parent,
        Controller* controller,
        std::shared_ptr<ControllerManager> controllerManager,
        UserSettingsPointer pConfig)
        : DlgPreferencePage(parent),
          m_pConfig(pConfig),
          m_pUserDir(userMappingsPath(pConfig)),
          m_pControllerManager(controllerManager),
          m_pController(controller),
          m_pDlgControllerLearning(nullptr),
          m_pInputTableModel(nullptr),
          m_pInputProxyModel(nullptr),
          m_pOutputTableModel(nullptr),
          m_pOutputProxyModel(nullptr),
          m_bDirty(false) {
    m_ui.setupUi(this);
    // Create text color for the file and wiki links
    createLinkColor();

    initTableView(m_ui.m_pInputMappingTableView);
    initTableView(m_ui.m_pOutputMappingTableView);

    std::shared_ptr<LegacyControllerMapping> pMapping = m_pController->cloneMapping();
    slotShowMapping(pMapping);

    m_ui.labelDeviceName->setText(m_pController->getName());
    QString category = m_pController->getCategory();
    if (!category.isEmpty()) {
        m_ui.labelDeviceCategory->setText(category);
    } else {
        m_ui.labelDeviceCategory->hide();
    }

    m_ui.groupBoxWarning->hide();
    m_ui.labelWarning->setText(tr(
            "<font color='#BB0000'><b>If you use this mapping your controller "
            "may not work correctly. "
            "Please select another mapping or disable the "
            "controller.</b></font><br><br>"
            "This mapping was designed for a newer Mixxx Controller Engine "
            "and cannot be used on your current Mixxx installation.<br>"
            "Your Mixxx installation has Controller Engine version %1. "
            "This mapping requires a Controller Engine version >= %2.<br><br>"
            "For more information visit the wiki page on "
            "<a "
            "href='https://mixxx.org/wiki/doku.php/"
            "controller_engine_versions'>Controller Engine Versions</a>.")
                                       .arg("2", "1"));
    QIcon icon = style()->standardIcon(QStyle::SP_MessageBoxWarning);
    m_ui.labelWarningIcon->setPixmap(icon.pixmap(50));

    // When the user picks a mapping, load it.
    connect(m_ui.comboBoxMapping,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &DlgPrefController::slotMappingSelected);

    // When the user toggles the Enabled checkbox, mark as dirty
    connect(m_ui.chkEnabledDevice, &QCheckBox::clicked, this, [this] { setDirty(true); });

    // Connect our signals to controller manager.
    connect(this,
            &DlgPrefController::applyMapping,
            m_pControllerManager.get(),
            &ControllerManager::slotApplyMapping);

    // Open script file links
    connect(m_ui.labelLoadedMappingScriptFileLinks,
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
    if (isDirty()) {
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

    if (!m_pMapping) {
        m_pMapping = std::shared_ptr<LegacyControllerMapping>(new LegacyMidiControllerMapping());
        emit applyMapping(m_pController, m_pMapping, true);
    }

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
            &DlgPrefController::slotStopLearning);
}

void DlgPrefController::slotStopLearning() {
    VERIFY_OR_DEBUG_ASSERT(m_pMapping) {
        emit mappingEnded();
        return;
    }

    applyMappingChanges();
    if (m_pMapping->filePath().isEmpty()) {
        // This mapping was created when the learning wizard was started
        if (m_pMapping->isDirty()) {
            QString mappingName = askForMappingName();
            QString mappingPath = mappingNameToPath(m_pUserDir, mappingName);
            m_pMapping->setName(mappingName);
            if (m_pMapping->saveMapping(mappingPath)) {
                qDebug() << "Mapping saved as" << mappingPath;
                m_pMapping->setFilePath(mappingPath);
                m_pMapping->setDirty(false);
                emit applyMapping(m_pController, m_pMapping, true);
                enumerateMappings(mappingPath);
            } else {
                qDebug() << "Failed to save mapping as" << mappingPath;
                // Discard the new mapping and disable the controller
                m_pMapping.reset();
                emit applyMapping(m_pController, m_pMapping, false);
            }
        } else {
            // No changes made to the new mapping, discard it and disable the
            // controller
            m_pMapping.reset();
            emit applyMapping(m_pController, m_pMapping, false);
        }
    }

    emit mappingEnded();
}

void DlgPrefController::midiInputMappingsLearned(
        const MidiInputMappings& mappings) {
    // This is just a shortcut since doing a round-trip from Learning ->
    // Controller -> slotMappingLoaded -> setMapping is too heavyweight.
    if (m_pInputTableModel != nullptr) {
        m_pInputTableModel->addMappings(mappings);
    }
}

QString DlgPrefController::mappingShortName(
        const std::shared_ptr<LegacyControllerMapping> pMapping) const {
    QString mappingName = tr("None");
    if (pMapping) {
        QString name = pMapping->name();
        QString author = pMapping->author();
        if (name.length() > 0 && author.length() > 0) {
            mappingName = tr("%1 by %2").arg(pMapping->name(), pMapping->author());
        } else if (name.length() > 0) {
            mappingName = name;
        } else if (pMapping->filePath().length() > 0) {
            QFileInfo file(pMapping->filePath());
            mappingName = file.baseName();
        }
    }
    return mappingName;
}

QString DlgPrefController::mappingName(
        const std::shared_ptr<LegacyControllerMapping> pMapping) const {
    if (pMapping) {
        QString name = pMapping->name();
        if (name.length() > 0) {
            return name;
        }
    }
    return tr("No Name");
}

QString DlgPrefController::mappingDescription(
        const std::shared_ptr<LegacyControllerMapping> pMapping) const {
    if (pMapping) {
        QString description = pMapping->description();
        if (description.length() > 0) {
            return description;
        }
    }
    return tr("No Description");
}

QString DlgPrefController::mappingAuthor(
        const std::shared_ptr<LegacyControllerMapping> pMapping) const {
    if (pMapping) {
        QString author = pMapping->author();
        if (author.length() > 0) {
            return author;
        }
    }
    return tr("No Author");
}

QString DlgPrefController::mappingSupportLinks(
        const std::shared_ptr<LegacyControllerMapping> pMapping) const {
    if (!pMapping) {
        return QString();
    }

    QStringList linkList;

    QString forumLink = pMapping->forumlink();
    if (!forumLink.isEmpty()) {
        linkList << coloredLinkString(
                m_pLinkColor,
                "Mixxx Forums",
                forumLink);
    }

    QString wikiLink = pMapping->wikilink();
    if (!wikiLink.isEmpty()) {
        linkList << coloredLinkString(
                m_pLinkColor,
                "Mixxx Wiki",
                wikiLink);
    }

    QString manualLink = pMapping->manualLink();
    if (!manualLink.isEmpty()) {
        linkList << coloredLinkString(
                m_pLinkColor,
                "Mixxx Manual",
                manualLink);
    }

    // There is always at least one support link.
    // TODO(rryan): This is a horrible general support link for MIDI!
    linkList << coloredLinkString(
            m_pLinkColor,
            tr("Troubleshooting"),
            MIXXX_WIKI_MIDI_SCRIPTING_URL);

    return QString(linkList.join("&nbsp;&nbsp;"));
}

QString DlgPrefController::mappingFileLinks(
        const std::shared_ptr<LegacyControllerMapping> pMapping) const {
    if (!pMapping) {
        return QString();
    }

    const QString builtinFileSuffix = QStringLiteral(" (") + tr("built-in") + QStringLiteral(")");
    QString systemMappingPath = resourceMappingsPath(m_pConfig);
    QStringList linkList;
    QString xmlFileLink = coloredLinkString(
            m_pLinkColor,
            QFileInfo(pMapping->filePath()).fileName(),
            pMapping->filePath());
    if (pMapping->filePath().startsWith(systemMappingPath)) {
        xmlFileLink += builtinFileSuffix;
    }
    linkList << xmlFileLink;

    for (const auto& script : pMapping->getScriptFiles()) {
        QString scriptFileLink = coloredLinkString(
                m_pLinkColor,
                script.name,
                script.file.absoluteFilePath());
        if (!script.file.exists()) {
            scriptFileLink +=
                    QStringLiteral(" (") + tr("missing") + QStringLiteral(")");
        } else if (script.file.absoluteFilePath().startsWith(
                           systemMappingPath)) {
            scriptFileLink += builtinFileSuffix;
        }

        linkList << scriptFileLink;
    }
    return linkList.join("<br/>");
}

void DlgPrefController::enumerateMappings(const QString& selectedMappingPath) {
    m_ui.comboBoxMapping->blockSignals(true);
    m_ui.comboBoxMapping->clear();

    // qDebug() << "Enumerating mappings for controller" << m_pController->getName();

    // Check the text color of the palette for whether to use dark or light icons
    QDir iconsPath;
    if (!Color::isDimColor(palette().text().color())) {
        iconsPath.setPath(":/images/preferences/light/");
    } else {
        iconsPath.setPath(":/images/preferences/dark/");
    }

    // Insert a dummy item at the top to try to make it less confusing.
    // (We don't want the first found file showing up as the default item when a
    // user has their controller plugged in)
    QIcon noMappingIcon(iconsPath.filePath("ic_none.svg"));
    m_ui.comboBoxMapping->addItem(noMappingIcon, "No Mapping");

    MappingInfo match;
    // Enumerate user mappings
    QIcon userMappingIcon(iconsPath.filePath("ic_custom.svg"));

    // Reload user mappings to detect added, changed or removed mappings
    m_pControllerManager->getMainThreadUserMappingEnumerator()->loadSupportedMappings();

    MappingInfo userMappingsMatch = enumerateMappingsFromEnumerator(
            m_pControllerManager->getMainThreadUserMappingEnumerator(),
            userMappingIcon);
    if (userMappingsMatch.isValid()) {
        match = userMappingsMatch;
    }

    // Insert a separator between user mappings (+ dummy item) and system mappings
    m_ui.comboBoxMapping->insertSeparator(m_ui.comboBoxMapping->count());

    // Enumerate system mappings
    QIcon systemMappingIcon(iconsPath.filePath("ic_mixxx_symbolic.svg"));
    MappingInfo systemMappingsMatch = enumerateMappingsFromEnumerator(
            m_pControllerManager->getMainThreadSystemMappingEnumerator(),
            systemMappingIcon);
    if (systemMappingsMatch.isValid()) {
        match = systemMappingsMatch;
    }

    // Preselect configured or matching mapping
    int index = -1;
    if (!selectedMappingPath.isEmpty()) {
        index = m_ui.comboBoxMapping->findData(selectedMappingPath);
    } else if (match.isValid()) {
        index = m_ui.comboBoxMapping->findText(match.getName());
    }
    if (index == -1) {
        m_ui.chkEnabledDevice->setEnabled(false);
    } else {
        m_ui.comboBoxMapping->setCurrentIndex(index);
        m_ui.chkEnabledDevice->setEnabled(true);
    }
    m_ui.comboBoxMapping->blockSignals(false);
    slotMappingSelected(m_ui.comboBoxMapping->currentIndex());
}

MappingInfo DlgPrefController::enumerateMappingsFromEnumerator(
        QSharedPointer<MappingInfoEnumerator> pMappingEnumerator, const QIcon& icon) {
    MappingInfo match;

    // Check if enumerator is ready. Should be rare that it isn't. We will
    // re-enumerate on the next open of the preferences.
    if (!pMappingEnumerator.isNull()) {
        // Get a list of mappings in alphabetical order
        QList<MappingInfo> systemMappings =
                pMappingEnumerator->getMappingsByExtension(
                        m_pController->mappingExtension());

        for (const MappingInfo& mapping : systemMappings) {
            m_ui.comboBoxMapping->addItem(
                    icon, mapping.getName(), mapping.getPath());
            if (m_pController->matchMapping(mapping)) {
                match = mapping;
            }
        }
    }

    return match;
}

void DlgPrefController::slotUpdate() {
    // Check if the controller is open.
    bool deviceOpen = m_pController->isOpen();
    // Check/uncheck the "Enabled" box
    m_ui.chkEnabledDevice->setChecked(deviceOpen);

    enumerateMappings(m_pControllerManager->getConfiguredMappingFileForDevice(
            m_pController->getName()));

    // If the controller is not mappable, disable the input and output mapping
    // sections and the learning wizard button.
    bool isMappable = m_pController->isMappable();
    m_ui.btnLearningWizard->setEnabled(isMappable);
    m_ui.inputMappingsTab->setEnabled(isMappable);
    m_ui.outputMappingsTab->setEnabled(isMappable);
}

void DlgPrefController::slotResetToDefaults() {
    m_ui.chkEnabledDevice->setChecked(false);
    enumerateMappings(QString());
    slotMappingSelected(m_ui.comboBoxMapping->currentIndex());
}

void DlgPrefController::applyMappingChanges() {
    if (m_pInputTableModel) {
        m_pInputTableModel->apply();
    }

    if (m_pOutputTableModel) {
        m_pOutputTableModel->apply();
    }
}

void DlgPrefController::slotApply() {
    applyMappingChanges();

    // If no changes were made, do nothing
    if (!(isDirty() || (m_pMapping && m_pMapping->isDirty()))) {
        return;
    }

    bool bEnabled = false;
    if (m_pMapping) {
        bEnabled = m_ui.chkEnabledDevice->isChecked();

        if (m_pMapping->isDirty()) {
            saveMapping();
        }
    }
    m_ui.chkEnabledDevice->setChecked(bEnabled);

    // The shouldn't be dirty at this point because we already tried to save
    // it. If that failed, don't apply the mapping.
    if (m_pMapping && m_pMapping->isDirty()) {
        return;
    }

    QString mappingPath = mappingPathFromIndex(m_ui.comboBoxMapping->currentIndex());
    m_pMapping = LegacyControllerMappingFileHandler::loadMapping(
            mappingPath, QDir(resourceMappingsPath(m_pConfig)));

    // Load the resulting mapping (which has been mutated by the input/output
    // table models). The controller clones the mapping so we aren't touching
    // the same mapping.
    emit applyMapping(m_pController, m_pMapping, bEnabled);

    // Mark the dialog as not dirty
    setDirty(false);
}

QUrl DlgPrefController::helpUrl() const {
    return QUrl(MIXXX_MANUAL_CONTROLLERS_URL);
}

QString DlgPrefController::mappingPathFromIndex(int index) const {
    if (index == 0) {
        // "No Mapping" item
        return QString();
    }

    return m_ui.comboBoxMapping->itemData(index).toString();
}

void DlgPrefController::slotMappingSelected(int chosenIndex) {
    QString mappingPath = mappingPathFromIndex(chosenIndex);
    if (mappingPath.isEmpty()) {
        // User picked "No Mapping" item
        m_ui.chkEnabledDevice->setEnabled(false);

        if (m_ui.chkEnabledDevice->isChecked()) {
            m_ui.chkEnabledDevice->setChecked(false);
            setDirty(true);
        }
    } else {
        // User picked a mapping
        m_ui.chkEnabledDevice->setEnabled(true);

        if (!m_ui.chkEnabledDevice->isChecked()) {
            m_ui.chkEnabledDevice->setChecked(true);
            setDirty(true);
        }
    }

    // Check if the mapping is different from the configured mapping
    if (m_pControllerManager->getConfiguredMappingFileForDevice(
                m_pController->getName()) != mappingPath) {
        setDirty(true);
    }

    applyMappingChanges();
    if (m_pMapping && m_pMapping->isDirty()) {
        if (QMessageBox::question(this,
                    tr("Mapping has been edited"),
                    tr("Do you want to save the changes?")) ==
                QMessageBox::Yes) {
            saveMapping();
        }
    }

    std::shared_ptr<LegacyControllerMapping> pMapping =
            LegacyControllerMappingFileHandler::loadMapping(
                    mappingPath, QDir(resourceMappingsPath(m_pConfig)));

    if (pMapping) {
        DEBUG_ASSERT(!pMapping->isDirty());
    }

    slotShowMapping(pMapping);
}

void DlgPrefController::saveMapping() {
    VERIFY_OR_DEBUG_ASSERT(m_pMapping) {
        return;
    }

    if (!m_pMapping->isDirty()) {
        qDebug() << "Mapping is not dirty, no need to save it.";
        return;
    }

    QString oldFilePath = m_pMapping->filePath();
    QString newFilePath;
    QFileInfo fileInfo(oldFilePath);
    QString mappingName = m_pMapping->name();

    bool isUserMapping = fileInfo.absoluteDir().absolutePath().append("/") == m_pUserDir;
    bool saveAsNew = true;
    if (m_pOverwriteMappings.contains(oldFilePath) &&
            m_pOverwriteMappings.value(oldFilePath) == true) {
        saveAsNew = false;
    }

    // If this is a user mapping, ask whether to overwrite or save with new name.
    // Optionally, tick checkbox to always overwrite this mapping in the current session.
    if (isUserMapping && saveAsNew) {
        QString overwriteTitle = tr("Mapping already exists.");
        QString overwriteLabel = tr(
                "<b>%1</b> already exists in user mapping folder.<br>"
                "Overwrite or save with a new name?");
        QString overwriteCheckLabel = tr("Always overwrite during this session");

        QMessageBox overwriteMsgBox;
        overwriteMsgBox.setIcon(QMessageBox::Question);
        overwriteMsgBox.setWindowTitle(overwriteTitle);
        overwriteMsgBox.setText(overwriteLabel.arg(mappingName));
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
                m_pOverwriteMappings.insert(m_pMapping->filePath(), true);
            }
        } else if (overwriteMsgBox.close()) {
            return;
        }
    }

    // Ask for a mapping name when
    // * initially saving a modified Mixxx mapping to the user folder
    // * saving a user mapping with a new name.
    // The name will be used as display name and file name.
    if (!saveAsNew) {
        newFilePath = oldFilePath;
    } else {
        mappingName = askForMappingName(mappingName);
        newFilePath = mappingNameToPath(m_pUserDir, mappingName);
        m_pMapping->setName(mappingName);
        qDebug() << "Mapping renamed to" << m_pMapping->name();
    }

    if (!m_pMapping->saveMapping(newFilePath)) {
        qDebug() << "Failed to save mapping as" << newFilePath;
        return;
    }
    qDebug() << "Mapping saved as" << newFilePath;

    m_pMapping->setFilePath(newFilePath);
    m_pMapping->setDirty(false);

    enumerateMappings(m_pMapping->filePath());
}

QString DlgPrefController::askForMappingName(const QString& prefilledName) const {
    QString saveMappingTitle = tr("Save user mapping");
    QString saveMappingLabel = tr("Enter the name for saving the mapping to the user folder.");
    QString savingFailedTitle = tr("Saving mapping failed");
    QString invalidNameLabel =
            tr("A mapping cannot have a blank name and may not contain "
               "special characters.");
    QString fileExistsLabel = tr("A mapping file with that name already exists.");
    // Only allow the name to contain letters, numbers, whitespaces and _-+()/
    const QRegExp rxRemove = QRegExp("[^[(a-zA-Z0-9\\_\\-\\+\\(\\)\\/|\\s]");

    // Choose a new file (base) name
    bool validMappingName = false;
    QString mappingName = prefilledName;
    while (!validMappingName) {
        QString userDir = m_pUserDir;
        bool ok = false;
        mappingName = QInputDialog::getText(nullptr,
                saveMappingTitle,
                saveMappingLabel,
                QLineEdit::Normal,
                mappingName,
                &ok)
                              .remove(rxRemove)
                              .trimmed();
        if (!ok) {
            continue;
        }
        if (mappingName.isEmpty()) {
            QMessageBox::warning(nullptr,
                    savingFailedTitle,
                    invalidNameLabel);
            continue;
        }
        // While / is allowed for the display name we can't use it for the file name.
        QString newFilePath = mappingNameToPath(userDir, mappingName);
        if (QFile::exists(newFilePath)) {
            QMessageBox::warning(nullptr,
                    savingFailedTitle,
                    fileExistsLabel);
            continue;
        }
        validMappingName = true;
    }
    return mappingName;
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

void DlgPrefController::slotShowMapping(std::shared_ptr<LegacyControllerMapping> pMapping) {
    m_ui.labelLoadedMapping->setText(mappingName(pMapping));
    m_ui.labelLoadedMappingDescription->setText(mappingDescription(pMapping));
    m_ui.labelLoadedMappingAuthor->setText(mappingAuthor(pMapping));
    m_ui.labelLoadedMappingSupportLinks->setText(mappingSupportLinks(pMapping));
    m_ui.labelLoadedMappingScriptFileLinks->setText(mappingFileLinks(pMapping));

    // We mutate this mapping so keep a reference to it while we are using it.
    // TODO(rryan): Clone it? Technically a waste since nothing else uses this
    // copy but if someone did they might not expect it to change.
    m_pMapping = pMapping;

    ControllerInputMappingTableModel* pInputModel =
            new ControllerInputMappingTableModel(this);
    pInputModel->setMapping(pMapping);

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
    pOutputModel->setMapping(pMapping);

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
