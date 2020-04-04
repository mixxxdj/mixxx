#ifndef DLGPREFDECK_H
#define DLGPREFDECK_H

#include <QWidget>

#include "engine/controls/cuecontrol.h"
#include "engine/controls/ratecontrol.h"
#include "preferences/constants.h"
#include "preferences/dialog/ui_dlgprefdeckdlg.h"
#include "preferences/dlgpreferencepage.h"
#include "preferences/usersettings.h"

class ControlProxy;
class ControlPotmeter;
class SkinLoader;
class PlayerManager;
class MixxxMainWindow;
class ControlObject;

namespace {
constexpr bool kDefaultCloneDeckOnLoad = true;
}

namespace TrackTime {
    enum class DisplayMode {
        ELAPSED,
        REMAINING,
        ELAPSED_AND_REMAINING,
    };

    enum class DisplayFormat {
        TRADITIONAL,
        TRADITIONAL_COARSE,
        SECONDS,
        SECONDS_LONG,
        KILO_SECONDS,
        HECTO_SECONDS,
    };
}

enum class KeylockMode {
    LockOriginalKey,
    LockCurrentKey
};

enum class KeyunlockMode {
    ResetLockedKey,
    KeepLockedKey
};

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

    void slotMoveIntroStartCheckbox(bool checked);
    void slotRateRangeComboBox(int index);
    void slotRateInversionCheckbox(bool invert);
    void slotKeyLockModeSelected(QAbstractButton*);
    void slotKeyUnlockModeSelected(QAbstractButton*);
    void slotRateTempCoarseSpinbox(double);
    void slotRateTempFineSpinbox(double);
    void slotRatePermCoarseSpinbox(double);
    void slotRatePermFineSpinbox(double);
    void slotSetTrackTimeDisplay(QAbstractButton*);
    void slotSetTrackTimeDisplay(double);
    void slotDisallowTrackLoadToPlayingDeckCheckbox(bool);
    void slotCueModeCombobox(int);
    void slotSetTrackLoadMode(int comboboxIndex);
    void slotCloneDeckOnLoadDoubleTapCheckbox(bool);
    void slotRateRampingModeLinearButton(bool);
    void slotRateRampSensitivitySlider(int);

    void slotTimeFormatChanged(double);

    void slotNumDecksChanged(double, bool initializing=false);
    void slotNumSamplersChanged(double, bool initializing=false);

    void slotUpdateSpeedAutoReset(bool);
    void slotUpdatePitchAutoReset(bool);

  private:
    // Because the CueDefault list is out of order, we have to set the combo
    // box using the user data, not the index.  Returns the index of the item
    // that has the corresponding userData. If the userdata is not in the list,
    // returns zero.
    int cueDefaultIndexByData(int userData) const;

    void setRateRangeForAllDecks(int rangePercent);
    void setRateDirectionForAllDecks(bool inverted);

    UserSettingsPointer m_pConfig;
    ControlObject* m_pControlTrackTimeDisplay;
    ControlObject* m_pControlTrackTimeFormat;
    ControlProxy* m_pNumDecks;
    ControlProxy* m_pNumSamplers;
    QList<ControlProxy*> m_cueControls;
    QList<ControlProxy*> m_rateControls;
    QList<ControlProxy*> m_rateDirectionControls;
    QList<ControlProxy*> m_rateRangeControls;
    QList<ControlProxy*> m_keylockModeControls;
    QList<ControlProxy*> m_keyunlockModeControls;
    MixxxMainWindow *m_mixxx;
    PlayerManager* m_pPlayerManager;

    int m_iNumConfiguredDecks;
    int m_iNumConfiguredSamplers;

    TrackTime::DisplayMode m_timeDisplayMode;

    CueMode m_cueMode;

    bool m_bSetIntroStartAtMainCue;
    bool m_bDisallowTrackLoadToPlayingDeck;
    bool m_bCloneDeckOnLoadDoubleTap;
    bool m_bAssignHotcueColors;

    int m_iRateRangePercent;
    bool m_bRateInverted;

    bool m_speedAutoReset;
    bool m_pitchAutoReset;
    KeylockMode m_keylockMode;
    KeyunlockMode m_keyunlockMode;
    SeekOnLoadMode m_seekOnLoadMode;

    RateControl::RampMode m_bRateRamping;
    int m_iRateRampSensitivity;
    double m_dRateTempCoarse;
    double m_dRateTempFine;
    double m_dRatePermCoarse;
    double m_dRatePermFine;
};

#endif
