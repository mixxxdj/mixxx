#pragma once

#include <QWidget>
#include <memory>

#include "engine/controls/cuecontrol.h"
#include "engine/controls/ratecontrol.h"
#include "preferences/constants.h"
#include "preferences/dialog/ui_dlgprefdeckdlg.h"
#include "preferences/dlgpreferencepage.h"
#include "preferences/usersettings.h"
#include "util/parented_ptr.h"

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

class DlgPrefDeck : public DlgPreferencePage, public Ui::DlgPrefDeckDlg  {
    Q_OBJECT
  public:
    DlgPrefDeck(QWidget* parent,
            UserSettingsPointer pConfig);
    ~DlgPrefDeck() override;

  public slots:
    void slotUpdate() override;
    void slotApply() override;
    void slotResetToDefaults() override;

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

    const UserSettingsPointer m_pConfig;

    const std::unique_ptr<ControlObject> m_pControlTrackTimeDisplay;
    const std::unique_ptr<ControlObject> m_pControlTrackTimeFormat;

    const parented_ptr<ControlProxy> m_pNumDecks;
    const parented_ptr<ControlProxy> m_pNumSamplers;

    QList<ControlProxy*> m_cueControls;
    QList<ControlProxy*> m_rateControls;
    QList<ControlProxy*> m_rateDirectionControls;
    QList<ControlProxy*> m_rateRangeControls;
    QList<ControlProxy*> m_keylockModeControls;
    QList<ControlProxy*> m_keyunlockModeControls;

    int m_iNumConfiguredDecks;
    int m_iNumConfiguredSamplers;

    TrackTime::DisplayMode m_timeDisplayMode;

    CueMode m_cueMode;

    bool m_bSetIntroStartAtMainCue;
    bool m_bDisallowTrackLoadToPlayingDeck;
    bool m_bCloneDeckOnLoadDoubleTap;

    int m_iRateRangePercent;
    bool m_bRateDownIncreasesSpeed;

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
