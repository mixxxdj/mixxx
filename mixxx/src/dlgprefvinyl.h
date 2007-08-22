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
#include "soundmanager.h"

class QWidget;
class PlayerProxy;
class ControlObject;

/**
  *@author Stefan Langhammer
  *@author Albert Santoni
  */

class DlgPrefVinyl : public QWidget, Ui::DlgPrefVinylDlg  {
    Q_OBJECT
public:
    DlgPrefVinyl(QWidget *parent, SoundManager* soundman, ConfigObject<ConfigValue> *_config);
    ~DlgPrefVinyl();

public slots:
    /** Update widget */
    void slotUpdate();
    void slotApply();
	void ChannelsSlotApply();
	void EnableRelativeModeSlotApply();
	void EnableScratchModeSlotApply();
	void EnableRIAASlotApply();
	void VinylTypeSlotApply();
	void AutoCalibrationSlotApply();
    void VinylGainSlotApply();

signals:
    void apply();
private:
    /** Pointer to player device */
    //PlayerProxy *player;
    SoundManager* m_pSoundManager;
    /** Pointer to config object */
    ConfigObject<ConfigValue> *config;
};

#endif
