/***************************************************************************
                          wtreelist.h  -  description
                             -------------------
    begin                : 10 02 2003
    copyright            : (C) 2003 by Ingo Kossyk
    email                : kossyki@cs.tu-berlin.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "wwidget.h"
#include <qapplication.h>
#include <qvariant.h>
#include <qheader.h>
#include <qlistview.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qaction.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qtoolbar.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qfile.h>
#include <qdir.h>
#include <qpoint.h>
#include <qstring.h>
#include <qevent.h>
#include <qstringlist.h>
#include <qmainwindow.h>
#include <qdom.h>
#include <qcolor.h>
#include <qmime.h>
#include <qdragobject.h>
#include <qmessagebox.h>
#include <qwidget.h>
#ifndef WTREELIST_H
#define WTREELIST_H

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QAction;
class QActionGroup;
class QToolBar;
class QPopupMenu;
class QMainWindow;
class QListView;
class QListViewItem;
class QDragEnterEvent;


class WTreeList : public QListView
{
	Q_OBJECT

public:
	WTreeList( QWidget *parent = 0, const char *name=0);
	~WTreeList();
	void setRoot(QString sRoot);
	WTreeItem * getRoot();
	void setPlaylist(QString sPlaylist);
	WTreeItem * getPlsRoot();
	void setup(QDomNode node);
	QString m_sPlaylistdir;
	bool mousePressed;
signals:
		void loadPls(QString);

public slots:
		void slotSetDirs(QString,QString);

protected slots:
    void slotFolderSelected( QListViewItem * );

protected:
	void contentsMouseMoveEvent( QMouseEvent *e );
	void contentsMouseReleaseEvent( QMouseEvent * );
	void contentsMousePressEvent( QMouseEvent* e );

private:
	bool deleteRoot();

	QPoint presspos;
	bool populateTree(QString dirPath, QListViewItem * listItem);
	bool populatePlaylists(QListViewItem * listItem);
	const QFileInfoList * subDirList;
	QDir * workingDir;
	QFileInfo * currentObject;
	
};

#endif
