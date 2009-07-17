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

#include "ui_dlgprefsounddlg.h"
#include "configobject.h"

class QWidget;
class SoundManager;
class ControlObject;

/**
  *@author Tue & Ken Haste Andersen
  */

class DlgPrefSound : public QWidget, public Ui::DlgPrefSoundDlg  {
    Q_OBJECT
public:
    DlgPrefSound(QWidget *parent, SoundManager* _soundman, ConfigObject<ConfigValue> *_config);
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
	void slotComboBoxSoundcardMasterChange();
	void slotComboBoxSoundcardHeadphonesChange();
    	void slotChannelChange();

signals:
    void apiUpdated();

private:
    /** Set QComboBox objects to be enabled or disabled based on different configuration states */
    void enableValidComboBoxes();
    /** Transform a slider value to latency value in msec */
    int getSliderLatencyMsec(int);
    /** Transform latency value in msec to slider value */
    int getSliderLatencyVal(int);
    /** Pointer to the sound manager */
    SoundManager* m_pSoundManager;
    /** Pointer to config object */
    ConfigObject<ConfigValue> *config;
    /** True if the mouse is currently dragging the latency slider */
    bool m_bLatencySliderDrag; 
	QWidget *m_parent;
};

#endif
