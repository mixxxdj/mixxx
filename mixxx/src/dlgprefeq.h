/***************************************************************************
                          dlgprefeq.h  -  description
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

#ifndef DLGPREFEQ_H
#define DLGPREFEQ_H

#include "ui_dlgprefeqdlg.h"
#include "configobject.h"
#include "engine/enginefilterblock.h"

class QWidget;
/**
  *@author John Sully
  */

class DlgPrefEQ : public QWidget, public Ui::DlgPrefEQDlg  {
    Q_OBJECT
public: 
    DlgPrefEQ(QWidget *parent, ConfigObject<ConfigValue> *_config);
    ~DlgPrefEQ();
public slots:
	void slotLoFiChanged();
    /** Update Hi EQ **/
    void slotUpdateHiEQ();
    /** Update Lo EQ **/
    void slotUpdateLoEQ();
    /** Apply changes to widget */
    void slotApply();
	void slotUpdate();
	void setDefaults();
signals:
    void apply(const QString &);
private:
	void loadSettings();
	int getEqFreq(int);
    int getSliderPosition(int eqFreq);
    void validate_levels();
    
	int m_lowEqFreq, m_highEqFreq;
    /** Pointer to config object */
    ConfigObject<ConfigValue> *config;

};

#endif
