/**
* @file dlgprefmappablecontroller.cpp
* @author Sean M. Pappalardo  spappalardo@mixxx.org
* @date Thur 12 Apr 2012
* @brief Configuration dialog for a DJ controller that supports GUI mapping
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

#include "dlgprefmappablecontroller.h"
#include "controllers/controllerlearningeventfilter.h"
#include "controllers/controllermanager.h"

DlgPrefMappableController::DlgPrefMappableController(QWidget *parent, Controller* controller,
                          ControllerManager* controllerManager,
                          ConfigObject<ConfigValue> *pConfig)
    : DlgPrefController(parent, controller, controllerManager, pConfig) {

    // Add our supplemental UI here
    // QWidget* containerWidget = new QWidget(this);
    // m_ui.setupUi(containerWidget);
    // addWidgetToLayout(containerWidget);

    // Input bindings

    getUi().pushButtonLearning->setEnabled(controller->isOpen());
    connect(getUi().pushButtonLearning, SIGNAL(clicked()),
            this, SLOT(slotShowLearnDialog()));

    connect(getUi().pushButtonLearning, SIGNAL(clicked()),
            this, SLOT(slotDirty()));
    // connect(m_ui.btnClearAllInputBindings, SIGNAL(clicked()),
    //         this, SLOT(clearAllInputBindings()));
    // connect(this, SIGNAL(clearInputs()),
    //         getController(), SLOT(clearInputMappings()));
    // connect(m_ui.btnClearAllInputBindings, SIGNAL(clicked()),
    //         this, SLOT(slotDirty()));
//     connect(m_ui.btnRemoveInputBinding, SIGNAL(clicked()), this, SLOT(slotRemoveInputBinding()));
//     connect(m_ui.btnRemoveInputBinding, SIGNAL(clicked()), this, SLOT(slotDirty()));
//     connect(m_ui.btnAddInputBinding, SIGNAL(clicked()), this, SLOT(slotAddInputBinding()));
//     connect(m_ui.btnAddInputBinding, SIGNAL(clicked()), this, SLOT(slotDirty()));

    // Output bindings
    // connect(m_ui.btnClearAllOutputBindings, SIGNAL(clicked()),
    //         this, SLOT(clearAllOutputBindings()));
    // connect(this, SIGNAL(clearOutputs()),
    //         getController(), SLOT(clearOutputMappings()));
    // connect(m_ui.btnClearAllOutputBindings, SIGNAL(clicked()),
    //         this, SLOT(slotDirty()));
//     connect(m_ui.btnRemoveOutputBinding, SIGNAL(clicked()), this, SLOT(slotRemoveOutputBinding()));
//     connect(m_ui.btnRemoveOutputBinding, SIGNAL(clicked()), this, SLOT(slotDirty()));
//     connect(m_ui.btnAddOutputBinding, SIGNAL(clicked()), this, SLOT(slotAddOutputBinding()));
//     connect(m_ui.btnAddOutputBinding, SIGNAL(clicked()), this, SLOT(slotDirty()));

}

void DlgPrefMappableController::slotShowLearnDialog() {
    // If the user has checked the "Enabled" checkbox but they haven't hit OK to
    // apply it yet, prompt them to apply the settings before we open the
    // learning dialog. If we don't apply the settings first and open the
    // device, the dialog won't react to controller messages.
    if (isEnabled() && !getController()->isOpen()) {
        QMessageBox::StandardButton result = QMessageBox::question(
            this,
            tr("Apply device settings?"),
            tr("Your settings must be applied before starting the learning wizard.\n"
               "Apply settings and continue?"),
            QMessageBox::Ok | QMessageBox::Cancel,  // Buttons to be displayed
            QMessageBox::Ok);  // Default button
        // Stop if the user has not pressed the Ok button,
        // which could be the Cancel or the Close Button.
        if (result != QMessageBox::Ok) {
            return;
        }
    }
    slotApply();

    // Note that DlgControllerLearning is set to delete itself on close using the
    // Qt::WA_DeleteOnClose attribute (so this "new" doesn't leak memory)
    m_pDlgControllerLearning = new DlgControllerLearning(this, getController());
    m_pDlgControllerLearning->show();
    ControllerLearningEventFilter* pControllerLearning = getControllerManager()->getControllerLearningEventFilter();
    pControllerLearning->startListening();
    connect(pControllerLearning, SIGNAL(controlClicked(ControlObject*)),
            m_pDlgControllerLearning, SLOT(controlClicked(ControlObject*)));
    connect(m_pDlgControllerLearning, SIGNAL(listenForClicks()),
            pControllerLearning, SLOT(startListening()));
    connect(m_pDlgControllerLearning, SIGNAL(stopListeningForClicks()),
            pControllerLearning, SLOT(stopListening()));
    connect(m_pDlgControllerLearning, SIGNAL(cancelLearning()),
            this, SLOT(show()));

    emit(mappingStarted());
    connect(m_pDlgControllerLearning, SIGNAL(cancelLearning()),
            this, SIGNAL(mappingEnded()));
}

void DlgPrefMappableController::slotDeviceState(int state) {
    DlgPrefController::slotDeviceState(state);
    //m_ui.toolBox->setEnabled(state == Qt::Checked);
    getUi().pushButtonLearning->setEnabled(state == Qt::Checked);
}

void DlgPrefMappableController::slotUpdate() {
    DlgPrefController::slotUpdate();
    getUi().pushButtonLearning->setEnabled(getController()->isOpen());
    //m_ui.toolBox->setEnabled(getController()->isOpen());
}

void DlgPrefMappableController::clearAllInputBindings() {
    if (QMessageBox::warning(this, tr("Clear Input Bindings"),
            tr("Are you sure you want to clear all bindings?"),
            QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel) != QMessageBox::Ok)
        return;
    emit(clearInputs());
}

void DlgPrefMappableController::clearAllOutputBindings() {
    if (QMessageBox::warning(this, tr("Clear Output Bindings"),
            tr("Are you sure you want to clear all output bindings?"),
            QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel) != QMessageBox::Ok)
        return;
    emit(clearOutputs());
}
