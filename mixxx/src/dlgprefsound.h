/***************************************************************************
                          dlgprefsound.h  -  description
                             -------------------
    begin                : Thu Apr 17 2003
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

#ifndef DLGPREFSOUND_H
#define DLGPREFSOUND_H

#include <qwidget.h>
#include "dlgprefsounddlg.h"
#include "configobject.h"

class Player;

/**
  *@author Tue & Ken Haste Andersen
  */

class DlgPrefSound : public DlgPrefSoundDlg  {
    Q_OBJECT
public: 
    DlgPrefSound(QWidget *parent, Player *_player,
                           ConfigObject<ConfigValue> *_config);
    ~DlgPrefSound();
public slots:
    /** Update widget */
    void slotUpdate();
    /** Update QComboBox values when devices are changed */
    void slotMasterDevice();
    void slotHeadDevice();
    void slotMasterDeviceOptions();
    void slotHeadDeviceOptions();
    void slotLatencyMaster();
    void slotLatencyHead();
    void slotApply();
signals:
    void apply();
private:
    /** Transform a slider value to latency value in msec */
    int getSliderLatencyMsec(int);
    /** Transform latency value in msec to slider value */
    int getSliderLatencyVal(int);
    /** Pointer to player device */
    Player *player;
    /** Pointer to config object */
    ConfigObject<ConfigValue> *config;
};

#endif
