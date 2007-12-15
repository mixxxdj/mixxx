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
	void slotLoFiChanged();
    /** Update Hi EQ **/
    void slotUpdateHiEQ();
    /** Update Lo EQ **/
    void slotUpdateLoEQ();
	/** Update X-Fader */
	void slotUpdateXFader();
    /** Apply changes to widget */
    void slotApply();
	void slotUpdate();
	void setDefaults();
signals:
    void apply(const QString &);
private:
	void loadSettings();
	void drawXfaderDisplay();
	int getEqFreq(int);
	int getSliderPosition(int eqFreq);

	int m_lowEqFreq, m_highEqFreq;
    /** Pointer to config object */
    ConfigObject<ConfigValue> *config;

	QGraphicsScene *m_pxfScene;

	/** X-fader values */
	double m_transform, m_cal;
};

#endif
