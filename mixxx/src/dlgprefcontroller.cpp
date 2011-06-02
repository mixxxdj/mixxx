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
#include "controllers/controller.h"
#include "controllers/controllermanager.h"
#include "configobject.h"

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

    connect(comboBoxPreset, SIGNAL(activated(const QString&)), this, SLOT(slotDirty()));
    
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
    QList<QString> presetsList = m_pControllerManager->getPresetList();   
    
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
    /* User has pressed OK, so enable or disable the device, write the
     * controls to the DOM, and reload the MIDI bindings.  FIXED: only
     * do this if the user has changed the preferences.
     */
    if (m_bDirty)
    {
        if (chkEnabledDevice->isChecked()) {
            //Enable the device.
            enableDevice();
//             m_pController->getMidiMapping()->applyPreset();
        }
        else disableDevice();
    }
    m_bDirty = false;    
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
    m_pController->close();
    m_pController->open();
    m_pConfig->set(ConfigKey("[Controller]", m_pController->getName().replace(" ", "_")), 1);

    //TODO: Should probably check if open() actually succeeded.
}

void DlgPrefController::disableDevice()
{
    m_pController->close();
    m_pConfig->set(ConfigKey("[Controller]", m_pController->getName().replace(" ", "_")), 0);

    //TODO: Should probably check if close() actually succeeded.
}