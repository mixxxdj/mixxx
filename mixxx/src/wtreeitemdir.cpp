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

WTreeItemDir::WTreeItemDir(Q3ListView *parent, const QString& filename) : WTreeItem(parent), f(filename), showDirsOnly(((WTreeView*)parent)->showDirsOnly())
{
    folderLocked = 0;
    folderClosed = 0;
    folderOpen = 0;
    fileNormal = 0;
    p = 0;
    readable = QDir(fullName()).isReadable();

//    setPixmap(new QPixmap(QString("C:\Documents and Settings\Tue\My Documents\cvs\mixxx\src\icons\unknown.png")));
}

WTreeItemDir::WTreeItemDir(WTreeItemDir *parent, const QString& filename, const QString &col2) : WTreeItem( parent, filename, col2 )
{
    folderLocked = 0;
    folderClosed = 0;
    folderOpen = 0;
    fileNormal = 0;

//    setPixmap(new QPixmap(QString("C:\Documents and Settings\Tue\My Documents\cvs\mixxx\src\icons\unknown.png")));
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
//    setPixmap(new QPixmap(QString("C:\Documents and Settings\Tue\My Documents\cvs\mixxx\src\icons\unknown.png")));
}

WTreeItemDir::~WTreeItemDir()
{
}

void WTreeItemDir::setOpen(bool o)
{
    int i;
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
        //        const QFileInfoList * files = thisDir.entryInfoList();

        if (!thisDir.entryInfoList().isEmpty()) {
          QFileInfoList list = thisDir.entryInfoList();

            for (int i = 0; i < list.size(); ++i) {
                QFileInfo fi = list.at(i);
                if ( fi.fileName() == "." || fi.fileName() == ".." )
                    ; // nothing
                else if ( fi.isSymLink() && !showDirsOnly ) {
                     WTreeItemFile *item = new WTreeItemFile( this, fi.fileName(),
                                                              "Symbolic Link" );
//                    item->setPixmap( fileNormal );
                }
                else if ( fi.isDir() )
                    (void)new WTreeItemDir( this, fi.fileName() );
                else if (!showDirsOnly &&
                         (fi.fileName().endsWith(".mp3") ||
                          fi.fileName().endsWith(".ogg") ||
                          fi.fileName().endsWith(".wav") ||
			  fi.fileName().endsWith(".aif") ||
			  fi.fileName().endsWith(".aiff") ||
			  fi.fileName().endsWith(".Mp3") ||
                          fi.fileName().endsWith(".Ogg") ||
                          fi.fileName().endsWith(".Wav") ||
                          fi.fileName().endsWith(".Aif") ||
                          fi.fileName().endsWith(".Aiff") ||
                          fi.fileName().endsWith(".MP3") ||
                          fi.fileName().endsWith(".OGG") ||
                          fi.fileName().endsWith(".WAV") ||
			  fi.fileName().endsWith(".AIF") ||
			  fi.fileName().endsWith(".AIFF")))
                {
                     WTreeItemFile *item = new WTreeItemFile(this, fi.fileName(),
                                                             fi.isFile()?"File":"Special" );
//                    item->setPixmap( fileNormal );
                }
            }
        }
        listView()->setUpdatesEnabled( TRUE );
    }
    Q3ListViewItem::setOpen( o );
}


void WTreeItemDir::setup()
{
    setExpandable(true);
    Q3ListViewItem::setup();
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
}

