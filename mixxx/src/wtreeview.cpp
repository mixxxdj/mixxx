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
#include "wtreeitem.h"

static const int autoopenTime = 750;

WTreeView::WTreeView(QString qRootPath, QWidget *parent, const char *name, bool sdo) : QListView( parent, name ), dirsOnly( sdo ), oldCurrent( 0 ),
      dropItem( 0 ), mousePressed( FALSE )
{
    folderLocked = 0;
    folderClosed = 0;
    folderOpen = 0;
    fileNormal = 0;

    autoopen_timer = new QTimer( this );
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
    //setColumnWidthMode(1, QListView::Maximum);
    //setTreeStepSize(20);
    setFrameStyle(QFrame::NoFrame);
    connect(autoopen_timer, SIGNAL(timeout()), this, SLOT(openFolder()));

    //
    // Load information into the view
    //
    new WTreeItemDir(this, qRootPath);

    // Insert root playlist
    m_pRootPlaylist = new WTreeItem(this, "Playlists");
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
    if (!i || !showDirsOnly())
        return;

    WTreeItemDir *dir = (WTreeItemDir*)i;
    emit folderSelected( dir->fullName() );
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

void WTreeView::updatePlaylists(QStrList qPlaylists)
{
    // Clear current lists
    while (m_pRootPlaylist->childCount()>0)
        delete m_pRootPlaylist->firstChild();

    // Insert new entries
    for (uint i=0; i<qPlaylists.count(); ++i)
        new WTreeItemPlaylist(m_pRootPlaylist, qPlaylists.at(i));
}

void WTreeView::contentsDragEnterEvent( QDragEnterEvent *e )
{
    if ( !QUriDrag::canDecode(e) )
    {
        e->ignore();
        return;
    }

    oldCurrent = currentItem();

    QListViewItem *i = itemAt( contentsToViewport(e->pos()) );
    if ( i )
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

        QMessageBox::information( this, "Drop target", str, "Not implemented" );
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
    QListView::contentsMousePressEvent(e);
    QPoint p( contentsToViewport( e->pos() ) );
    WTreeItem *i = (WTreeItem *)itemAt( p );
    if ( i )
    {
        // If the user right clicked, bring up a popup menu
        if (e->button()==Qt::RightButton)
        {
            emit(playlistPopup(i->text(0)));
        }
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
        WTreeItem *item = (WTreeItem *)itemAt( contentsToViewport(presspos) );
        if (item)
            item->drag(viewport());
    }
}

void WTreeView::contentsMouseReleaseEvent( QMouseEvent * )
{
    mousePressed = FALSE;
}

void WTreeView::setDir( const QString &s )
{
    QListViewItemIterator it( this );
    ++it;
    for ( ; it.current(); ++it )
    {
        it.current()->setOpen( FALSE );
    }

    QStringList lst( QStringList::split( "/", s ) );
    QListViewItem *item = firstChild();
    QStringList::Iterator it2 = lst.begin();
    for ( ; it2 != lst.end(); ++it2 )
    {
        while ( item )
        {
            if ( item->text( 0 ) == *it2 )
            {
                item->setOpen( TRUE );
                break;
            }
            item = item->itemBelow();
        }
    }

    if ( item )
        setCurrentItem( item );
}
