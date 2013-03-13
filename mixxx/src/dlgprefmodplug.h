/***************************************************************************
                          dlgprefmodplug.h  -  modplug settings dialog
                             -------------------
    copyright            : (C) 2013 by Stefan Nuernberger
    email                : kabelfrickler@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef DLGPREFMODPLUG_H
#define DLGPREFMODPLUG_H

#include <QDialog>
#include "configobject.h"

namespace Ui {
class DlgPrefModplug;
}

class DlgPrefModplug : public QDialog
{
    Q_OBJECT

public:
    explicit DlgPrefModplug(QWidget *parent, ConfigObject<ConfigValue> *_config);
    ~DlgPrefModplug();

public slots:
    /** Apply changes to widget */
   void slotApply();
   void slotUpdate();

   void loadSettings();
   void saveSettings();
   void applySettings();

private:
    Ui::DlgPrefModplug *ui;
    ConfigObject<ConfigValue> *config;
};

#endif // DLGPREFMODPLUG_H
