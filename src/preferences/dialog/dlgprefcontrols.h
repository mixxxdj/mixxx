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

#include <QWidget>

#include "preferences/dialog/ui_dlgprefcontrolsdlg.h"
#include "preferences/usersettings.h"
#include "preferences/dlgpreferencepage.h"

class ControlProxy;
class ControlPotmeter;
class SkinLoader;
class PlayerManager;
class MixxxMainWindow;
class ControlObject;

namespace TrackTime {
    enum class DisplayMode {
        Elapsed,
        Remaining,
        ElapsedAndRemaining,
    };
}

/**
  *@author Tue & Ken Haste Andersen
  */

class DlgPrefControls : public DlgPreferencePage, public Ui::DlgPrefControlsDlg  {
    Q_OBJECT
  public:
    DlgPrefControls(QWidget *parent, MixxxMainWindow *mixxx,
                    SkinLoader* pSkinLoader, PlayerManager* pPlayerManager,
                    UserSettingsPointer pConfig);
    virtual ~DlgPrefControls();

  public slots:
    void slotUpdate();
    void slotApply();
    void slotResetToDefaults();

    void slotSetRateRange(int pos);
    void slotSetRateRangePercent(int rateRangePercent);
    void slotSetRateDir(bool invert);
    void slotSetRateDir(int pos);
    void slotKeyLockMode(QAbstractButton*);
    void slotKeyUnlockMode(QAbstractButton*);
    void slotSetRateTempLeft(double);
    void slotSetRateTempRight(double);
    void slotSetRatePermLeft(double);
    void slotSetRatePermRight(double);
    void slotSetTooltips();
    void slotSetSkin(int);
    void slotSetScheme(int);
    void slotUpdateSchemes();
    void slotSetTrackTimeDisplay(QAbstractButton*);
    void slotSetTrackTimeDisplay(double);
    void slotSetAllowTrackLoadToPlayingDeck(bool);
    void slotSetCueDefault(int);
    void slotSetCueRecall(bool);
    void slotSetRateRamp(bool);
    void slotSetRateRampSensitivity(int);
    void slotSetLocale(int);
    void slotSetScaleFactor(int index);
    void slotSetScaleFactorAuto(bool checked);
    void slotSetStartInFullScreen(bool b);

    void slotNumDecksChanged(double);
    void slotNumSamplersChanged(double);

    void slotUpdateSpeedAutoReset(bool);
    void slotUpdatePitchAutoReset(bool);

  private:
    void notifyRebootNecessary();
    bool checkSkinResolution(QString skin);

    // Because the CueDefault list is out of order, we have to set the combo
    // box using the user data, not the index.  Returns the index of the item
    // that has the corresponding userData. If the userdata is not in the list,
    // returns zero.
    int cueDefaultIndexByData(int userData) const;

    UserSettingsPointer m_pConfig;
    ControlObject* m_pControlTrackTimeDisplay;
    ControlProxy* m_pNumDecks;
    ControlProxy* m_pNumSamplers;
    QList<ControlProxy*> m_cueControls;
    QList<ControlProxy*> m_rateControls;
    QList<ControlProxy*> m_rateDirControls;
    QList<ControlProxy*> m_rateRangeControls;
    QList<ControlProxy*> m_keylockModeControls;
    QList<ControlProxy*> m_keyunlockModeControls;
    MixxxMainWindow *m_mixxx;
    SkinLoader* m_pSkinLoader;
    PlayerManager* m_pPlayerManager;

    int m_iNumConfiguredDecks;
    int m_iNumConfiguredSamplers;

    bool m_speedAutoReset;
    bool m_pitchAutoReset;
    int m_keylockMode;
    int m_keyunlockMode;
    double m_autoScaleFactor;
};

#endif
