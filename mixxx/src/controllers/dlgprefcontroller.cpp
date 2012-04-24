/**
* @file dlgprefcontroller.cpp
* @author Sean M. Pappalardo  spappalardo@mixxx.org
* @date Mon May 2 2011
* @brief Configuration dialog for a DJ controller
*/

#include <QtGui>
#include <QDebug>

#include "controllers/dlgprefcontroller.h"
#include "controllers/controller.h"
#include "controllers/controllermanager.h"
#include "controllers/defs_controllers.h"
#include "configobject.h"

DlgPrefController::DlgPrefController(QWidget *parent, Controller* controller,
                                     ControllerManager* controllerManager,
                                     ConfigObject<ConfigValue> *pConfig)
        : QWidget(parent),
          m_pConfig(pConfig),
          m_pControllerManager(controllerManager),
          m_pController(controller),
          m_bDirty(false) {
    //QWidget * containerWidget = new QWidget();
    //QWidget * containerWidget = new QWidget(this);
    //m_ui.setupUi(containerWidget);
    m_ui.setupUi(this);
    m_pLayout = m_ui.gridLayout_4;
    m_pVerticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
    m_pLayout->addItem(m_pVerticalSpacer, 4, 0, 1, 3);

    //QVBoxLayout* vLayout = new QVBoxLayout(this);
    //vLayout->addWidget(containerWidget);
    //setLayout(vLayout);

    m_ui.labelDeviceName->setText(m_pController->getName());

    connect(m_ui.comboBoxPreset, SIGNAL(activated(const QString&)),
            this, SLOT(slotLoadPreset(const QString&)));
    connect(m_ui.comboBoxPreset, SIGNAL(activated(const QString&)),
            this, SLOT(slotDirty()));

    connect(this, SIGNAL(openController(Controller*)),
            m_pControllerManager, SLOT(openController(Controller*)));
    connect(this, SIGNAL(closeController(Controller*)),
            m_pControllerManager, SLOT(closeController(Controller*)));
    connect(this, SIGNAL(loadPreset(Controller*, QString, bool)),
            m_pControllerManager, SLOT(loadPreset(Controller*, QString, bool)));

    // Load the list of presets into the presets combobox.
    enumeratePresets();
}

DlgPrefController::~DlgPrefController() {
}

void DlgPrefController::addWidgetToLayout(QWidget* pWidget) {
    // Remove the vertical spacer since we're adding stuff
    m_pLayout->removeItem(m_pVerticalSpacer);
    m_pLayout->addWidget(pWidget);
}

void DlgPrefController::slotDirty() {
    m_bDirty = true;
}

void DlgPrefController::enumeratePresets() {
    m_ui.comboBoxPreset->clear();

    //Insert a dummy "..." item at the top to try to make it less confusing.
    //(We don't want the first found file showing up as the default item
    // when a user has their controller plugged in)
    m_ui.comboBoxPreset->addItem("...");

    // Ask the controller manager for a list of applicable presets
    QList<QString> presetsList =
        m_pControllerManager->getPresetList(m_pController->presetExtension());

    //Sort in alphabetical order
    qSort(presetsList);
    m_ui.comboBoxPreset->addItems(presetsList);
}

void DlgPrefController::slotUpdate() {
    // Check if the device that this dialog is for is already enabled...
    bool deviceOpen = m_pController->isOpen();
    // Check/uncheck the "Enabled" box
    m_ui.chkEnabledDevice->setCheckState(deviceOpen ? Qt::Checked : Qt::Unchecked);
    // Enable/disable presets group box.
    m_ui.groupBoxPresets->setEnabled(deviceOpen);
    // Connect the "Enabled" checkbox after the checkbox state is set
    connect(m_ui.chkEnabledDevice, SIGNAL(stateChanged(int)),
            this, SLOT(slotDeviceState(int)));
    connect(m_ui.chkEnabledDevice, SIGNAL(stateChanged(int)),
            this, SLOT(slotDirty()));
}

void DlgPrefController::slotApply() {
    /* User has pressed OK, so if anything has been changed, enable or disable
     * the device, write the controls to the DOM, and reload the presets.
     */
    if (m_bDirty) {
        if (m_ui.chkEnabledDevice->isChecked()) {
            enableDevice();
        } else {
            disableDevice();
        }

        //Select the "..." item again in the combobox.
        m_ui.comboBoxPreset->setCurrentIndex(0);
    }
    m_bDirty = false;
}

void DlgPrefController::slotLoadPreset(const QString &name) {
    if (name != "...") {
        emit(loadPreset(m_pController, name, true));
        // It's applied on prefs close
    }
}

void DlgPrefController::slotDeviceState(int state) {
    bool checked = state == Qt::Checked;
    // Enable/disable presets group box.
    m_ui.groupBoxPresets->setEnabled(checked);
    // Set tree item text to normal/bold
    emit(deviceStateChanged(this, checked));
}

void DlgPrefController::enableDevice() {
    emit(openController(m_pController));
    //TODO: Should probably check if open() actually succeeded.
}

void DlgPrefController::disableDevice() {
    emit(closeController(m_pController));
    //TODO: Should probably check if close() actually succeeded.
}
