#pragma once

#include <QObject>

#include "preferences/usersettings.h"
#include "engine/controls/enginecontrol.h"
#include "engine/sync/syncable.h"

class BpmControl;
class Rotary;
class ControlTTRotary;
class ControlObject;
class ControlPotmeter;
class ControlPushButton;
class ControlProxy;
class EngineChannel;
class PositionScratchController;

// RateControl is an EngineControl that is in charge of managing the rate of
// playback of a given channel of audio in the Mixxx engine. Using input from
// various controls, RateControl will calculate the current rate.
class RateControl : public EngineControl {
    Q_OBJECT
public:
  RateControl(const QString& group, UserSettingsPointer pConfig);
  ~RateControl() override;

  // Enumerations which hold the state of the pitchbend buttons.
  // These enumerations can be used like a bitmask.
  enum class RampDirection {
      None,      // No buttons are held down
      Down,      // Down button is being held
      Up,        // Up button is being held
      DownSmall, // DownSmall button is being held
      UpSmall,   // UpSmall button is being held
  };

  enum class RampMode {
      Stepping = 0, // pitch takes a temporary step up/down a certain amount
      Linear = 1    // pitch moves up/down in a progressively linear fashion
  };

  void setBpmControl(BpmControl* bpmcontrol);

  // Returns the current engine rate.  "reportScratching" is used to tell
  // the caller that the user is currently scratching, and this is used to
  // disable keylock.
  double calculateSpeed(
          double baserate,
          double speed,
          bool paused,
          int iSamplesPerBuffer,
          bool* pReportScratching,
          bool* pReportReverse);

  // Set rate change when temp rate button is pressed
  static void setTemporaryRateChangeCoarseAmount(double v);
  static double getTemporaryRateChangeCoarseAmount();
  // Set rate change when temp rate small button is pressed
  static void setTemporaryRateChangeFineAmount(double v);
  static double getTemporaryRateChangeFineAmount();
  // Set rate change when perm rate button is pressed
  static void setPermanentRateChangeCoarseAmount(double v);
  static double getPermanentRateChangeCoarseAmount();
  // Set rate change when perm rate small button is pressed
  static void setPermanentRateChangeFineAmount(double v);
  static double getPermanentRateChangeFineAmount();
  // Set Rate Ramp Mode
  static void setRateRampMode(RampMode mode);
  static RampMode getRateRampMode();
  // Set Rate Ramp Sensitivity
  static void setRateRampSensitivity(int);
  static int getRateRampSensitivity();
  void notifySeek(double dNewPlaypos) override;
  bool isReverseButtonPressed();

public slots:
  void slotRateRangeChanged(double);
  void slotRateSliderChanged(double);
  void slotRateRatioChanged(double);
  void slotReverseRollActivate(double);
  void slotControlRatePermDown(double);
  void slotControlRatePermDownSmall(double);
  void slotControlRatePermUp(double);
  void slotControlRatePermUpSmall(double);
  void slotControlFastForward(double);
  void slotControlFastBack(double);

private:
  void processTempRate(const int bufferSamples);
  double getJogFactor() const;
  double getWheelFactor() const;
  SyncMode getSyncMode() const;

  // Set rate change of the temporary pitch rate
  void setRateTemp(double v);
  // Add a value to the temporary pitch rate
  void addRateTemp(double v);
  // Subtract a value from the temporary pitch rate
  void subRateTemp(double v);
  // Reset the temporary pitch rate
  void resetRateTemp(void);
  // Get the 'Raw' Temp Rate
  double getTempRate(void);

  // Values used when temp and perm rate buttons are pressed
  static ControlValueAtomic<double> m_dTemporaryRateChangeCoarse;
  static ControlValueAtomic<double> m_dTemporaryRateChangeFine;
  static ControlValueAtomic<double> m_dPermanentRateChangeCoarse;
  static ControlValueAtomic<double> m_dPermanentRateChangeFine;

  ControlPushButton* m_pButtonRateTempDown;
  ControlPushButton* m_pButtonRateTempDownSmall;
  ControlPushButton* m_pButtonRateTempUp;
  ControlPushButton* m_pButtonRateTempUpSmall;

  ControlPushButton* m_pButtonRatePermDown;
  ControlPushButton* m_pButtonRatePermDownSmall;
  ControlPushButton* m_pButtonRatePermUp;
  ControlPushButton* m_pButtonRatePermUpSmall;

  ControlObject* m_pRateRatio;
  ControlObject* m_pRateDir;
  ControlObject* m_pRateRange;
  ControlPotmeter* m_pRateSlider;
  ControlPotmeter* m_pRateSearch;
  ControlPushButton* m_pReverseButton;
  ControlPushButton* m_pReverseRollButton;
  ControlObject* m_pBackButton;
  ControlObject* m_pForwardButton;

  ControlTTRotary* m_pWheel;
  ControlObject* m_pScratch2;
  PositionScratchController* m_pScratchController;

  ControlPushButton* m_pScratch2Enable;
  ControlObject* m_pJog;
  ControlObject* m_pVCEnabled;
  ControlObject* m_pVCScratching;
  ControlObject* m_pVCMode;
  ControlObject* m_pScratch2Scratching;
  Rotary* m_pJogFilter;

  ControlObject* m_pSampleRate;

  // For Master Sync
  BpmControl* m_pBpmControl;

  ControlProxy* m_pSyncMode;
  ControlProxy* m_pSlipEnabled;

  // This is true if we've already started to ramp the rate
  bool m_bTempStarted;
  // Set the Temporary Rate Change Mode
  static RampMode m_eRateRampMode;
  // The Rate Temp Sensitivity, the higher it is the slower it gets
  static int m_iRateRampSensitivity;
  // Factor applied to the deprecated "wheel" rate value.
  static const double kWheelMultiplier;
  // Factor applied to jogwheels when the track is paused to speed up seeking.
  static const double kPausedJogMultiplier;
  // Temporary pitchrate, added to the permanent rate for calculateRate
  double m_tempRateRatio;
  // Speed for temporary rate change
  double m_dRateTempRampChange;
};
