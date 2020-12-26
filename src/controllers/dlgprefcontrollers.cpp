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
        ControllerManager* pControllerManager,
        QTreeWidgetItem* pControllersRootItem)
        : DlgPreferencePage(pPreferences),
          m_pDlgPreferences(pPreferences),
          m_pConfig(pConfig),
          m_pControllerManager(pControllerManager),
          m_pControllersRootItem(pControllersRootItem) {
    setupUi(this);
    setupControllerWidgets();

    const QString presetsPath = userPresetsPath(m_pConfig);
    connect(btnOpenUserPresets,
            &QPushButton::clicked,
            this,
            [this, presetsPath] { slotOpenLocalFile(presetsPath); });

    // Connections
    connect(m_pControllerManager,
            &ControllerManager::devicesChanged,
            this,
            &DlgPrefControllers::rescanControllers);
}

DlgPrefControllers::~DlgPrefControllers() {
    destroyControllerWidgets();
}

void DlgPrefControllers::slotOpenLocalFile(const QString& file) {
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

    foreach (Controller* pController, controllerList) {
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
        m_pDlgPreferences->addPageWidget(pControllerDlg,
                pControllerTreeItem,
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
