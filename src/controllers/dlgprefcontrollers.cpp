#include "controllers/dlgprefcontrollers.h"

#include <QDesktopServices>

#include "controllers/controllermanager.h"
#include "controllers/defs_controllers.h"
#include "controllers/dlgprefcontroller.h"
#include "defs_urls.h"
#include "moc_dlgprefcontrollers.cpp"
#include "preferences/dialog/dlgpreferences.h"

DlgPrefControllers::DlgPrefControllers(DlgPreferences* pPreferences,
        UserSettingsPointer pConfig,
        std::shared_ptr<ControllerManager> pControllerManager,
        QTreeWidgetItem* pControllersRootItem)
        : DlgPreferencePage(pPreferences),
          m_pDlgPreferences(pPreferences),
          m_pConfig(pConfig),
          m_pControllerManager(pControllerManager),
          m_pControllersRootItem(pControllersRootItem) {
    setupUi(this);
    // Create text color for the cue mode link "?" to the manual
    createLinkColor();
    setupControllerWidgets();

    connect(btnOpenUserMappings, &QPushButton::clicked, [=]() {
        QString mappingsPath = userMappingsPath(m_pConfig);
        openLocalFile(mappingsPath);
    });

    // Connections
    connect(m_pControllerManager.get(),
            &ControllerManager::devicesChanged,
            this,
            &DlgPrefControllers::rescanControllers);

    // Setting the description text here instead of in the ui file allows to paste
    // a formatted link (text color is a more readable blend of text color and original link color).
    txtMappingsOverview->setText(tr(
            "Mixxx uses \"mappings\" to connect messages from your controller to "
            "controls in Mixxx. If you do not see a mapping for your controller "
            "in the \"Load Mapping\" menu when you click on your controller on the "
            "left sidebar, you may be able to download one online from the %1. "
            "Place the XML (.xml) and Javascript (.js) file(s) in the \"User Mapping "
            "Folder\" then restart Mixxx. If you download a mapping in a ZIP file, "
            "extract the XML and Javascript file(s) from the ZIP file to your "
            "\"User Mapping Folder\" then restart Mixxx.")
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
    QDesktopServices::openUrl(QUrl::fromLocalFile(file));
}

void DlgPrefControllers::slotUpdate() {
    for (DlgPrefController* pControllerDlg : qAsConst(m_controllerPages)) {
        pControllerDlg->slotUpdate();
    }
}

void DlgPrefControllers::slotCancel() {
    for (DlgPrefController* pControllerDlg : qAsConst(m_controllerPages)) {
        pControllerDlg->slotCancel();
    }
}

void DlgPrefControllers::slotApply() {
    for (DlgPrefController* pControllerDlg : qAsConst(m_controllerPages)) {
        pControllerDlg->slotApply();
    }
}

void DlgPrefControllers::slotResetToDefaults() {
    for (DlgPrefController* pControllerDlg : qAsConst(m_controllerPages)) {
        pControllerDlg->slotResetToDefaults();
    }
}

QUrl DlgPrefControllers::helpUrl() const {
    return QUrl(MIXXX_MANUAL_CONTROLLERS_URL);
}

bool DlgPrefControllers::handleTreeItemClick(QTreeWidgetItem* clickedItem) {
    int controllerIndex = m_controllerTreeItems.indexOf(clickedItem);
    if (controllerIndex >= 0) {
        DlgPrefController* pControllerDlg = m_controllerPages.value(controllerIndex);
        if (pControllerDlg) {
            m_pDlgPreferences->switchToPage(pControllerDlg);
        }
        return true;
    } else if (clickedItem == m_pControllersRootItem) {
        // Switch to the root page and expand the controllers tree item.
        m_pDlgPreferences->expandTreeItem(clickedItem);
        m_pDlgPreferences->switchToPage(this);
        return true;
    }
    return false;
}

void DlgPrefControllers::rescanControllers() {
    destroyControllerWidgets();
    setupControllerWidgets();
}

void DlgPrefControllers::destroyControllerWidgets() {
    while (!m_controllerPages.isEmpty()) {
        DlgPrefController* pControllerDlg = m_controllerPages.takeLast();
        m_pDlgPreferences->removePageWidget(pControllerDlg);
        delete pControllerDlg;
    }

    m_controllerTreeItems.clear();
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

    std::sort(controllerList.begin(), controllerList.end(), controllerCompare);

    for (auto* pController : controllerList) {
        DlgPrefController* pControllerDlg = new DlgPrefController(
                this, pController, m_pControllerManager, m_pConfig);
        connect(pControllerDlg,
                &DlgPrefController::mappingStarted,
                m_pDlgPreferences,
                &DlgPreferences::hide);
        connect(pControllerDlg,
                &DlgPrefController::mappingEnded,
                m_pDlgPreferences,
                &DlgPreferences::show);

        m_controllerPages.append(pControllerDlg);

        connect(pController,
                &Controller::openChanged,
                [this, pControllerDlg](bool bOpen) {
                    slotHighlightDevice(pControllerDlg, bOpen);
                });

        QTreeWidgetItem* pControllerTreeItem = new QTreeWidgetItem(
                QTreeWidgetItem::Type);
        m_pDlgPreferences->addPageWidget(
                DlgPreferences::PreferencesPage(pControllerDlg, pControllerTreeItem),
                pController->getName(),
                "ic_preferences_controllers.svg");

        m_pControllersRootItem->addChild(pControllerTreeItem);
        m_controllerTreeItems.append(pControllerTreeItem);

        // If controller is open make controller label bold
        QFont temp = pControllerTreeItem->font(0);
        temp.setBold(pController->isOpen());
        pControllerTreeItem->setFont(0, temp);
    }
}

void DlgPrefControllers::slotHighlightDevice(DlgPrefController* pControllerDlg, bool enabled) {
    int controllerPageIndex = m_controllerPages.indexOf(pControllerDlg);
    if (controllerPageIndex < 0) {
        return;
    }

    QTreeWidgetItem* pControllerTreeItem =
            m_controllerTreeItems.at(controllerPageIndex);
    if (!pControllerTreeItem) {
        return;
    }

    QFont temp = pControllerTreeItem->font(0);
    temp.setBold(enabled);
    pControllerTreeItem->setFont(0, temp);
}
