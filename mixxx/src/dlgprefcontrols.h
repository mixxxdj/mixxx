/***************************************************************************
                          dlgprefcontrols.h  -  description
                             -------------------
    begin                : Sat Jul 5 2003
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

#ifndef DLGPREFCONTROLS_H
#define DLGPREFCONTROLS_H

#include "ui_dlgprefcontrolsdlg.h"
#include "configobject.h"
#include "mixxx.h"

class QWidget;
class ControlObjectThreadMain;
class ControlPotmeter;
class MixxxView;

/**
  *@author Tue & Ken Haste Andersen
  */

class DlgPrefControls : public QWidget, public Ui::DlgPrefControlsDlg  {
    Q_OBJECT
public: 
    DlgPrefControls(QWidget *parent, MixxxView *pView, MixxxApp *mixxx, ConfigObject<ConfigValue> *pConfig);
    ~DlgPrefControls();
public slots:
    void slotUpdate();
    void slotSetRateRange(int pos);
    void slotSetRateDir(int pos);
    void slotSetRateTempLeft(double);
    void slotSetRateTempRight(double);
    void slotSetRatePermLeft(double);
    void slotSetRatePermRight(double);
    void slotSetVisuals(int pos);
    void slotSetTooltips(int pos);
    void slotSetSkin(int);
	void slotSetScheme(int);
	void slotUpdateSchemes();
    void slotSetPositionDisplay(int);
    void slotSetCueDefault(int);
    void slotSetCueRecall(int);
    void slotSetRateRamp(bool);
    void slotSetRateRampSensitivity(int);
    void slotApply();
private:
    /** Pointer to ConfigObject */
    ConfigObject<ConfigValue> *m_pConfig;
    /** Pointers to ControlObjects associated with rate sliders */
    ControlObjectThreadMain *m_pControlRate1, *m_pControlRate2, *m_pControlRateRange1, *m_pControlRateRange2;
    /** Pointer to ControlObjects for controlling direction of rate sliders */
    ControlObjectThreadMain *m_pControlRateDir1, *m_pControlRateDir2;
    /** Pointer to ControlObjects for cue behavior */
    ControlObjectThreadMain *m_pControlCueDefault1, *m_pControlCueDefault2;
    /** Pointer to MixxxView */
    MixxxView *m_pView;
	MixxxApp *m_mixxx;
};

#endif
