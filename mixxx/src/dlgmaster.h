/***************************************************************************
                          dlgmaster.h  -  description
                             -------------------
    begin                : Sun Apr 28 2002
    copyright            : (C) 2002 by 
    email                : 
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef DLGMASTER_H
#define DLGMASTER_H

#include <qwidget.h>
#include <dlgmasterdlg.h>

/**
  *@author 
  */

class DlgMaster : public DlgMasterDlg  {
   Q_OBJECT
public: 
	DlgMaster(QWidget *parent=0, const char *name=0);
	~DlgMaster();
};

#endif
