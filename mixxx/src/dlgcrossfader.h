/***************************************************************************
                          dlgcrossfader.h  -  description
                             -------------------
    begin                : Sun May 12 2002
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

#ifndef DLGCROSSFADER_H
#define DLGCROSSFADER_H

#include <qwidget.h>
#include <dlgcrossfaderdlg.h>

/**
  *@author 
  */

class DlgCrossfader : public DlgCrossfaderDlg  {
   Q_OBJECT
public: 
	DlgCrossfader(QWidget *parent=0, const char *name=0);
	~DlgCrossfader();
};

#endif
