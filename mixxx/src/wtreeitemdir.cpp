//
// C++ Implementation: wtreeitemdir
//
// Description:
//
//
// Author: Tue Haste Andersen <haste@diku.dk>, (C) 2003
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "wtreeitemdir.h"
#include "wtreeitemfile.h"
#include "wtreeview.h"

WTreeItemDir::WTreeItemDir(QListView *parent, const QString& filename) : WTreeItem(parent), f(filename), showDirsOnly(((WTreeView*)parent)->showDirsOnly())
{
    folderLocked = 0;
    folderClosed = 0;
    folderOpen = 0;
    fileNormal = 0;
    p = 0;
    readable = QDir(fullName()).isReadable();
}

WTreeItemDir::WTreeItemDir(WTreeItemDir *parent, const QString& filename, const QString &col2) : WTreeItem( parent, filename, col2 )
{
    folderLocked = 0;
    folderClosed = 0;
    folderOpen = 0;
    fileNormal = 0;
}

WTreeItemDir::WTreeItemDir(WTreeItemDir *parent, const QString& filename) : WTreeItem( parent ), f(filename), showDirsOnly(parent->showDirsOnly)
{
    folderLocked = 0;
    folderClosed = 0;
    folderOpen = 0;
    fileNormal = 0;
    p = parent;
    readable = QDir(fullName()).isReadable();

/*
    if (!readable)
        setPixmap(folderLocked);
    else
        setPixmap(folderClosed);
*/
}

WTreeItemDir::~WTreeItemDir()
{
}

void WTreeItemDir::setOpen(bool o)
{
/*
    if ( o )
        setPixmap( folderOpen );
    else
        setPixmap( folderClosed );
*/

    if ( o && !childCount() ) {
        QString s( fullName() );
        QDir thisDir( s );
        if ( !thisDir.isReadable() ) {
            readable = FALSE;
            setExpandable( FALSE );
            return;
        }

        listView()->setUpdatesEnabled( FALSE );
        const QFileInfoList * files = thisDir.entryInfoList();
        if ( files ) {
            QFileInfoListIterator it( *files );
            QFileInfo * fi;
            while( (fi=it.current()) != 0 ) {
                ++it;
                if ( fi->fileName() == "." || fi->fileName() == ".." )
                    ; // nothing
                else if ( fi->isSymLink() && !showDirsOnly ) {
                    WTreeItemFile *item = new WTreeItemFile( this, fi->fileName(),
                                                     "Symbolic Link" );
//                    item->setPixmap( fileNormal );
                }
                else if ( fi->isDir() )
                    (void)new WTreeItemDir( this, fi->fileName() );
                else if (!showDirsOnly &&
                         (fi->fileName().endsWith(".mp3", false) ||
                          fi->fileName().endsWith(".ogg", false) ||
                          fi->fileName().endsWith(".wav", false)))
                {
                    WTreeItemFile *item = new WTreeItemFile(this, fi->fileName(),
                                                            fi->isFile()?"File":"Special" );
//                    item->setPixmap( fileNormal );
                }
            }
        }
        listView()->setUpdatesEnabled( TRUE );
    }
    QListViewItem::setOpen( o );
}


void WTreeItemDir::setup()
{
    setExpandable(true);
    QListViewItem::setup();
}


QString WTreeItemDir::fullName()
{
    QString s;
    if ( p ) {
        s = p->fullName();
        s.append( f.name() );
        s.append( "/" );
    } else {
        s = f.name();
    }
    return s;
}


QString WTreeItemDir::text( int column ) const
{
    if ( column == 0 )
        return f.name();
    else if ( readable )
        return "Directory";
    else
        return "Unreadable Directory";
}

void WTreeItemDir::popupMenu()
{
    qDebug("popup dir");
}

