/***************************************************************************
                          mixxxview.h  -  description
                             -------------------
    begin                : Mon Feb 18 09:48:17 CET 2002
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

#ifndef MIXXXVIEW_H
#define MIXXXVIEW_H

// include files for QT
#include <qwidget.h>
#include <qlayout.h>

// application specific includes
#include "mixxxdoc.h"
#include "dlgplaycontrol.h"
#include "dlgchannel.h"
#include "dlgplaylist.h"

/**
 * This class provides an incomplete base for your application view. 
 */

class MixxxView : public QWidget
{
  Q_OBJECT
  public:
    MixxxView(QWidget *parent=0, MixxxDoc* doc=0);
    ~MixxxView();
    DlgPlaycontrol *playcontrol;
	DlgChannel	*channel;
	DlgPlaylist *playlist;

  protected slots:
    void slotDocumentChanged();
  private:
	//QHBoxLayout *hMainBox;
	QGridLayout *mainGrid;

};

#endif
