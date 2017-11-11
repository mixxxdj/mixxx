#ifndef DLGPREFDECK_H
#define DLGPREFDECK_H

#include <QWidget>

#include "preferences/constants.h"
#include "preferences/dialog/ui_dlgprefdeckdlg.h"
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

class DlgPrefDeck : public DlgPreferencePage, public Ui::DlgPrefDeckDlg  {
    Q_OBJECT
  public:
    DlgPrefDeck(QWidget *parent, MixxxMainWindow *mixxx,
                    PlayerManager* pPlayerManager,
                    UserSettingsPointer pConfig);
    virtual ~DlgPrefDeck();

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
    void slotSetTrackTimeDisplay(QAbstractButton*);
    void slotSetTrackTimeDisplay(double);
    void slotSetAllowTrackLoadToPlayingDeck(bool);
    void slotSetCueDefault(int);
    void slotSetCueRecall(bool);
    void slotSetRateRamp(bool);
    void slotSetRateRampSensitivity(int);

    void slotNumDecksChanged(double);
    void slotNumSamplersChanged(double);

    void slotUpdateSpeedAutoReset(bool);
    void slotUpdatePitchAutoReset(bool);

  private:
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
    PlayerManager* m_pPlayerManager;

    int m_iNumConfiguredDecks;
    int m_iNumConfiguredSamplers;

    bool m_speedAutoReset;
    bool m_pitchAutoReset;
    int m_keylockMode;
    int m_keyunlockMode;
};

#endif
