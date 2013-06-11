/***************************************************************************
                          dlgprefvinyl.h  -  description
                             -------------------
    begin                : Thu Oct 23 2006
    copyright            : (C) 2006 by Stefan Langhammer
    email                : stefan.langhammer@9elements.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef DLGPREFVINYL_H
#define DLGPREFVINYL_H

#include "ui_dlgprefvinyldlg.h"
#include "configobject.h"
#include "vinylcontrol/vinylcontrolsignalwidget.h"
#include "controlobjectthreadmain.h"

class QWidget;
class PlayerProxy;
class ControlObject;
class ControlObjectThreadMain;
class VinylControlManager;

/**
  *@author Stefan Langhammer
  *@author Albert Santoni
  */

class DlgPrefVinyl : public QWidget, Ui::DlgPrefVinylDlg  {
    Q_OBJECT
public:
    DlgPrefVinyl(QWidget *pParent, VinylControlManager *m_pVCMan, ConfigObject<ConfigValue> *_config);
    ~DlgPrefVinyl();

public slots:
    /** Update widget */
    void slotUpdate();
    void slotApply();
    void VinylTypeSlotApply();
    void VinylGainSlotApply();
    void slotClose();
    void slotShow();

signals:
private:
    VinylControlSignalWidget m_signalWidget1;
    VinylControlSignalWidget m_signalWidget2;


    /** Pointer to player device */
    //PlayerProxy *player;
    VinylControlManager* m_pVCManager;
    /** Pointer to config object */
    ConfigObject<ConfigValue> *config;
    ControlObjectThreadMain m_COSpeed1;
    ControlObjectThreadMain m_COSpeed2;
};

#endif
