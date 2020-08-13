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

#pragma once

#include <QWidget>

#include "preferences/dialog/ui_dlgprefnovinyldlg.h"
#include "preferences/usersettings.h"
#include "preferences/dlgpreferencepage.h"

class SoundManager;

class DlgPrefNoVinyl : public DlgPreferencePage, Ui::DlgPrefNoVinylDlg  {
    Q_OBJECT
  public:
    DlgPrefNoVinyl(QWidget *parent, SoundManager* soundman, UserSettingsPointer _config);
    virtual ~DlgPrefNoVinyl();
};
