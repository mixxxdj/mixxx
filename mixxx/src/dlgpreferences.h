/***************************************************************************
                          dlgpreferences.h  -  description
                             -------------------
    begin                : Sun Jun 30 2002
    copyright            : (C) 2002 by Tue & Ken Haste Andersen
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

#ifndef DLGPREFERENCES_H
#define DLGPREFERENCES_H

#include <qwidget.h>
#include <dlgpreferencesdlg.h>

/**
  *@author Tue & Ken Haste Andersen
  */

class DlgPreferences : public DlgPreferencesDlg  {
   Q_OBJECT
public: 
	DlgPreferences(QWidget *parent=0, const char *name=0);
	~DlgPreferences();
};

#endif
