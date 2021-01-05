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
                                       QTreeWidgetItem* pControllerTreeItem)
        : DlgPreferencePage(pPreferences),
          m_pDlgPreferences(pPreferences),
          m_pConfig(pConfig),
          m_pControllerManager(pControllerManager),
          m_pControllerTreeItem(pControllerTreeItem) {
    setupUi(this);
    setupControllerWidgets();

    const QString presetsPath = userPresetsPath(m_pConfig);
    connect(btnOpenUserPresets, &QPushButton::clicked,
            this, [this, presetsPath] { slotOpenLocalFile(presetsPath); });

    // Connections
    connect(m_pControllerManager,
            &ControllerManager::devicesChanged,
            this,
            &DlgPrefControllers::rescanControllers);

    // Setting the description text here instead of in the ui file allows to paste
    // a formatted link (text color is a more readable blend of text color and original link color).
    txtPresetsOverview->setText(tr(
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
            tr("MIDI Mapping File Format"),
            MIXXX_WIKI_CONTROLLER_PRESET_FORMAT_URL));

    txtControllerPresetFormat->setText(coloredLinkString(
            m_pLinkColor,
            QStringLiteral("Mixxx Controller Forums"),
            MIXXX_CONTROLLER_FORUMS_URL));

    txtControllerScripting->setText(coloredLinkString(
            m_pLinkColor,
            tr("MIDI Scripting with Javascript"),
            MIXXX_WIKI_MIDI_SCRIPTING_URL));
}

DlgPrefControllers::~DlgPrefControllers() {
    destroyControllerWidgets();
}

void DlgPrefControllers::slotOpenLocalFile(const QString& file) {
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

    foreach (Controller* pController, controllerList) {
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
        m_pDlgPreferences->addPageWidget(controllerDlg);

        connect(pController,
                &Controller::openChanged,
                [this, controllerDlg](bool bOpen) {
                    slotHighlightDevice(controllerDlg, bOpen);
                });

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
