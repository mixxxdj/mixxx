#include "controllers/dlgprefcontroller.h"

#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QInputDialog>
#include <QKeyEvent>

#include "controllers/controller.h"
#include "controllers/controllerinputmappingtablemodel.h"
#include "controllers/controllerlearningeventfilter.h"
#include "controllers/controllermanager.h"
#include "controllers/controllermappinginfoenumerator.h"
#include "controllers/controlleroutputmappingtablemodel.h"
#include "controllers/controlpickermenu.h"
#ifdef MIXXX_USE_QML
#include "controllers/controllerscreenpreview.h"
#endif
#include "controllers/defs_controllers.h"
#include "controllers/dlgcontrollerlearning.h"
#ifdef __HID__
#include "controllers/hid/hidcontroller.h"
#endif
#include "controllers/midi/legacymidicontrollermapping.h"
#include "controllers/midi/midicontroller.h"
#include "controllers/scripting/legacy/controllerscriptenginelegacy.h"
#include "defs_urls.h"
#include "moc_dlgprefcontroller.cpp"
#include "preferences/usersettings.h"
#include "util/cmdlineargs.h"
#include "util/desktophelper.h"
#include "util/parented_ptr.h"
#include "util/string.h"
#include "widget/wcollapsiblegroupbox.h"

namespace {

constexpr int kNoMappingIndex = 0; // "No Mapping" is always at the first position;

const QString kMappingExt(".midi.xml");

QString mappingNameToPath(const QString& directory, const QString& mappingName) {
    // While / is allowed for the display name we can't use it for the file name.
    QString fileName = QString(mappingName).replace(QChar('/'), QChar('-'));
    return directory + fileName + kMappingExt;
}

const QString kBuiltinFileSuffix =
        QStringLiteral(" (") + QObject::tr("built-in") + QStringLiteral(")");

/// @brief  Format a controller file to display attributes (system, missing) in the UI.
/// @return The formatted string.
QString formatFilePath(UserSettingsPointer pConfig,
        QColor linkColor,
        const QString& name,
        const QFileInfo& file) {
    QString systemMappingPath = resourceMappingsPath(pConfig);
    QString scriptFileLink = coloredLinkString(
            linkColor,
            name,
            file.absoluteFilePath());
    if (!file.exists()) {
        scriptFileLink +=
                QStringLiteral(" (") + QObject::tr("missing") + QStringLiteral(")");
    } else if (file.absoluteFilePath().startsWith(systemMappingPath)) {
        scriptFileLink += kBuiltinFileSuffix;
    }
    return scriptFileLink;
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
          m_GuiInitialized(false),
          m_bDirty(false),
          m_inputMappingsTabIndex(-1),
          m_outputMappingsTabIndex(-1),
          m_settingsTabIndex(-1),
          m_screensTabIndex(-1) {
    m_ui.setupUi(this);
    // Create text color for the file and wiki links
    createLinkColor();

    m_pControlPickerMenu = make_parented<ControlPickerMenu>(this);

    initTableView(m_ui.midiInputMappingTableView);
    initTableView(m_ui.midiOutputMappingTableView);

    std::shared_ptr<LegacyControllerMapping> pMapping = m_pController->getMapping();
    showMapping(pMapping);

    m_ui.labelDeviceName->setText(m_pController->getName());

    m_ui.labelPhysicalInterfaceValue->setText(
            Controller::physicalTransport2String(
                    m_pController->getPhysicalTransportProtocol()));

    const QString dataHandlingProtocol =
            [protocol = m_pController->getDataRepresentationProtocol()] {
                switch (protocol) {
                case DataRepresentationProtocol::USB_BULK_TRANSFER:
                    return QStringLiteral("USB Bulk");
                case DataRepresentationProtocol::HID:
                    return QStringLiteral("HID");
                case DataRepresentationProtocol::MIDI:
                    return QStringLiteral("MIDI");
                }
                return QString();
            }();
    m_ui.labelDataHandlingProtocolValue->setText(dataHandlingProtocol);

    auto formatHex = [](unsigned short value) {
        return QString::number(value, 16).toUpper().rightJustified(4, '0');
    };

    auto vendorString = m_pController->getVendorString();
    if (!vendorString.isEmpty()) {
        m_ui.labelVendorValue->setText(vendorString);
        m_ui.labelVendor->setVisible(true);
        m_ui.labelVendorValue->setVisible(true);
    } else {
        m_ui.labelVendor->setVisible(false);
        m_ui.labelVendorValue->setVisible(false);
    }

    // Product-String is always available
    m_ui.labelProductValue->setText(m_pController->getProductString());

    if (auto vid = m_pController->getVendorId()) {
        m_ui.labelVidValue->setText(formatHex(*vid));
        m_ui.labelVid->setVisible(true);
        m_ui.labelVidValue->setVisible(true);
    } else {
        m_ui.labelVid->setVisible(false);
        m_ui.labelVidValue->setVisible(false);
    }

    if (auto pid = m_pController->getProductId()) {
        m_ui.labelPidValue->setText(formatHex(*pid));
        m_ui.labelPid->setVisible(true);
        m_ui.labelPidValue->setVisible(true);
    } else {
        m_ui.labelPid->setVisible(false);
        m_ui.labelPidValue->setVisible(false);
    }

    auto serialNo = m_pController->getSerialNumber();
    if (!serialNo.isEmpty()) {
        m_ui.labelSerialNumberValue->setText(serialNo);
        m_ui.labelSerialNumber->setVisible(true);
        m_ui.labelSerialNumberValue->setVisible(true);
    } else {
        m_ui.labelSerialNumber->setVisible(false);
        m_ui.labelSerialNumberValue->setVisible(false);
    }

    auto interfaceNumber = m_pController->getUsbInterfaceNumber();
    if (m_pController->getPhysicalTransportProtocol() == PhysicalTransportProtocol::USB &&
            interfaceNumber) {
        m_ui.labelUsbInterfaceNumberValue->setText(QString::number(*interfaceNumber));
        m_ui.labelUsbInterfaceNumber->setVisible(true);
        m_ui.labelUsbInterfaceNumberValue->setVisible(true);
    } else {
        // Not a USB device -> USB interface number is not applicable
        m_ui.labelUsbInterfaceNumber->setVisible(false);
        m_ui.labelUsbInterfaceNumberValue->setVisible(false);
    }

#ifdef __HID__
    // Display HID UsagePage and Usage if the controller is an HidController
    if (auto* hidController = dynamic_cast<HidController*>(m_pController)) {
        m_ui.labelHidUsagePageValue->setText(QStringLiteral("%1 (%2)")
                        .arg(formatHex(hidController->getUsagePage()),
                                hidController->getUsagePageDescription()));

        m_ui.labelHidUsageValue->setText(QStringLiteral("%1 (%2)")
                        .arg(formatHex(hidController->getUsage()),
                                hidController->getUsageDescription()));
        m_ui.labelHidUsagePage->setVisible(true);
        m_ui.labelHidUsagePageValue->setVisible(true);
        m_ui.labelHidUsage->setVisible(true);
        m_ui.labelHidUsageValue->setVisible(true);
    } else
#endif
    {
        m_ui.labelHidUsagePage->setVisible(false);
        m_ui.labelHidUsagePageValue->setVisible(false);
        m_ui.labelHidUsage->setVisible(false);
        m_ui.labelHidUsageValue->setVisible(false);
    }

    m_ui.groupBoxWarning->hide();
    m_ui.labelWarning->setTextFormat(Qt::RichText);
    m_ui.labelWarning->setTextInteractionFlags(Qt::TextBrowserInteraction |
            Qt::LinksAccessibleByMouse | Qt::TextSelectableByMouse);
    m_ui.labelWarning->setOpenExternalLinks(true);
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
            &ControllerManager::slotApplyMapping,
            Qt::BlockingQueuedConnection);
    // Wait until the mapping has been cloned in the controller thread
    // and we can continue to edit our copy

    // Update GUI
    connect(m_pControllerManager.get(),
            &ControllerManager::mappingApplied,
            this,
            &DlgPrefController::enableWizardAndIOTabs);
#ifdef MIXXX_USE_QML
    if (CmdlineArgs::Instance()
                    .getControllerPreviewScreens()) {
        connect(m_pController,
                &Controller::engineStarted,
                this,
                &DlgPrefController::slotShowPreviewScreens);
        connect(m_pController,
                &Controller::engineStopped,
                this,
                &DlgPrefController::slotClearPreviewScreens);
    }
#endif

    // Open script file links
    connect(m_ui.labelMappingScriptFileLinks,
            &QLabel::linkActivated,
            [](const QString& path) {
                mixxx::DesktopHelper::openUrl(QUrl::fromLocalFile(path));
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

    connect(m_ui.inputControlSearch,
            &QLineEdit::returnPressed,
            this,
            &DlgPrefController::slotInputControlSearch);
    connect(m_ui.inputControlSearchBtn,
            &QPushButton::clicked,
            this,
            &DlgPrefController::slotInputControlSearch);
    connect(m_ui.outputControlSearch,
            &QLineEdit::returnPressed,
            this,
            &DlgPrefController::slotOutputControlSearch);
    connect(m_ui.outputControlSearchBtn,
            &QPushButton::clicked,
            this,
            &DlgPrefController::slotOutputControlSearch);

    // Store the index of the input and output mappings tabs
    m_inputMappingsTabIndex = m_ui.controllerTabs->indexOf(m_ui.inputMappingsTab);
    m_outputMappingsTabIndex = m_ui.controllerTabs->indexOf(m_ui.outputMappingsTab);
    m_settingsTabIndex = m_ui.controllerTabs->indexOf(m_ui.settingsTab);
    m_screensTabIndex = m_ui.controllerTabs->indexOf(m_ui.screensTab);

#ifndef MIXXX_USE_QML
    // Remove the screens tab
    m_ui.controllerTabs->removeTab(m_screensTabIndex);
    // Just to be save
    m_screensTabIndex = -1;
#endif
}

DlgPrefController::~DlgPrefController() {
}

void DlgPrefController::slotRecreateControlPickerMenu() {
    m_pControlPickerMenu = make_parented<ControlPickerMenu>(this);
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
        m_pMapping = std::make_shared<LegacyMidiControllerMapping>();
        emit applyMapping(m_pController, m_pMapping, true);
        // shortcut for creating and assigning required I/O table models
        showMapping(m_pMapping);
    }

    // Note that DlgControllerLearning is set to delete itself on close using
    // the Qt::WA_DeleteOnClose attribute (so this "new" doesn't leak memory)
    m_pDlgControllerLearning =
            new DlgControllerLearning(this, m_pController, m_pControlPickerMenu);
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

    // This will show() -> slotUpdate() -> enumerateMappings() etc.
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
                "Mixxx&nbsp;Forums",
                forumLink);
    }

    QString wikiLink = pMapping->wikilink();
    if (!wikiLink.isEmpty()) {
        linkList << coloredLinkString(
                m_pLinkColor,
                "Mixxx&nbsp;Wiki",
                wikiLink);
    }

    QString manualLink = pMapping->manualLink();
    if (!manualLink.isEmpty()) {
        linkList << coloredLinkString(
                m_pLinkColor,
                "Mixxx&nbsp;Manual",
                manualLink);
    }

    // There is always at least one support link.
    // TODO(rryan): This is a horrible general support link for MIDI!
    linkList << coloredLinkString(
            m_pLinkColor,
            tr("Troubleshooting"),
            MIXXX_WIKI_MIDI_SCRIPTING_URL);
    // Without &nbsp; would be rendered as regular whitespace (thin, &ensp;)
    return QString(linkList.join("&emsp;&nbsp;"));
}

QString DlgPrefController::mappingFileLinks(
        const std::shared_ptr<LegacyControllerMapping> pMapping) const {
    if (!pMapping) {
        return QString();
    }

    QStringList linkList;
    linkList.append(formatFilePath(m_pConfig,
            m_pLinkColor,
            QFileInfo(pMapping->filePath()).fileName(),
            QFileInfo(pMapping->filePath())));
    for (const auto& script : pMapping->getScriptFiles()) {
        linkList.append(formatFilePath(m_pConfig,
                m_pLinkColor,
                script.name,
                QFileInfo(script.file.absoluteFilePath())));
    }
#ifdef MIXXX_USE_QML
    for (const auto& qmlLibrary : pMapping->getModules()) {
        auto fileInfo = QFileInfo(qmlLibrary.dirinfo.absoluteFilePath());
        linkList.append(formatFilePath(m_pConfig, m_pLinkColor, fileInfo.fileName(), fileInfo));
    }
#endif
    return linkList.join("<br/>");
}

void DlgPrefController::enumerateMappings(const QString& selectedMappingPath) {
    m_ui.comboBoxMapping->blockSignals(true);
    QString currentMappingFilePath = mappingFilePathFromIndex(m_ui.comboBoxMapping->currentIndex());
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
    int index = kNoMappingIndex;
    if (!selectedMappingPath.isEmpty()) {
        index = m_ui.comboBoxMapping->findData(selectedMappingPath);
    } else if (match.isValid()) {
        index = m_ui.comboBoxMapping->findText(match.getName());
    }
    QString newMappingFilePath;
    if (index <= kNoMappingIndex) { // findData() returns -1 for not found
        index = kNoMappingIndex;
        m_ui.chkEnabledDevice->setEnabled(false);
    } else {
        newMappingFilePath = mappingFilePathFromIndex(index);
        m_ui.comboBoxMapping->setCurrentIndex(index);
    }
    m_ui.comboBoxMapping->blockSignals(false);
    if (newMappingFilePath != currentMappingFilePath) {
        slotMappingSelected(index);
    }
}

MappingInfo DlgPrefController::enumerateMappingsFromEnumerator(
        QSharedPointer<MappingInfoEnumerator> pMappingEnumerator, const QIcon& icon) {
    MappingInfo match;

    // Check if enumerator is ready. Should be rare that it isn't. We will
    // re-enumerate on the next open of the preferences.
    if (!pMappingEnumerator.isNull()) {
        // Get a list of mappings in alphabetical order
        const QList<MappingInfo> systemMappings =
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
    enumerateMappings(m_pControllerManager->getConfiguredMappingFileForDevice(
            m_pController->getName()));
    // Note: this is called by closeDlg() when MIDI learning starts
    // "No Mapping" is selected but we have a mapping for learning
    if (m_ui.comboBoxMapping->currentIndex() > kNoMappingIndex || !m_pMapping) {
        // Force updating the controller settings
        slotMappingSelected(m_ui.comboBoxMapping->currentIndex());
    }

    // enumerateMappings() calls slotMappingSelected() which will tick the 'Enabled'
    // checkbox if there is a valid mapping saved in the mixxx.cfg file.
    // However, the checkbox should only be checked if the device is currently enabled.
    // TODO fix in slotMappingSelected()?
    m_ui.chkEnabledDevice->setChecked(m_pController->isOpen());

    // If the controller is not mappable, disable the input and output mapping
    // sections and the learning wizard button.
    enableWizardAndIOTabs(m_pController->isMappable() && m_pController->isOpen());

    // When slotUpdate() is run for the first time, this bool keeps slotPresetSelected()
    // from setting a false-postive 'dirty' flag when updating the fresh GUI.
    m_GuiInitialized = true;
}

void DlgPrefController::slotHide() {
    slotUpdate();
}

void DlgPrefController::slotResetToDefaults() {
    if (m_pMapping) {
        m_pMapping->resetSettings();
    }
    enumerateMappings(QString());
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
            if (saveMapping()) {
                // We might have saved the previous mapping with a new name,
                // so update the mapping combobox.
                enumerateMappings(m_pMapping->filePath());
            }
        }
    }
    m_ui.chkEnabledDevice->setChecked(bEnabled);

    // The shouldn't be dirty at this point because we already tried to save
    // it. If that failed, don't apply the mapping.
    if (m_pMapping && m_pMapping->isDirty()) {
        return;
    }

    // If there is currently a mapping loaded, we save the new settings for it.
    // Note that `m_pMapping`, `mappingFileInfo` and the setting on the screen
    // will always match as the settings displayed are updated depending of the
    // currently selected mapping in `slotMappingSelected`
    if (m_pMapping) {
        m_pMapping->saveSettings(m_pConfig, m_pController->getName());
    }

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

void DlgPrefController::keyPressEvent(QKeyEvent* pEvent) {
    // Filter Return/Enter keypress in control search boxes so it's not
    // forwarded to the DlgPreferences button box' default button (Apply)
    if ((pEvent->key() == Qt::Key_Return || pEvent->key() == Qt::Key_Enter) &&
            (m_ui.inputControlSearch->hasFocus() || m_ui.inputControlSearch->hasFocus())) {
        return;
    }
    QWidget::keyPressEvent(pEvent);
}

void DlgPrefController::enableWizardAndIOTabs(bool enable) {
    // We always enable the Wizard button if this is a MIDI controller so we can
    // create a new mapping from scratch with 'No Mapping'
    const auto* midiController = qobject_cast<MidiController*>(m_pController);
    m_ui.btnLearningWizard->setEnabled(midiController != nullptr);
    m_ui.inputMappingsTab->setEnabled(enable);
    m_ui.outputMappingsTab->setEnabled(enable);
}

QString DlgPrefController::mappingFilePathFromIndex(int index) const {
    if (index <= kNoMappingIndex) {
        return QString();
    }

    return m_ui.comboBoxMapping->itemData(index).toString();
}

void DlgPrefController::slotMappingSelected(int chosenIndex) {
    // Note that this is also called by slotUpdate() after MIDI learning finished
    // and we may have pending changes. Force-reloading the mapping from file
    // would wipe those so we need to make sure to return before
    // LegacyControllerMappingFileHandler::loadMapping()
    QString mappingFilePath = mappingFilePathFromIndex(chosenIndex);
    if (mappingFilePath.isEmpty()) {
        // User picked "No Mapping" item
        m_ui.chkEnabledDevice->setEnabled(false);

        if (m_ui.chkEnabledDevice->isChecked()) {
            m_ui.chkEnabledDevice->setChecked(false);
            if (m_GuiInitialized) {
                setDirty(true);
            }
            enableWizardAndIOTabs(false);
        }

    } else { // User picked a mapping
        m_ui.chkEnabledDevice->setEnabled(true);

        if (!m_ui.chkEnabledDevice->isChecked()) {
            m_ui.chkEnabledDevice->setChecked(true);
            if (m_GuiInitialized) {
                setDirty(true);
            }
        }
    }

    // Check if the mapping is different from the configured mapping
    if (m_GuiInitialized) {
        if (m_pControllerManager->getConfiguredMappingFileForDevice(
                    m_pController->getName()) != mappingFilePath) {
            setDirty(true);
            m_settingsCollapsedStates.clear();
        } else if (m_pMapping && m_pMapping->isDirty()) {
            // We have pending changes, don't reload the mapping from file!
            // This is called by show()/slotUpdate() after MIDI learning ended
            // and there is no need to update the GUI.
            return;
        }
    }

    applyMappingChanges();
    bool previousMappingSaved = false;
    if (m_pMapping && m_pMapping->isDirty()) {
        if (QMessageBox::question(this,
                    tr("Mapping has been edited"),
                    tr("Do you want to save the changes?")) ==
                QMessageBox::Yes) {
            previousMappingSaved = saveMapping();
        }
    }

    auto mappingFileInfo = QFileInfo(mappingFilePath);
    std::shared_ptr<LegacyControllerMapping> pMapping =
            LegacyControllerMappingFileHandler::loadMapping(
                    mappingFileInfo, QDir(resourceMappingsPath(m_pConfig)));

    if (pMapping) {
        DEBUG_ASSERT(!pMapping->isDirty());
    }

    if (previousMappingSaved) {
        // We might have saved the previous preset with a new name, so update
        // the preset combobox.
        enumerateMappings(mappingFilePath);
    }
    showMapping(pMapping);

    // These tabs are only usable for MIDI controllers
    bool showMidiTabs = m_pController->getDataRepresentationProtocol() ==
                    DataRepresentationProtocol::MIDI &&
            !mappingFilePath.isEmpty();
    m_ui.controllerTabs->setTabVisible(m_inputMappingsTabIndex, showMidiTabs);
    m_ui.controllerTabs->setTabVisible(m_outputMappingsTabIndex, showMidiTabs);
    // Also disable accordingly so hidden tabs can not get keyboard focus
    m_ui.controllerTabs->setTabEnabled(m_inputMappingsTabIndex, showMidiTabs);
    m_ui.controllerTabs->setTabEnabled(m_outputMappingsTabIndex, showMidiTabs);

    // Hide the entire QTabWidget if all tabs are removed
    m_ui.controllerTabs->setVisible(getNumberOfVisibleTabs() > 0);
}

unsigned int DlgPrefController::getNumberOfVisibleTabs() {
    unsigned int visibleTabsCount = 0;
    for (int tabIdx = 0; tabIdx < m_ui.controllerTabs->count(); ++tabIdx) {
        if (m_ui.controllerTabs->isTabVisible(tabIdx)) {
            ++visibleTabsCount;
        }
    }
    return visibleTabsCount;
}

int DlgPrefController::getIndexOfFirstVisibleTab() {
    for (int tabIdx = 0; tabIdx < m_ui.controllerTabs->count(); ++tabIdx) {
        if (m_ui.controllerTabs->isTabVisible(tabIdx)) {
            return tabIdx;
        }
    }
    return -1;
}

bool DlgPrefController::saveMapping() {
    VERIFY_OR_DEBUG_ASSERT(m_pMapping) {
        return false;
    }

    if (!m_pMapping->isDirty()) {
        qDebug() << "Mapping is not dirty, no need to save it.";
        return false;
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
        // QMessageBox handles Esc or pressing the X window button only if there
        // is a button with either RejectRole or NoRole, so let's add one.
        // https://doc.qt.io/qt-6/qmessagebox.html#escapeButton
        QPushButton* pCancel = overwriteMsgBox.addButton(QMessageBox::Cancel);
        // Hide Cancel since we don't really need it (we have the X button),
        // furthermore it'll likely be auto-positioned in between Save and Overwrite
        // which is not optimal (rules for order depend on OS).
        pCancel->hide();
        overwriteMsgBox.setDefaultButton(pSaveAsNew);
        overwriteMsgBox.exec();

        if (overwriteMsgBox.clickedButton() == pOverwrite) {
            saveAsNew = false;
            if (overwriteCheckBox.checkState() == Qt::Checked) {
                m_pOverwriteMappings.insert(m_pMapping->filePath(), true);
            }
        } else if (overwriteMsgBox.clickedButton() != pSaveAsNew) {
            // Dialog was closed without clicking one of our buttons
            return false;
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
        if (mappingName.isEmpty()) {
            // QInputDialog was closed
            qDebug() << "Mapping not saved, new name is empty";
            return false;
        }
        m_pMapping->setName(mappingName);
        qDebug() << "Mapping renamed to" << m_pMapping->name();
    }

    if (!m_pMapping->saveMapping(newFilePath)) {
        qDebug() << "Failed to save mapping as" << newFilePath;
        return false;
    }
    qDebug() << "Mapping saved as" << newFilePath;

    m_pMapping->setFilePath(newFilePath);
    m_pMapping->setDirty(false);

    return true;
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
    static const QRegularExpression rxRemove = QRegularExpression(
            QStringLiteral("[^[(a-zA-Z0-9\\_\\-\\+\\(\\)\\/|\\s]"));

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
            // Return empty string if the dialog was canceled. Callers will deal with this.
            return QString();
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

#ifdef MIXXX_USE_QML
void DlgPrefController::slotShowPreviewScreens(
        const ControllerScriptEngineLegacy* scriptEngine) {
    QLayoutItem* pItem;
    VERIFY_OR_DEBUG_ASSERT(m_ui.screensTab->layout()) {
        return;
    }
    while ((pItem = m_ui.screensTab->layout()->takeAt(0)) != nullptr) {
        delete pItem->widget();
        delete pItem;
    }

    if (!m_pMapping) {
        return;
    }

    if (!scriptEngine) {
        return;
    }

    auto screens = m_pMapping->getInfoScreens();

    for (const LegacyControllerMapping::ScreenInfo& screen : std::as_const(screens)) {
        ControllerScreenPreview* pPreviewScreen =
                new ControllerScreenPreview(m_ui.screensTab, screen);
        m_ui.screensTab->layout()->addWidget(pPreviewScreen);

        connect(scriptEngine,
                &ControllerScriptEngineLegacy::previewRenderedScreen,
                pPreviewScreen,
                &ControllerScreenPreview::updateFrame);
    }
}
#endif

void DlgPrefController::showMapping(std::shared_ptr<LegacyControllerMapping> pMapping) {
    QString name, description, author, supportLinks, scriptFileLinks;

    if (pMapping) {
        name = pMapping->name();
        description = pMapping->description();
        author = pMapping->author();
        supportLinks = mappingSupportLinks(pMapping);
        scriptFileLinks = mappingFileLinks(pMapping);
    }

    m_ui.labelMappingNameValue->setText(name);
    m_ui.labelMappingDescriptionValue->setText(description);
    m_ui.labelMappingAuthorValue->setText(author);
    m_ui.labelMappingSupportLinks->setText(supportLinks);
    m_ui.labelMappingScriptFileLinks->setText(scriptFileLinks);

    // Hide or show labels based on the presence of text
    m_ui.labelMappingName->setVisible(!name.isEmpty());
    m_ui.labelMappingNameValue->setVisible(!name.isEmpty());
    m_ui.labelMappingDescription->setVisible(!description.isEmpty());
    m_ui.labelMappingDescriptionValue->setVisible(!description.isEmpty());
    m_ui.labelMappingAuthor->setVisible(!author.isEmpty());
    m_ui.labelMappingAuthorValue->setVisible(!author.isEmpty());
    m_ui.labelMappingSupport->setVisible(!supportLinks.isEmpty());
    m_ui.labelMappingScriptFiles->setVisible(!scriptFileLinks.isEmpty());
    // We do not hide the support links label and
    // the script files label if they are empty,
    // because without them the layout would be collapse.

    if (pMapping) {
        pMapping->loadSettings(m_pConfig, m_pController->getName());
        auto settings = pMapping->getSettings();
        auto* pLayout = pMapping->getSettingsLayout();

        QLayoutItem* pItem;
        while ((pItem = m_ui.settingsTab->layout()->takeAt(0)) != nullptr) {
            delete pItem->widget();
            delete pItem;
        }

        if (pLayout != nullptr && !settings.isEmpty()) {
            QWidget* pSettingsWidget = pLayout->build(m_ui.settingsTab);
            m_ui.settingsTab->layout()->addWidget(pSettingsWidget);

            // Add an expanding spacer so that when we collapse all groups,
            // they are pushed to the top.
            m_ui.settingsTab->layout()->addItem(new QSpacerItem(
                    1, 1, QSizePolicy::Minimum, QSizePolicy::Expanding));

            // Make all top-level groupboxes checkable so we get the
            // collapse/expand functionality. Qt::FindDirectChildrenOnly
            // ensures we only iterate over the top-level groupboxes.
            const QList<WCollapsibleGroupBox*> boxes =
                    pSettingsWidget->findChildren<WCollapsibleGroupBox*>(
                            QString() /* match any ObjectName */,
                            Qt::FindDirectChildrenOnly);
            for (auto* pBox : std::as_const(boxes)) {
                const QString title = pBox->title();
                pBox->setCheckable(true);
                // The collapsed state is saved/restored via the groupbox' title.
                // Note: If multiple top-levle groups happen to have the same title
                // (which should not normally happen in well-behaved controller mappings,
                // but is not strictly prohibited), the last one to be expanded/
                // collapsed determines the state that will be restored.
                if (m_settingsCollapsedStates.contains(title)) {
                    pBox->setChecked(m_settingsCollapsedStates.value(title));
                }

                connect(pBox,
                        &WCollapsibleGroupBox::toggled,
                        this,
                        [this, title](bool checked) {
                            m_settingsCollapsedStates.insert(title, checked);
                        });
            }

            for (const auto& setting : std::as_const(settings)) {
                connect(setting.get(),
                        &AbstractLegacyControllerSetting::changed,
                        this,
                        [this] { setDirty(true); });
            }
        }
    }

    // Show or hide the settings tab based on the presence of settings
    bool showSettings = pMapping && !pMapping->getSettings().isEmpty();
    m_ui.controllerTabs->setTabVisible(m_settingsTabIndex, showSettings);
    m_ui.controllerTabs->setTabEnabled(m_settingsTabIndex, showSettings);

    // If there is still settings that may be saved and no new mapping selected
    // (e.g restored default), we keep the the dirty mapping live so it can be
    // saved in apply slot. If there is a new mapping, then setting changes are
    // discarded
    if (pMapping || (m_pMapping && !m_pMapping->hasDirtySettings())) {
        // We mutate this mapping so keep a reference to it while we are using it.
        // TODO: Clone it? Technically a waste since nothing else uses this
        // copy but if someone did they might not expect it to change.
        m_pMapping = pMapping;
    }

#ifdef MIXXX_USE_QML
    // Add the screens tab if there are screens
    if (pMapping && CmdlineArgs::Instance().getControllerPreviewScreens()) {
        auto screens = pMapping->getInfoScreens();
        bool hasScreens = !screens.isEmpty();
        m_ui.controllerTabs->setTabVisible(m_screensTabIndex, hasScreens);
        m_ui.controllerTabs->setTabEnabled(m_screensTabIndex, hasScreens);
        if (hasScreens) {
            slotShowPreviewScreens(m_pController->getScriptEngine().get());
        }
    } else {
        m_ui.controllerTabs->setTabVisible(m_screensTabIndex, false);
        m_ui.controllerTabs->setTabEnabled(m_screensTabIndex, false);
    }
#endif

    // Inputs tab
    ControllerInputMappingTableModel* pInputModel =
            new ControllerInputMappingTableModel(this,
                    m_pControlPickerMenu,
                    m_ui.midiInputMappingTableView);
    pInputModel->setMapping(pMapping);

    ControllerMappingTableProxyModel* pInputProxyModel =
            new ControllerMappingTableProxyModel(pInputModel);
    m_ui.midiInputMappingTableView->setModel(pInputProxyModel);

    for (int i = 0; i < pInputModel->columnCount(); ++i) {
        QAbstractItemDelegate* pDelegate = pInputModel->delegateForColumn(
                i, m_ui.midiInputMappingTableView);
        if (pDelegate) {
            qDebug() << "Setting input delegate for column" << i << pDelegate;
            m_ui.midiInputMappingTableView->setItemDelegateForColumn(i, pDelegate);
        }
    }

    // Now that we have set the new model our old model can be deleted.
    delete m_pInputProxyModel;
    m_pInputProxyModel = pInputProxyModel;
    delete m_pInputTableModel;
    m_pInputTableModel = pInputModel;
    // Trigger search when the model was recreated after hitting Apply
    slotInputControlSearch();

    // Outputs tab
    ControllerOutputMappingTableModel* pOutputModel =
            new ControllerOutputMappingTableModel(this,
                    m_pControlPickerMenu,
                    m_ui.midiOutputMappingTableView);
    pOutputModel->setMapping(pMapping);

    ControllerMappingTableProxyModel* pOutputProxyModel =
            new ControllerMappingTableProxyModel(pOutputModel);
    m_ui.midiOutputMappingTableView->setModel(pOutputProxyModel);

    for (int i = 0; i < pOutputModel->columnCount(); ++i) {
        QAbstractItemDelegate* pDelegate = pOutputModel->delegateForColumn(
                i, m_ui.midiOutputMappingTableView);
        if (pDelegate) {
            qDebug() << "Setting output delegate for column" << i << pDelegate;
            m_ui.midiOutputMappingTableView->setItemDelegateForColumn(i, pDelegate);
        }
    }

    // Now that we have set the new model our old model can be deleted.
    delete m_pOutputProxyModel;
    m_pOutputProxyModel = pOutputProxyModel;
    delete m_pOutputTableModel;
    m_pOutputTableModel = pOutputModel;
    // Trigger search when the model was recreated after hitting Apply
    slotOutputControlSearch();

    // Hide the entire QTabWidget if all tabs are removed
    m_ui.controllerTabs->setVisible(getNumberOfVisibleTabs() > 0);

    // Set the first visible tab as the current tab
    int firstVisibleTabIndex = getIndexOfFirstVisibleTab();
    if (firstVisibleTabIndex >= 0) {
        m_ui.controllerTabs->setCurrentIndex(firstVisibleTabIndex);
    }
}

void DlgPrefController::slotInputControlSearch() {
    VERIFY_OR_DEBUG_ASSERT(m_pInputProxyModel) {
        return;
    }
    m_pInputProxyModel->search(m_ui.inputControlSearch->text());
}

void DlgPrefController::slotOutputControlSearch() {
    VERIFY_OR_DEBUG_ASSERT(m_pOutputProxyModel) {
        return;
    }
    m_pOutputProxyModel->search(m_ui.outputControlSearch->text());
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
        m_ui.midiInputMappingTableView->selectionModel()->select(
                QItemSelection(left, right),
                QItemSelectionModel::Clear | QItemSelectionModel::Select);
        m_ui.midiInputMappingTableView->scrollTo(left);
    }
}

void DlgPrefController::removeInputMappings() {
    if (m_pInputProxyModel) {
        QItemSelection selection = m_pInputProxyModel->mapSelectionToSource(
                m_ui.midiInputMappingTableView->selectionModel()->selection());
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
        m_ui.midiOutputMappingTableView->selectionModel()->select(
                QItemSelection(left, right),
                QItemSelectionModel::Clear | QItemSelectionModel::Select);
        m_ui.midiOutputMappingTableView->scrollTo(left);
    }
}

void DlgPrefController::removeOutputMappings() {
    if (m_pOutputProxyModel) {
        QItemSelection selection = m_pOutputProxyModel->mapSelectionToSource(
                m_ui.midiOutputMappingTableView->selectionModel()->selection());
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
