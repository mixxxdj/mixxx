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

class PlayerProxy;
class ControlObject;

/**
  *@author Tue & Ken Haste Andersen
  */

class DlgPrefSound : public DlgPrefSoundDlg  {
    Q_OBJECT
public:
    DlgPrefSound(QWidget *parent, PlayerProxy *_player, ConfigObject<ConfigValue> *_config);
    ~DlgPrefSound();
public slots:
    /** Update widget */
    void slotUpdate();
    void slotLatency();
    void slotApply();
    void slotApplyApi();

private slots:
    void slotLatencySliderClick();
    void slotLatencySliderRelease();
    void slotLatencySliderChange(int);
signals:
    void apply();
private:
    /** Transform a slider value to latency value in msec */
    int getSliderLatencyMsec(int);
    /** Transform latency value in msec to slider value */
    int getSliderLatencyVal(int);
    /** Pointer to player device */
    PlayerProxy *player;
    /** Pointer to config object */
    ConfigObject<ConfigValue> *config;
    /** True if the mouse is currently dragging the latency slider */
    bool m_bLatencySliderDrag; 
};

#endif
