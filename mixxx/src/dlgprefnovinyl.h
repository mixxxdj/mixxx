/***************************************************************************
                          dlgprefnovinyl.h  -  description
                             -------------------
    begin                : Thu Feb 24 2011
    copyright            : (C) 2011 by Owen Williams
    email                : owen-bugs@ywwg.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef DLGPREFNOVINYL_H
#define DLGPREFNOVINYL_H

#include "ui_dlgprefnovinyldlg.h"
#include "configobject.h"

class QWidget;
class SoundManager;

/**
  *@author Stefan Langhammer
  *@author Albert Santoni
  */

class DlgPrefNoVinyl : public QWidget, Ui::DlgPrefNoVinylDlg  {
    Q_OBJECT
public:
    DlgPrefNoVinyl(QWidget *parent, SoundManager* soundman, ConfigObject<ConfigValue> *_config);
    ~DlgPrefNoVinyl();
};

#endif
