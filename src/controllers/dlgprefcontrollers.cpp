#include "controllers/dlgprefcontrollers.h"

#include "control/controlproxy.h"
#include "controllers/controller.h"
#include "controllers/controllermanager.h"
#include "controllers/defs_controllers.h"
#include "controllers/dlgprefcontroller.h"
#include "defs_urls.h"
#include "moc_dlgprefcontrollers.cpp"
#include "preferences/dialog/dlgpreferences.h"
#include "util/desktophelper.h"
#include "util/string.h"

namespace {
const QString kAppGroup = QStringLiteral("[App]");
} // namespace

DlgPrefControllers::DlgPrefControllers(DlgPreferences* pPreferences,
        UserSettingsPointer pConfig,
        std::shared_ptr<ControllerManager> pControllerManager,
        QTreeWidgetItem* pControllersRootItem)
        : DlgPreferencePage(pPreferences),
          m_pDlgPreferences(pPreferences),
          m_pConfig(pConfig),
          m_pControllerManager(pControllerManager),
          m_pControllersRootItem(pControllersRootItem),
          m_pNumDecks(make_parented<ControlProxy>(
                  kAppGroup, QStringLiteral("num_decks"), this)),
          m_pNumSamplers(make_parented<ControlProxy>(
                  kAppGroup, QStringLiteral("num_samplers"), this)) {
    setupUi(this);
    // Create text color for the cue mode link "?" to the manual
    createLinkColor();
    setupControllerWidgets();

    connect(btnOpenUserMappings, &QPushButton::clicked, this, [this]() {
        QString mappingsPath = userMappingsPath(m_pConfig);
        openLocalFile(mappingsPath);
    });

    // Connections
    connect(m_pControllerManager.get(),
            &ControllerManager::devicesChanged,
            this,
            &DlgPrefControllers::rescanControllers);

    connect(m_pControllerManager.get(),
            &ControllerManager::deviceAdded,
            this,
            &DlgPrefControllers::slotSetupControllerWidget);

    connect(m_pControllerManager.get(),
            &ControllerManager::deviceRemoved,
            this,
            &DlgPrefControllers::slotDestroyControllerWidget);

    comboBox_midiAPI->addItem("None", "None");

#ifdef __PORTMIDI__
    comboBox_midiAPI->addItem("PortMidi", "PortMidi");
#endif

#ifdef __LIBREMIDI__
    comboBox_midiAPI->addItem("Libremidi", "Libremidi");
#endif

#if defined(__PORTMIDI__) || defined(__LIBREMIDI__)
    checkBox_midithrough->setChecked(m_pConfig->getValue(kMidiThroughCfgKey, false));
    comboBox_midiAPI->setCurrentText(m_pConfig->getValue(kMidiAPI, kDefaultMidiAPI));
    connect(comboBox_midiAPI,
            &QComboBox::currentTextChanged,
            this,
            &DlgPrefControllers::slotMidiAPIChanged);
    connect(checkBox_midithrough,
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
            &QCheckBox::checkStateChanged,
#else
            &QCheckBox::stateChanged,
#endif
            this,
            &DlgPrefControllers::slotMidiThroughChanged);
    txt_midithrough->setTextFormat(Qt::RichText);

    //: text enclosed in <b> is bold, <br/> is a linebreak
    //: %1 is the placehodler for 'MIDI Through Port'
    txt_midithrough->setText(tr(
            "%1 is a virtual controller that allows to use e.g. the 'MIDI for light' "
            "mapping.<br/>"
            "You need to restart Mixxx in order to enable it.<br/>"
            "<b>Note:</b> mappings meant for physical controllers can cause issues and "
            "even render the Mixxx GUI unresponsive when being loaded to %1.")
                    .arg(kMidiThroughPortPrefix));
#else
    line_midithrough->hide();
    checkBox_midithrough->hide();
    txt_midithrough->hide();
    groupBox_midiAPI->hide();
#endif

    // Setting the description text here instead of in the ui file allows to paste
    // a formatted link (text color is a more readable blend of text color and original link color).
    txtMappingsOverview->setText(tr(
            "Mixxx uses \"mappings\" to connect messages from your controller to "
            "controls in Mixxx. If you do not see a mapping for your controller "
            "in the \"Load Mapping\" menu when you click on your controller on the "
            "left sidebar, you may be able to download one online from the %1. "
            "Place the XML (.xml) and Javascript (.js) file(s) in the \"User Mapping "
            "Folder\" then click the Reload button next to the mapping selector to "
            "reload all available mappings. If you download a mapping in a ZIP file, "
            "extract the XML and Javascript file(s) from the ZIP file to your "
            "\"User Mapping Folder\" then click the Reload button.")
                    .arg(coloredLinkString(
                            m_pLinkColor,
                            QStringLiteral("Mixxx Controller Forums"),
                            MIXXX_CONTROLLER_FORUMS_URL)));

    txtHardwareCompatibility->setText(coloredLinkString(
            m_pLinkColor,
            tr("Mixxx DJ Hardware Guide"),
            MIXXX_WIKI_HARDWARE_COMPATIBILITY_URL));

    txtControllerForums->setText(coloredLinkString(
            m_pLinkColor,
            QStringLiteral("Mixxx Controller Forums"),
            MIXXX_CONTROLLER_FORUMS_URL));

    txtControllerMappingFormat->setText(coloredLinkString(
            m_pLinkColor,
            tr("MIDI Mapping File Format"),
            MIXXX_WIKI_CONTROLLER_MAPPING_FORMAT_URL));

    txtControllerScripting->setText(coloredLinkString(
            m_pLinkColor,
            tr("MIDI Scripting with Javascript"),
            MIXXX_WIKI_MIDI_SCRIPTING_URL));
}

DlgPrefControllers::~DlgPrefControllers() {
    destroyControllerWidgets();
}

void DlgPrefControllers::openLocalFile(const QString& file) {
    mixxx::DesktopHelper::openUrl(QUrl::fromLocalFile(file));
}

void DlgPrefControllers::slotUpdate() {
}

void DlgPrefControllers::slotCancel() {
    for (const auto& [_, pair] : std::as_const(m_controllerMap)) {
        pair.first->slotCancel();
    }
}

void DlgPrefControllers::slotApply() {
    for (const auto& [_, pair] : std::as_const(m_controllerMap)) {
        pair.first->slotApply();
    }
}

void DlgPrefControllers::slotResetToDefaults() {
    for (const auto& [_, pair] : std::as_const(m_controllerMap)) {
        pair.first->slotResetToDefaults();
    }
}

QUrl DlgPrefControllers::helpUrl() const {
    return QUrl(MIXXX_MANUAL_CONTROLLERS_URL);
}

bool DlgPrefControllers::handleTreeItemClick(QTreeWidgetItem* pClickedItem) {
    DlgPrefController* pControllerDlg = nullptr;

    for (const auto& [_, pair] : m_controllerMap) {
        if (pair.second == pClickedItem) {
            pControllerDlg = pair.first;
        }
    }

    if (pControllerDlg) {
        const QString pageTitle = m_pControllersRootItem->text(0) + " - " +
                pClickedItem->text(0);
        m_pDlgPreferences->switchToPage(pageTitle, pControllerDlg);
        return true;
    } else if (pClickedItem == m_pControllersRootItem) {
        // Switch to the root page and expand the controllers tree item.
        m_pDlgPreferences->expandTreeItem(pClickedItem);
        const QString pageTitle = pClickedItem->text(0);
        m_pDlgPreferences->switchToPage(pageTitle, this);
        return true;
    }
    return false;
}

void DlgPrefControllers::rescanControllers() {
    destroyControllerWidgets();
    setupControllerWidgets();
}

void DlgPrefControllers::slotSetupControllerWidget(Controller* pController) {
    setupControllerWidget(pController);
}

void DlgPrefControllers::slotDestroyControllerWidget(Controller* pController) {
    destroyControllerWidget(pController);
}

void DlgPrefControllers::setupControllerWidget(Controller* pController) {
    auto pControllerDlg = make_parented<DlgPrefController>(
            this, pController, m_pControllerManager, m_pConfig);
    connect(pControllerDlg,
            &DlgPrefController::mappingStarted,
            m_pDlgPreferences,
            &DlgPreferences::hide);
    connect(pControllerDlg,
            &DlgPrefController::mappingEnded,
            m_pDlgPreferences,
            &DlgPreferences::show);
    // Recreate the control picker menus when decks or samplers are added
    m_pNumDecks->connectValueChanged(pControllerDlg.get(),
            &DlgPrefController::slotRecreateControlPickerMenu);
    m_pNumSamplers->connectValueChanged(pControllerDlg.get(),
            &DlgPrefController::slotRecreateControlPickerMenu);

    connect(pController,
            &Controller::openChanged,
            this,
            [this, pController](bool bOpen) {
                slotHighlightDevice(pController, bOpen);
            });

    QTreeWidgetItem* pControllerTreeItem = new QTreeWidgetItem(
            QTreeWidgetItem::Type);

    const QString treeImage = [protocol = pController->getDataRepresentationProtocol()] {
        switch (protocol) {
        case DataRepresentationProtocol::USB_BULK_TRANSFER:
            return QStringLiteral("ic_preferences_bulk.svg");
        case DataRepresentationProtocol::HID:
            return QStringLiteral("ic_preferences_hid.svg");
        case DataRepresentationProtocol::MIDI:
            return QStringLiteral("ic_preferences_midi.svg");
        default:
            return QStringLiteral("ic_preferences_controllers.svg");
        }
    }();

    m_pDlgPreferences->addPageWidget(
            DlgPreferences::PreferencesPage(pControllerDlg, pControllerTreeItem),
            pController->getName(),
            treeImage);

    m_pControllersRootItem->addChild(pControllerTreeItem);
    m_controllerMap.emplace(pController, std::pair(pControllerDlg.get(), pControllerTreeItem));

    // If controller is open make controller label bold
    QFont temp = pControllerTreeItem->font(0);
    temp.setBold(pController->isOpen());
    pControllerTreeItem->setFont(0, temp);
}

void DlgPrefControllers::destroyControllerWidget(Controller* pController) {
    // pController->disconnect(this);
    const auto& value = m_controllerMap.extract(pController);

    DEBUG_ASSERT(!value.empty());

    DlgPrefController* pControllerDlg = value.mapped().first;
    m_pDlgPreferences->removePageWidget(pControllerDlg);
    delete pControllerDlg;

    QTreeWidgetItem* pControllerTreeItem = value.mapped().second;
    m_pControllersRootItem->removeChild(pControllerTreeItem);
    delete pControllerTreeItem;
}

void DlgPrefControllers::destroyControllerWidgets() {
    // NOTE: this assumes that the list of controllers does not change during the lifetime of Mixxx.
    // This is currently true, but once we support hotplug, we will need better lifecycle management
    // to keep this dialog and the controllermanager consistent.
    QList<Controller*> controllerList =
            m_pControllerManager->getControllerList(false, true);
    for (auto* pController : std::as_const(controllerList)) {
        pController->disconnect(this);
    }

    for (auto& [key, value] : m_controllerMap) {
        DlgPrefController* pControllerDlg = value.first;
        m_pDlgPreferences->removePageWidget(pControllerDlg);
    }

    m_controllerMap.clear();

    while (m_pControllersRootItem->childCount() > 0) {
        QTreeWidgetItem* pControllerTreeItem = m_pControllersRootItem->takeChild(0);
        delete pControllerTreeItem;
    }
}

void DlgPrefControllers::setupControllerWidgets() {
    // For each controller, create a dialog and put a little link to it in the
    // treepane on the left.
    QList<Controller*> controllerList =
            m_pControllerManager->getControllerList(false, true);
    if (controllerList.isEmpty()) {
        // If no controllers are available, show the "No controllers available" message.
        txtNoControllersAvailable->setVisible(true);
        return;
    }
    txtNoControllersAvailable->setVisible(false);

    for (auto* pController : std::as_const(controllerList)) {
        setupControllerWidget(pController);
    }
}

void DlgPrefControllers::slotHighlightDevice(Controller* pController, bool enabled) {
    if (!m_controllerMap.contains(pController)) {
        return;
    }

    QTreeWidgetItem* pControllerTreeItem = m_controllerMap.at(pController).second;

    if (!pControllerTreeItem) {
        return;
    }

    QFont temp = pControllerTreeItem->font(0);
    temp.setBold(enabled);
    pControllerTreeItem->setFont(0, temp);
}

#if defined(__PORTMIDI__) || defined(__LIBREMIDI__)
void DlgPrefControllers::slotMidiAPIChanged(const QString& api) {
    m_pConfig->setValue(kMidiAPI, api);
}
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
void DlgPrefControllers::slotMidiThroughChanged(Qt::CheckState state) {
    m_pConfig->setValue(kMidiThroughCfgKey, state != Qt::Unchecked);
}
#else
void DlgPrefControllers::slotMidiThroughChanged(bool checked) {
    m_pConfig->setValue(kMidiThroughCfgKey, checked);
}
#endif
#endif
