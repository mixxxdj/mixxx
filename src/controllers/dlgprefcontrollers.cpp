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
        QTreeWidgetItem* pControllerTreeItem)
        : DlgPreferencePage(pPreferences),
          m_pDlgPreferences(pPreferences),
          m_pConfig(pConfig),
          m_pControllerManager(pControllerManager),
          m_pControllerTreeItem(pControllerTreeItem) {
    setupUi(this);
    setupControllerWidgets();

    connect(btnOpenUserPresets, &QPushButton::clicked, [=]() {
        QString presetsPath = userPresetsPath(m_pConfig);
        openLocalFile(presetsPath);
    });

    // Connections
    connect(m_pControllerManager.get(),
            &ControllerManager::devicesChanged,
            this,
            &DlgPrefControllers::rescanControllers);
}

DlgPrefControllers::~DlgPrefControllers() {
    destroyControllerWidgets();
}

void DlgPrefControllers::openLocalFile(const QString& file) {
    QDesktopServices::openUrl(QUrl::fromLocalFile(file));
}

void DlgPrefControllers::slotUpdate() {
    for (DlgPrefController* pControllerWindows : qAsConst(m_controllerWindows)) {
        pControllerWindows->slotUpdate();
    }
}

void DlgPrefControllers::slotCancel() {
    for (DlgPrefController* pControllerWindows : qAsConst(m_controllerWindows)) {
        pControllerWindows->slotCancel();
    }
}

void DlgPrefControllers::slotApply() {
    for (DlgPrefController* pControllerWindows : qAsConst(m_controllerWindows)) {
        pControllerWindows->slotApply();
    }
}

void DlgPrefControllers::slotResetToDefaults() {
    for (DlgPrefController* pControllerWindows : qAsConst(m_controllerWindows)) {
        pControllerWindows->slotResetToDefaults();
    }
}

QUrl DlgPrefControllers::helpUrl() const {
    return QUrl(MIXXX_MANUAL_CONTROLLERS_URL);
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

    for (auto* pController : controllerList) {
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

        DlgPrefController* controllerDlg = new DlgPrefController(
                this, pController, m_pControllerManager, m_pConfig);
        connect(controllerDlg,
                &DlgPrefController::mappingStarted,
                m_pDlgPreferences,
                &DlgPreferences::hide);
        connect(controllerDlg,
                &DlgPrefController::mappingEnded,
                m_pDlgPreferences,
                &DlgPreferences::show);

        m_controllerWindows.append(controllerDlg);
        m_pDlgPreferences->addPageWidget(DlgPreferences::PreferencesPage(controllerDlg, controllerWindowLink));

        connect(pController,
                &Controller::openChanged,
                this,
                [this, controllerDlg](bool bOpen) {
                    slotHighlightDevice(controllerDlg, bOpen);
                });
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
