/***************************************************************************
                          dlgprefshoutcast.h  -  description
                             -------------------
    begin                : Thu Jun 7 2007
    copyright            : (C) 2008 by Wesley Stessens
                           (C) 2007 by John Sully
                           (C) 2008 by Albert Santoni
    email                :
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef DLGPREFSHOUTCAST_H
#define DLGPREFSHOUTCAST_H

#include "ui_dlgprefshoutcastdlg.h"
#include "controlobject.h"
#include "configobject.h"
#include "controlobjectthreadmain.h"
#include "shoutcast/defs_shoutcast.h"

class QWidget;
/**
  *@author John Sully
  */

class DlgPrefShoutcast : public QWidget, public Ui::DlgPrefShoutcastDlg  {
    Q_OBJECT
public:
    DlgPrefShoutcast(QWidget *parent, ConfigObject<ConfigValue> *_config);
    ~DlgPrefShoutcast();
public slots:
    /** Apply changes to widget */
    void slotApply();
    void slotUpdate(); //Does nothing but keep convention (and DlgPreferences happy)

signals:
    void apply(const QString &);
private:
    /** Pointer to config object */
    ConfigObject<ConfigValue> *m_pConfig;
    ControlObjectThreadMain* m_pUpdateShoutcastFromPrefs; //If set to 1, EngineShoutcast will update it's settings.

};

#endif
