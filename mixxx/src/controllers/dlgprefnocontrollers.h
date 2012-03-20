/***************************************************************************
                          dlgprefnocontrollers.h  -  "No controllers available"
                             -------------------
    begin                : Thu Apr 17 2003
    copyright            : (C) 2003 by Tue & Ken Haste Andersen
    email                : haste@diku.dk
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef DLGPREFNOCONTROLLERS_H
#define DLGPREFNOCONTROLLERS_H

#include "controllers/ui_dlgprefnocontrollersdlg.h"
#include "configobject.h"

//class QWidget;

class DlgPrefNoControllers : public QWidget, public Ui::DlgPrefNoControllersDlg  {
    Q_OBJECT
public:
    DlgPrefNoControllers(QWidget *parent, ConfigObject<ConfigValue> *_config);
    ~DlgPrefNoControllers();
};

#endif
