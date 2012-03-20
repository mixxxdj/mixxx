/**
* @file dlgprefcontroller.cpp
* @author Sean M. Pappalardo  spappalardo@mixxx.org
* @date Mon May 2 2011
* @brief Configuration dialog for a DJ controller
*
*/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QtGui>
#include <QDebug>
#include "dlgprefcontroller.h"
#include "controller.h"
#include "controllermanager.h"
#include "configobject.h"
#include "defs_controllers.h"

DlgPrefController::DlgPrefController(QWidget *parent, Controller* controller,
                                         ControllerManager* controllerManager,
                                         ConfigObject<ConfigValue> *pConfig)
    : QWidget(parent), Ui::DlgPrefControllerDlg()
{
    setupUi(this);
    m_pConfig = pConfig;
    m_pController = controller;
    m_pControllerManager = controllerManager;
    
    m_bDirty = false;

    labelDeviceName->setText(m_pController->getName());

    connect(comboBoxPreset, SIGNAL(activated(const QString&)), this, SLOT(slotLoadPreset(const QString&)));
    connect(comboBoxPreset, SIGNAL(activated(const QString&)), this, SLOT(slotDirty()));

    connect(this, SIGNAL(openController(bool)), m_pController, SLOT(open()));
    connect(this, SIGNAL(openController(bool)), m_pControllerManager, SLOT(enablePolling(bool)));
    connect(this, SIGNAL(closeController(bool)), m_pController, SLOT(close()));
    connect(this, SIGNAL(closeController(bool)), m_pControllerManager, SLOT(enablePolling(bool)));
    connect(this, SIGNAL(loadPreset(QString,bool)), m_pController, SLOT(loadPreset(QString,bool)));
    connect(this, SIGNAL(applyPreset()), m_pController, SLOT(applyPreset()));
    
    //Load the list of presets into the presets combobox.
    enumeratePresets();
}

DlgPrefController::~DlgPrefController() {
}

void DlgPrefController::slotDirty ()
{
    m_bDirty = true;
}

void DlgPrefController::enumeratePresets()
{
    comboBoxPreset->clear();

    //Insert a dummy "..." item at the top to try to make it less confusing.
    //(We don't want the first found file showing up as the default item
    // when a user has their controller plugged in)
    comboBoxPreset->addItem("...");
    
    // Ask the controller manager for a list of applicable presets
    QList<QString> presetsList =
        m_pControllerManager->getPresetList(m_pController->presetExtension());
    
    //Sort in alphabetical order
    qSort(presetsList);
    comboBoxPreset->addItems(presetsList);
}

/* slotUpdate()
 * Called when the dialog is displayed.
 */
void DlgPrefController::slotUpdate() {

    //Check if the device that this dialog is for is already enabled...
    if (m_pController->isOpen())
    {
        chkEnabledDevice->setCheckState(Qt::Checked); //Check the "Enabled" box
//         toolBox->setEnabled(true); //Enable MIDI in/out toolbox.
        groupBoxPresets->setEnabled(true); //Enable presets group box.
    }
    else {
        chkEnabledDevice->setCheckState(Qt::Unchecked); //Uncheck the "Enabled" box
//         toolBox->setEnabled(false); //Disable MIDI in/out toolbox.
        groupBoxPresets->setEnabled(false); //Disable presets group box.
    }

    //Connect the "Enabled" checkbox after the checkbox state is set
    connect(chkEnabledDevice, SIGNAL(stateChanged(int)), this, SLOT(slotDeviceState(int)));
    connect(chkEnabledDevice, SIGNAL(stateChanged(int)), this, SLOT(slotDirty()));
}

/* slotApply()
 * Called when the OK button is pressed.
 */
void DlgPrefController::slotApply() {
    /* User has pressed OK, so if anything has been changed, enable or disable
     * the device, write the controls to the DOM, and reload the presets.
     */
    if (m_bDirty)
    {
        if (chkEnabledDevice->isChecked()) {
            //Enable the device.
            enableDevice();
            emit(applyPreset());
        }
        else disableDevice();

        //Select the "..." item again in the combobox.
        comboBoxPreset->setCurrentIndex(0);
    }
    m_bDirty = false;
}

/* slotLoadPreset()
* Loads the specified XML preset.
*/
void DlgPrefController::slotLoadPreset(const QString &name) {
    
    if (name == "...")
        return;
    
    QString filename = LPRESETS_PATH + name + m_pController->presetExtension();
    QFile ftest(filename);
    if ( !ftest.exists() ) filename = m_pConfig->getConfigPath().append("controllers/") + name + m_pController->presetExtension();
    
    if (!filename.isNull()) emit(loadPreset(filename, true));    // It's applied on prefs close

    // Save the file path/name in the config so it can be auto-loaded at startup next time
    m_pConfig->set(ConfigKey("[ControllerPreset]", m_pController->getName().replace(" ", "_")), filename);
}

void DlgPrefController::slotDeviceState(int state) {
  if (state == Qt::Checked) {
      groupBoxPresets->setEnabled(true);    //Enable presets group box.
      emit deviceStateChanged(this,true);  // Set tree item text to bold
  }
  else {
      groupBoxPresets->setEnabled(false);   //Disable presets group box.
      emit deviceStateChanged(this,false);  // Set tree item text to not bold
  }
}

void DlgPrefController::enableDevice()
{
    emit(closeController(false));
    emit(openController(m_pController->needPolling()));
    m_pConfig->set(ConfigKey("[Controller]", m_pController->getName().replace(" ", "_")), 1);

    //TODO: Should probably check if open() actually succeeded.
}

void DlgPrefController::disableDevice()
{
    emit(closeController(false));
    m_pConfig->set(ConfigKey("[Controller]", m_pController->getName().replace(" ", "_")), 0);

    //TODO: Should probably check if close() actually succeeded.
}