#include "controllers/dlgprefcontrollers.h"

#include "dlgpreferences.h"
#include "controllers/controllermanager.h"
#include "controllers/dlgprefcontroller.h"
#include "controllers/dlgprefmappablecontroller.h"

DlgPrefControllers::DlgPrefControllers(DlgPreferences* pPreferences,
                                       ConfigObject<ConfigValue>* pConfig,
                                       ControllerManager* pControllerManager,
                                       QTreeWidgetItem* pControllerTreeItem)
        : DlgPreferencePage(pPreferences),
          m_pDlgPreferences(pPreferences),
          m_pConfig(pConfig),
          m_pControllerManager(pControllerManager),
          m_pControllerTreeItem(pControllerTreeItem) {
    setupUi(this);
    setupControllerWidgets();

    // Connections
    connect(m_pControllerManager, SIGNAL(devicesChanged()),
            this, SLOT(rescanControllers()));

}

DlgPrefControllers::~DlgPrefControllers() {
    destroyControllerWidgets();
}

void DlgPrefControllers::slotUpdate() {

}

void DlgPrefControllers::slotApply() {
    m_pControllerManager->savePresets();
}

bool DlgPrefControllers::handleTreeItemClick(QTreeWidgetItem* clickedItem) {
    int controllerIndex = m_controllerTreeItems.indexOf(clickedItem);
    if (controllerIndex >= 0) {
        DlgPrefController* controllerWidget = m_controllerWindows.value(controllerIndex);
        if (controllerWidget) {
            m_pDlgPreferences->switchToPage(controllerWidget);
            // Manually fire this slot since it doesn't work right...
            // TODO(rryan): Investigate whether/why this is still needed.
            controllerWidget->slotUpdate();
        }
        return true;
    } else if (clickedItem == m_pControllerTreeItem) {
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
    while (!m_controllerWindows.isEmpty()) {
        DlgPrefController* controllerDlg = m_controllerWindows.takeLast();
        m_pDlgPreferences->removePageWidget(controllerDlg);
        delete controllerDlg;
    }

    m_controllerTreeItems.clear();
    while(m_pControllerTreeItem->childCount() > 0) {
        QTreeWidgetItem* controllerWindowLink = m_pControllerTreeItem->takeChild(0);
        delete controllerWindowLink;
    }
}

void DlgPrefControllers::setupControllerWidgets() {
    // For each controller, create a dialog and put a little link to it in the
    // treepane on the left.
    QList<Controller*> controllerList =
            m_pControllerManager->getControllerList(false, true);
    qSort(controllerList.begin(), controllerList.end(), controllerCompare);

    foreach (Controller* pController, controllerList) {
        DlgPrefController* controllerDlg = NULL;
        if (pController->isMappable()) {
            controllerDlg = new DlgPrefMappableController(
                    this, pController, m_pControllerManager, m_pConfig);
            connect(controllerDlg, SIGNAL(mappingStarted()),
                    m_pDlgPreferences, SLOT(hide()));
            connect(controllerDlg, SIGNAL(mappingEnded()),
                    m_pDlgPreferences, SLOT(show()));
        } else {
            controllerDlg = new DlgPrefController(
                    this, pController, m_pControllerManager, m_pConfig);
        }

        m_controllerWindows.append(controllerDlg);
        m_pDlgPreferences->addPageWidget(controllerDlg);

        connect(m_pDlgPreferences, SIGNAL(showDlg()),
                controllerDlg, SLOT(enumeratePresets()));
        connect(m_pDlgPreferences, SIGNAL(showDlg()),
                controllerDlg, SLOT(slotUpdate()));

        connect(controllerDlg, SIGNAL(deviceStateChanged(DlgPrefController*, bool)),
                this, SLOT(slotHighlightDevice(DlgPrefController*, bool)));

        QTreeWidgetItem * controllerWindowLink = new QTreeWidgetItem(QTreeWidgetItem::Type);
        controllerWindowLink->setIcon(0, QIcon(":/images/preferences/ic_preferences_controllers.png"));
        QString curDeviceName = pController->getName();
        controllerWindowLink->setText(0, curDeviceName);
        controllerWindowLink->setTextAlignment(0, Qt::AlignLeft | Qt::AlignVCenter);
        controllerWindowLink->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        m_pControllerTreeItem->addChild(controllerWindowLink);
        m_controllerTreeItems.append(controllerWindowLink);

        // Set the font correctly
        QFont temp = controllerWindowLink->font(0);
        temp.setBold(pController->isOpen());
        controllerWindowLink->setFont(0, temp);
    }

    // If no controllers are available, show the "No controllers available"
    // message.
    noControllersAvailable->setVisible(controllerList.empty());
}

void DlgPrefControllers::slotHighlightDevice(DlgPrefController* dialog, bool enabled) {
    int dialogIndex = m_controllerWindows.indexOf(dialog);
    if (dialogIndex < 0) {
        return;
    }

    QTreeWidgetItem * controllerWindowLink =
            m_controllerTreeItems.at(dialogIndex);

    if (!controllerWindowLink) {
        return;
    }

    QFont temp = controllerWindowLink->font(0);
    temp.setBold(enabled);
    controllerWindowLink->setFont(0,temp);
}
