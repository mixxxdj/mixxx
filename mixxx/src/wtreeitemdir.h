//
// C++ Interface: wtreeitemdir
//
// Description: 
//
//
// Author: Tue Haste Andersen <haste@diku.dk>, (C) 2003
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef WTREEITEMDIR_H
#define WTREEITEMDIR_H

#include <qdir.h>
#include "wtreeitem.h"
/**
@author Tue Haste Andersen
*/
class WTreeItemDir : public WTreeItem
{
public:
    WTreeItemDir(QListView *parent, const QString& filename);
    WTreeItemDir(WTreeItemDir *parent, const QString& filename, const QString &col2);
    WTreeItemDir(WTreeItemDir *parent, const QString& filename);
    ~WTreeItemDir();
    QString text(int column) const;
    QString fullName();
    void setOpen(bool);
    void setup();
    void popupMenu();

private:
    QFile f;
    WTreeItemDir *p;
    bool readable;
    bool showDirsOnly;

    QPixmap *folderLocked;
    QPixmap *folderClosed;
    QPixmap *folderOpen;
    QPixmap *fileNormal;

};

#endif
