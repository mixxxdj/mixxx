/***************************************************************************
                          dlgplaylist.h  -  description
                             -------------------
    begin                : Mon Feb 25 2002
    copyright            : (C) 2002 by Tue and Ken Haste Andersen
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

#ifndef DLGPLAYLIST_H
#define DLGPLAYLIST_H

#include <qwidget.h>
#include "dlgplaylistdlg.h"

/**
  *@author Tue and Ken Haste Andersen
  */

class DlgPlaylist : public DlgPlaylistDlg  {
   Q_OBJECT
public: 
	DlgPlaylist(QWidget *parent=0, const char *name=0);
	~DlgPlaylist();
};

#endif
