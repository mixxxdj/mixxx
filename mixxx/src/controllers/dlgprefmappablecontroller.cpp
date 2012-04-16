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

DlgPrefMappableController::DlgPrefMappableController(QWidget *parent, Controller* controller,
                          ControllerManager* controllerManager,
                          ConfigObject<ConfigValue> *pConfig)
    : DlgPrefController(parent, controller, controllerManager, pConfig)
{
    // Remove the vertical spacer since we're adding stuff
    layout->removeItem(verticalSpacer);
    // Add our supplemental UI here
    QWidget * containerWidget = new QWidget(this);
    m_ui.setupUi(containerWidget);
    layout->addWidget(containerWidget);
}