/****************************************************************************
** Form interface generated from reading ui file 'qfileview-test.ui'
**
** Created: Mon Okt 6 18:19:08 2003
**      by: The User Interface Compiler ($Id: wtreelist.h 585 2003-11-21 08:50:54Z tuehaste $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

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

#ifndef TREEDIRVIEW_H
#define TREEDIRVIEW_H

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

	

class WTreeItem : public QListViewItem
{
	//Q_OBJECT
	
	public:
		WTreeItem(QListView * parent=0, int *fileType =(int*) 0);
		WTreeItem(QListViewItem * parent, int *fileType =(int*) 0);
		~WTreeItem();
		QListView * listParent;
		bool getState();
		void setState(bool state);
		QListViewItem * itemAbove();
		int * getType();
		void setType(int * fileType);
		QString filePath;
	
	private:
		bool Expanded;
		int *Type;
	
		
};


class WTreeList : public QListView
{
	Q_OBJECT
	
	public:
	WTreeList( QWidget *parent = 0, const char *name=0);
	~WTreeList();
	void setRoot(QString sRoot);
	void setPlaylist(QString sPlaylist);
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
