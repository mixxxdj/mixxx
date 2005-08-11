//
// C++ Implementation: wtree
//
// Description:
//
//
// Author: Tue Haste Andersen <haste@diku.dk>, (C) 2003
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "wtreeview.h"
#include "xmlparse.h"
#include "wtreeitemdir.h"
#include "wtreeitemplaylist.h"
#include "wtreeitemplaylistroot.h"
#include "wtreeitem.h"
#include "trackplaylist.h"
#include <qdragobject.h>

static const int autoopenTime = 750;

WTreeView::WTreeView(QString qRootPath, QWidget *parent, const char *name, bool sdo) : QListView( parent, name ), dirsOnly( sdo ), oldCurrent( 0 ),
      dropItem(0), mousePressed(false)
{
    folderLocked = 0;
    folderClosed = 0;
    folderOpen = 0;
    fileNormal = 0;
    m_pClickedItem = 0;
    m_pRootDir = 0;

    autoopen_timer = new QTimer(this);

/*
 * if (!folderLocked)
    {
        folderLocked = new QPixmap( folder_locked );
        folderClosed = new QPixmap( folder_closed_xpm );
        folderOpen = new QPixmap( folder_open_xpm );
        fileNormal = new QPixmap( pix_file );
    }
*/
    connect(this, SIGNAL(doubleClicked(QListViewItem *)), this, SLOT(slotFolderSelected(QListViewItem *)));
    connect(this, SIGNAL(returnPressed(QListViewItem *)), this, SLOT(slotFolderSelected(QListViewItem *)));

    // Set properties
    setAcceptDrops(false);
    //viewport()->setAcceptDrops(true);
    addColumn("Name", -1);
    setSelectionMode(QListView::Extended);
    setResizeMode(QListView::AllColumns);
//    header()->hide();
    
    setColumnWidthMode(1, QListView::Maximum);
    setTreeStepSize(20);
    
    setFrameStyle(QFrame::NoFrame);
    connect(autoopen_timer, SIGNAL(timeout()), this, SLOT(openFolder()));

    //
    // Load information into the view
    //
    slotUpdateDir(qRootPath);

    // Insert root playlist
    m_pRootPlaylist = new WTreeItemPlaylistRoot(this, "Playlists");
    m_pRootPlaylist->setOpen(true); // be interesting
}

WTreeView::~WTreeView()
{
}

void WTreeView::setup(QDomNode node)
{

    // Position
    if (!XmlParse::selectNode(node, "Pos").isNull())
    {
        QString pos = XmlParse::selectNodeQString(node, "Pos");
        int x = pos.left(pos.find(",")).toInt();
        int y = pos.mid(pos.find(",")+1).toInt();
        move(x,y);
    }

    // Size
    if (!XmlParse::selectNode(node, "Size").isNull())
    {
        QString size = XmlParse::selectNodeQString(node, "Size");
        int x = size.left(size.find(",")).toInt();
        int y = size.mid(size.find(",")+1).toInt();
        setBaseSize(x,y);
    }

    // Background color
    if (!XmlParse::selectNode(node, "BgColor").isNull())
    {
        QColor c;
        c.setNamedColor(XmlParse::selectNodeQString(node, "BgColor"));
        setPaletteBackgroundColor(c);
    }

    // Foreground color
    if (!XmlParse::selectNode(node, "FgColor").isNull())
    {
        QColor c;
        c.setNamedColor(XmlParse::selectNodeQString(node, "FgColor"));
        setPaletteForegroundColor(c);
    }
}

void WTreeView::slotFolderSelected( QListViewItem *i )
{
    // Abort any rename click which is about to be executed
    m_pClickedItem = 0;

    if (!i)
        return;

    WTreeItem *t = (WTreeItem *)i;

    if (t->type()=="WTreeItemDir" && showDirsOnly())
    {
        WTreeItemDir *dir = (WTreeItemDir*)t;
        emit folderSelected( dir->fullName() );
    }
    else if (t->type() == "WTreeItemPlaylist")
    {
        WTreeItemPlaylist *list = (WTreeItemPlaylist*)t;
        emit activatePlaylist(list->name());
    }
}

void WTreeView::openFolder()
{
    autoopen_timer->stop();
    if ( dropItem && !dropItem->isOpen() )
    {
        dropItem->setOpen( TRUE );
        dropItem->repaint();
    }
}

void WTreeView::updatePlaylists(QPtrList<TrackPlaylist> *pList)
{
    // Clear current lists
    while (m_pRootPlaylist->childCount()>0)
        delete m_pRootPlaylist->firstChild();

    // Insert new entries
    for (uint i=0; i<pList->count(); ++i)
        new WTreeItemPlaylist(m_pRootPlaylist, pList->at(i));
}

void WTreeView::contentsDragEnterEvent( QDragEnterEvent *e )
{
    if (!QUriDrag::canDecode(e))
    {
        e->ignore();
        return;
    }

    oldCurrent = currentItem();

    QListViewItem *i = itemAt(contentsToViewport(e->pos()));
    if (i)
    {
        dropItem = i;
        autoopen_timer->start( autoopenTime );
    }
}


void WTreeView::contentsDragMoveEvent( QDragMoveEvent *e )
{
    if ( !QUriDrag::canDecode(e) )
    {
        e->ignore();
        return;
    }

    QPoint vp = contentsToViewport( ( (QDragMoveEvent*)e )->pos() );
    QListViewItem *i = itemAt( vp );
    if ( i )
    {
        setSelected( i, TRUE );
        e->accept();
        if ( i != dropItem )
        {
            autoopen_timer->stop();
            dropItem = i;
            autoopen_timer->start( autoopenTime );
        }
        switch ( e->action() )
        {
        case QDropEvent::Copy:
            break;
        case QDropEvent::Move:
            e->acceptAction();
            break;
        case QDropEvent::Link:
            e->acceptAction();
            break;
        default:
            ;
        }
    }
    else
    {
        e->ignore();
        autoopen_timer->stop();
        dropItem = 0;
    }
}

void WTreeView::contentsDragLeaveEvent( QDragLeaveEvent * )
{
    autoopen_timer->stop();
    dropItem = 0;

    setCurrentItem( oldCurrent );
    setSelected( oldCurrent, TRUE );
}

void WTreeView::contentsDropEvent( QDropEvent *e )
{
    autoopen_timer->stop();

    if ( !QUriDrag::canDecode(e) )
    {
        e->ignore();
        return;
    }

    QListViewItem *item = itemAt( contentsToViewport(e->pos()) );
    if ( item )
    {

        QStrList lst;

        QUriDrag::decode( e, lst );

        QString str;

        switch ( e->action() )
        {
            case QDropEvent::Copy:
            str = "Copy";
            break;
            case QDropEvent::Move:
            str = "Move";
            e->acceptAction();
            break;
            case QDropEvent::Link:
            str = "Link";
            e->acceptAction();
            break;
            default:
            str = "Unknown";
        }

        str += "\n\n";

        e->accept();

        for ( uint i = 0; i < lst.count(); ++i )
        {
            QString filename = lst.at( i );
            str += filename + "\n";
        }
        str += QString( "\nTo\n\n   %1" )
               .arg( fullPath(item) );

        //QMessageBox::information( this, "Drop target", str, "Not implemented" );
    }
    else
        e->ignore();

}


QString WTreeView::fullPath(QListViewItem* item)
{
    QString fullpath = item->text(0);
    while ( (item=item->parent()) )
    {
        if ( item->parent() )
            fullpath = item->text(0) + "/" + fullpath;
        else
            fullpath = item->text(0) + fullpath;
    }
    return fullpath;
}

void WTreeView::contentsMousePressEvent( QMouseEvent* e )
{
    // Stop the timer, so a rename click is not issued
    mouseMoved = false;

    QListView::contentsMousePressEvent(e);
    QPoint p( contentsToViewport( e->pos() ) );
    
    WTreeItem *i = (WTreeItem *)itemAt( p );
    if ( i )
    {
        m_pClickedItem = i;
     
        // If the user right clicked, bring up a popup menu
        if (e->button()==Qt::RightButton)
            m_pClickedItem->popupMenu();
        else
        {
            // if the user clicked into the root decoration of the item, don't try to start a drag!
            if ( p.x() > header()->cellPos( header()->mapToActual( 0 ) ) +
                 treeStepSize() * ( i->depth() + ( rootIsDecorated() ? 1 : 0) ) + itemMargin() ||
                 p.x() < header()->cellPos( header()->mapToActual( 0 ) ) )
            {
                presspos = e->pos();
                mousePressed = TRUE;
            }
        }
    }
}

void WTreeView::contentsMouseMoveEvent( QMouseEvent* e )
{
    if ( mousePressed && ( presspos - e->pos() ).manhattanLength() > QApplication::startDragDistance() )
    {
        mousePressed = FALSE;

        // The item the mouse is over decides if we are dragging a playlist or
        // file/dirs...
        WTreeItem *item = (WTreeItem *)itemAt(contentsToViewport(presspos));
        if (item && item->type()=="WTreeItemPlaylist")
        {
            QTextDrag *td = new QTextDrag(item->drag(), viewport());
            const QCString type("Playlist");
            td->setSubtype(type);
            td->dragCopy();
        }
        else
        {
            QStrList lst;
            item = (WTreeItem *)m_pRootDir;
            while (item)
            {
                if (item->isSelected())
                    lst.append(item->drag());
                item = (WTreeItem *)item->itemBelow();
            }
            QUriDrag *ud = new QUriDrag(viewport());
            ud->setUris(lst);
            ud->dragCopy();
        }
    }
    mouseMoved = true;
}

void WTreeView::contentsMouseReleaseEvent(QMouseEvent *)
{
    mousePressed = false;
    mouseMoved = false;
}

void WTreeView::slotUpdateDir( const QString &s )
{
    QString dir = s;

    // Ensure that the dir ends with a separator
    if (!(dir.endsWith(QString(QChar(QDir::separator()))) || dir.endsWith(QString("/"))))
        dir += QChar(QDir::separator());
        
    // Clear current dir
    if (m_pRootDir)
        delete m_pRootDir;
    
    m_pRootDir = new WTreeItemDir(this, dir);
}

void WTreeView::slotHighlightPlaylist(TrackPlaylist *p)
{
    // Find playlist and highlight
    QListViewItem *it = m_pRootPlaylist->firstChild();
    while (it)
    {
        if (it->text(0)==p->getListName())
        {
            ensureItemVisible(it);
            setCurrentItem(it);    
            break;
        }
        it = it->nextSibling();
    }
}
