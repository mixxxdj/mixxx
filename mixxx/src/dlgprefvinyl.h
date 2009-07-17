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
#include "vinylcontrolsignalwidget.h"

class QWidget;
class PlayerProxy;
class ControlObject;
class ControlObjectThreadMain;


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
    void slotComboBoxDeviceDeck1Change();
    void slotComboBoxDeviceDeck2Change();
	void EnableRelativeModeSlotApply();
	void EnableScratchModeSlotApply();
	void VinylTypeSlotApply();
	void AutoCalibrationSlotApply();
    void VinylGainSlotApply();
    void enableValidComboBoxes();
    void updateSignalQuality1(double value);
    void updateSignalQuality2(double value);
    void updateInputLevelLeft1(double value);
    void updateInputLevelRight1(double value);
    void updateInputLevelLeft2(double value);
    void updateInputLevelRight2(double value);
    void slotClose();
    void slotShow();

signals:
    void apply();
private:
	void applySoundDeviceChanges();

    VinylControlSignalWidget m_signalWidget1;
    VinylControlSignalWidget m_signalWidget2;


    /** Pointer to player device */
    //PlayerProxy *player;
    SoundManager* m_pSoundManager;
    /** Pointer to config object */
    ConfigObject<ConfigValue> *config;
    /** Indicates the strength of the timecode signal on each input */
    ControlObjectThreadMain* m_timecodeQuality1;
    ControlObjectThreadMain* m_timecodeQuality2;
    ControlObjectThreadMain* m_vinylControlInput1L;
    ControlObjectThreadMain* m_vinylControlInput1R;
    ControlObjectThreadMain* m_vinylControlInput2L;
    ControlObjectThreadMain* m_vinylControlInput2R;
};

#endif
