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

#include <QWidget>

#include "ui_dlgprefvinyldlg.h"
#include "configobject.h"
#include "vinylcontrol/vinylcontrolsignalwidget.h"
#include "controlobjectthreadmain.h"
#include "preferences/dlgpreferencepage.h"

class VinylControlManager;

class DlgPrefVinyl : public DlgPreferencePage, Ui::DlgPrefVinylDlg  {
    Q_OBJECT
  public:
    DlgPrefVinyl(QWidget* pParent, VinylControlManager* m_pVCMan, ConfigObject<ConfigValue>* _config);
    virtual ~DlgPrefVinyl();

  public slots:
    void slotUpdate();
    void slotApply();
    void slotHide();
    void slotShow();
    void VinylTypeSlotApply();
    void VinylGainSlotApply();

  private:
    VinylControlSignalWidget m_signalWidget1;
    VinylControlSignalWidget m_signalWidget2;

    VinylControlManager* m_pVCManager;
    ConfigObject<ConfigValue>* config;
    ControlObjectThread m_COSpeed1;
    ControlObjectThread m_COSpeed2;
};

#endif
