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

#include <qwidget.h>
#include "dlgprefcontrolsdlg.h"
#include "configobject.h"

class ControlObject;
class ControlPotmeter;
class MixxxView;

/**
  *@author Tue & Ken Haste Andersen
  */

class DlgPrefControls : public DlgPrefControlsDlg  {
    Q_OBJECT
public: 
    DlgPrefControls(QWidget *parent, ControlObject *pControl, MixxxView *pView,
                    ConfigObject<ConfigValue> *pConfig);
    ~DlgPrefControls();
public slots:
    void slotUpdate();
    void slotSetRateRange(int pos);
    void slotSetRateDir(int pos);
    void slotSetVisuals(int pos);
    void slotSetSkin(int);
    void slotSetPositionDisplay(int);
    void slotApply();
private:
    /** Pointer to ConfigObject */
    ConfigObject<ConfigValue> *m_pConfig;
    /** Pointers to ControlObjects associated with rate sliders */
    ControlPotmeter *m_pControlRate1, *m_pControlRate2;
    /** Pointer to ControlObjects for controlling direction of rate sliders */
    ControlObject *m_pControlRateDir1, *m_pControlRateDir2;
    /** Pointer to MixxxView */
    MixxxView *m_pView;
};

#endif
