/***************************************************************************
                          dlgchannel.h  -  description
                             -------------------
    begin                : Wed Feb 20 2002
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

#ifndef DLGCHANNEL_H
#define DLGCHANNEL_H

#include <qwidget.h>
#include "dlgchanneldlg.h"

/**
  *@author Tue and Ken Haste Andersen
  */

class DlgChannel : public DlgChannelDlg  {
   Q_OBJECT
public: 
	DlgChannel(QWidget *parent=0, const char *name=0);
	~DlgChannel();
    void layoutMirror();
};

#endif
