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

class QWidget;
class ControlObjectThreadMain;
class ControlPotmeter;
class SkinLoader;
class PlayerManager;
class MixxxApp;
class ControlObject;

/**
  *@author Tue & Ken Haste Andersen
  */

class DlgPrefControls : public QWidget, public Ui::DlgPrefControlsDlg  {
    Q_OBJECT
public:
    DlgPrefControls(QWidget *parent, MixxxApp *mixxx,
                    SkinLoader* pSkinLoader, PlayerManager* pPlayerManager,
                    ConfigObject<ConfigValue> *pConfig);
    ~DlgPrefControls();

public slots:
    void slotUpdate();
    void slotSetRateRange(int pos);
    void slotSetRateDir(int pos);
    void slotSetRateTempLeft(double);
    void slotSetRateTempRight(double);
    void slotSetRatePermLeft(double);
    void slotSetRatePermRight(double);
    void slotSetTooltips(int pos);
    void slotSetSkin(int);
    void slotSetScheme(int);
    void slotUpdateSchemes();
    void slotSetPositionDisplay(int);
    void slotSetPositionDisplay(double);
    void slotSetAllowTrackLoadToPlayingDeck(int);
    void slotSetCueDefault(int);
    void slotSetCueRecall(int);
    void slotSetRateRamp(bool);
    void slotSetRateRampSensitivity(int);
    void slotSetLocale(int);
    void slotApply();

    void slotSetFrameRate(int frameRate);
    void slotSetWaveformType(int index);
    void slotSetDefaultZoom(int index);
    void slotSetZoomSynchronization(bool checked);
    void slotSetVisualGainAll(double gain);
    void slotSetVisualGainLow(double gain);
    void slotSetVisualGainMid(double gain);
    void slotSetVisualGainHigh(double gain);
    void slotSetNormalizeOverview( bool normalize);

    virtual void onShow();
    virtual void onHide();

protected:
    void timerEvent(QTimerEvent *);

private:
    void initWaveformControl();
    void notifyRebootNecessary();
    bool checkSkinResolution(QString skin);

private:
    /** Pointer to ConfigObject */
    ConfigObject<ConfigValue> *m_pConfig;
    int m_timer;
    ControlObject* m_pControlPositionDisplay;
    QList<ControlObjectThreadMain*> m_cueControls;
    QList<ControlObjectThreadMain*> m_rateControls;
    QList<ControlObjectThreadMain*> m_rateDirControls;
    QList<ControlObjectThreadMain*> m_rateRangeControls;
    PlayerManager* m_pPlayerManager;
    MixxxApp *m_mixxx;
    SkinLoader* m_pSkinLoader;
};

#endif
