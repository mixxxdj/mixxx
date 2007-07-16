/***************************************************************************
                          dlgprefmixer.h  -  description
                             -------------------
    begin                : Thu Jun 7 2007
    copyright            : (C) 2007 by John Sully
    email                : jsully@scs.ryerson.ca
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef DLGPREFMIXER_H
#define DLGPREFMIXER_H

#include "ui_dlgprefmixerdlg.h"
#include "configobject.h"
#include "enginefilterblock.h"

class QWidget;
/**
  *@author John Sully
  */

class DlgPrefMixer : public QWidget, public Ui::DlgPrefMixerDlg  {
    Q_OBJECT
public: 
    DlgPrefMixer(QWidget *parent, ConfigObject<ConfigValue> *_config);
    ~DlgPrefMixer();
public slots:
    /** Update Hi EQ **/
    void slotUpdateHiEQ();
    /** Update Lo EQ **/
    void slotUpdateLoEQ();
    /** Apply changes to widget */
    void slotApply();
	void slotUpdate();
signals:
    void apply(const QString &);
private:
    void setMidEQ();
    /** Pointer to config object */
    ConfigObject<ConfigValue> *config;
};

#endif
