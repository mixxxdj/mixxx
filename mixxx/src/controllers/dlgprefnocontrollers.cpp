/***************************************************************************
                          dlgprefnocontrollers.cpp  -  
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "dlgprefnocontrollers.h"
#include <QtCore>
#include <QtGui>

DlgPrefNoControllers::DlgPrefNoControllers(QWidget * parent, ConfigObject<ConfigValue> * _config) :  QWidget(parent), Ui::DlgPrefNoControllersDlg()
{
    setupUi(this);
}

DlgPrefNoControllers::~DlgPrefNoControllers()
{
}