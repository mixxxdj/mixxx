/***************************************************************************
                          dlgprefnocontrollers.h  -  "No controllers available"
                             -------------------
    begin                : Thu Apr 17 2003
    copyright            : (C) 2003 by Tue & Ken Haste Andersen
    email                : haste@diku.dk
 ***************************************************************************/
#ifndef DLGPREFNOCONTROLLERS_H
#define DLGPREFNOCONTROLLERS_H

#include <QWidget>

#include "controllers/ui_dlgprefnocontrollersdlg.h"
#include "configobject.h"

class DlgPrefNoControllers : public QWidget, public Ui::DlgPrefNoControllersDlg  {
    Q_OBJECT
  public:
    DlgPrefNoControllers(QWidget *parent, ConfigObject<ConfigValue> *_config);
    virtual ~DlgPrefNoControllers();
};

#endif
