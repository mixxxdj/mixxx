/***************************************************************************
                          dlgprefplaylist.h  -  description
                             -------------------
    begin                : Thu Apr 17 2003
    copyright            : (C) 2003 by Tue & Ken Haste Andersen
    email                : haste@diku.dk
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef DLGPREFPLAYLIST_H
#define DLGPREFPLAYLIST_H

#include <qwidget.h>
#include "dlgprefplaylistdlg.h"
#include "configobject.h"

/**
  *@author Tue & Ken Haste Andersen
  */

class DlgPrefPlaylist : public DlgPrefPlaylistDlg  {
    Q_OBJECT
public: 
    DlgPrefPlaylist(QWidget *parent, ConfigObject<ConfigValue> *_config);
    ~DlgPrefPlaylist();
public slots:
    /** Update widget */
    void slotUpdate();
    /** Dialog to browse for music file directory */
    void slotBrowseDir();
    /** Dialog to browse for Playlist directory */
	void slotBrowsePlaydir();
	/** Apply changes to widget */
    void slotApply();
signals:
    void apply(QString, QString);
private:
    /** Pointer to config object */
    ConfigObject<ConfigValue> *config;
};

#endif
