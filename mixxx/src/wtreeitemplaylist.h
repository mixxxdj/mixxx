//
// C++ Interface: wtreeitemplaylist
//
// Description: 
//
//
// Author: Tue Haste Andersen <haste@diku.dk>, (C) 2003
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef WTREEITEMPLAYLIST_H
#define WTREEITEMPLAYLIST_H

#include "wtreeitem.h"

/**
@author Tue Haste Andersen
*/
class WTreeItemPlaylist : public QObject, WTreeItem
{
    Q_OBJECT
public:
    WTreeItemPlaylist(WTreeItem *parent, const QString &s1);
    ~WTreeItemPlaylist();

    void popupMenu();
    void drag(QWidget *viewport);
signals:
    void playlistPopup();
};

#endif
