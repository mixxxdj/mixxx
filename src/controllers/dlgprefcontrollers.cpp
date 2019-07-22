#include <QDesktopServices>

#include "controllers/dlgprefcontrollers.h"

#include "preferences/dialog/dlgpreferences.h"
#include "controllers/controllermanager.h"
#include "controllers/dlgprefcontroller.h"
#include "controllers/defs_controllers.h"

DlgPrefControllers::DlgPrefControllers(DlgPreferences* pPreferences,
                                       UserSettingsPointer pConfig,
                                       ControllerManager* pControllerManager,
                                       QTreeWidgetItem* pControllerTreeItem)
        : DlgPreferencePage(pPreferences),
          m_pDlgPreferences(pPreferences),
          m_pConfig(pConfig),
          m_pControllerManager(pControllerManager),
          m_pControllerTreeItem(pControllerTreeItem) {
    setupUi(this);
    setupControllerWidgets();

    connect(&m_buttonMapper, SIGNAL(mapped(QString)),
            this, SLOT(slotOpenLocalFile(QString)));

    connect(btnOpenUserPresets, SIGNAL(clicked()),
            &m_buttonMapper, SLOT(map()));

    m_buttonMapper.setMapping(btnOpenUserPresets, userPresetsPath(m_pConfig));

    // Connections
    connect(m_pControllerManager, SIGNAL(devicesChanged()),
            this, SLOT(rescanControllers()));
}

DlgPrefControllers::~DlgPrefControllers() {
    destroyControllerWidgets();
}

void DlgPrefControllers::slotOpenLocalFile(const QString& file) {
    QDesktopServices::openUrl(QUrl::fromLocalFile(file));
}

void DlgPrefControllers::slotUpdate() {
    // Update our sub-windows.
    foreach (DlgPrefController* pControllerWindows, m_controllerWindows) {
        pControllerWindows->slotUpdate();
    }
}

void DlgPrefControllers::slotCancel() {
    // Update our sub-windows.
    foreach (DlgPrefController* pControllerWindows, m_controllerWindows) {
        pControllerWindows->slotCancel();
    }
}

void DlgPrefControllers::slotApply() {
    // Update our sub-windows.
    foreach (DlgPrefController* pControllerWindows, m_controllerWindows) {
        pControllerWindows->slotApply();
    }

    // Save all controller presets.
    // TODO(rryan): Get rid of this and make DlgPrefController do this for each
    // preset.
    m_pControllerManager->savePresets();
}

bool DlgPrefControllers::handleTreeItemClick(QTreeWidgetItem* clickedItem) {
    int controllerIndex = m_controllerTreeItems.indexOf(clickedItem);
    if (controllerIndex >= 0) {
        DlgPrefController* controllerWidget = m_controllerWindows.value(controllerIndex);
        if (controllerWidget) {
            m_pDlgPreferences->switchToPage(controllerWidget);
        }
        return true;
    } else if (clickedItem == m_pControllerTreeItem) {
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
    std::sort(controllerList.begin(), controllerList.end(), controllerCompare);

    foreach (Controller* pController, controllerList) {
        DlgPrefController* controllerDlg = new DlgPrefController(
            this, pController, m_pControllerManager, m_pConfig);
        connect(controllerDlg, SIGNAL(mappingStarted()),
                m_pDlgPreferences, SLOT(hide()));
        connect(controllerDlg, SIGNAL(mappingEnded()),
                m_pDlgPreferences, SLOT(show()));

        m_controllerWindows.append(controllerDlg);
        m_pDlgPreferences->addPageWidget(controllerDlg);

        connect(controllerDlg, SIGNAL(controllerEnabled(DlgPrefController*, bool)),
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
    txtNoControllersAvailable->setVisible(controllerList.empty());
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
