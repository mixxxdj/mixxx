/***************************************************************************
                          dlgabout.h  -  description
                             -------------------
    begin                : Mon Nov 19 2007
    copyright            : (C) 2007 by Albert Santoni
    email                : gamegod at users.sf.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef DLGABOUT_H
#define DLGABOUT_H

#include "ui_dlgaboutdlg.h"
#include "configobject.h"

class QWidget;
/**
  *@author Albert Santoni
  */

class DlgAbout : public QDialog, public Ui::DlgAboutDlg  {
    Q_OBJECT
public: 
    DlgAbout(QWidget *parent);
    ~DlgAbout();
public slots:

    /** Apply changes to widget */
    void slotApply();
	void slotUpdate();
signals:
    void apply(const QString &);
private:
    /** Pointer to config object */
    //ConfigObject<ConfigValue> *config;
};

#endif
