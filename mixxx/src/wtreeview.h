//
// C++ Interface: wtree
//
// Description: 
//
//
// Author: Tue Haste Andersen <haste@diku.dk>, (C) 2003. Based on QT example
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef WTREEVIEW_H
#define WTREEVIEW_H

#include <qlistview.h>
#include <qstring.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qtimer.h>
#include <qdom.h>
#include <qdir.h>
#include <qpixmap.h>
#include <qevent.h>
#include <qpoint.h>
#include <qmessagebox.h>
#include <qdragobject.h>
#include <qmime.h>
#include <qptrlist.h>
#include <qstringlist.h>
#include <qapplication.h>
#include <qheader.h>


class QWidget;
class QDragEnterEvent;
class QDragMoveEvent;
class QDragLeaveEvent;
class QDropEvent;
class WTreeItem;
class TrackPlaylist;

/**
@author Tue Haste Andersen
*/

class WTreeView : public QListView
{
    Q_OBJECT
public:
    WTreeView(QString qRootPath, QWidget *parent=0, const char *name=0, bool sdo=FALSE);
    ~WTreeView();
    /** Setup object based on XML description */
    void setup(QDomNode node);
    bool showDirsOnly() { return dirsOnly; }
    /** Updates the playlists entries to match the names given in the string list */
    void updatePlaylists(QPtrList<TrackPlaylist> *pList);
    static QString fullPath(QListViewItem* item);
public slots:
    void slotUpdateDir(const QString &);
    void slotHighlightPlaylist(TrackPlaylist *);
    
signals:
    void folderSelected(const QString &);
    void playlistPopup(QString);
    void activatePlaylist(QString);

protected slots:
    void slotFolderSelected(QListViewItem *);
    void openFolder();
    void slotRenameItem();
   
protected:
    void contentsDragEnterEvent(QDragEnterEvent *e);
    void contentsDragMoveEvent(QDragMoveEvent *e);
    void contentsDragLeaveEvent(QDragLeaveEvent *e);
    void contentsDropEvent(QDropEvent *e);
    void contentsMouseMoveEvent(QMouseEvent *e);
    void contentsMousePressEvent(QMouseEvent *e);
    void contentsMouseReleaseEvent(QMouseEvent *e);

private:
    /** Root item for playlists */
    WTreeItem *m_pRootPlaylist;
    /** Root item for directory structure */
    WTreeItem *m_pRootDir;    
    bool dirsOnly;
    QListViewItem *oldCurrent;
    QListViewItem *dropItem;
    QTimer *autoopen_timer;
    /** Item of last clicked item */
    WTreeItem *m_pClickedItem;

    QPoint presspos;
    bool mousePressed, mouseMoved;

    QPixmap *folderLocked;
    QPixmap *folderClosed;
    QPixmap *folderOpen;
    QPixmap *fileNormal;
};

#endif
