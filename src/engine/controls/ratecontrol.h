#pragma once

#include <QObject>

#include "preferences/usersettings.h"
#include "engine/controls/enginecontrol.h"
#include "engine/sync/syncable.h"

class PitchBendControl;
class BpmControl;
class Rotary;
class ControlTTRotary;
class ControlObject;
class ControlPotmeter;
class ControlPushButton;
class ControlProxy;
class PositionScratchController;

// RateControl is an EngineControl that is in charge of managing the rate of
// playback of a given channel of audio in the Mixxx engine. Using input from
// various controls, RateControl will calculate the current rate.
class RateControl : public EngineControl {
    Q_OBJECT
public:
  RateControl(const QString& group, UserSettingsPointer pConfig);
  ~RateControl() override;

  void setPitchBendControl(PitchBendControl* pitchBendControl);
  void setBpmControl(BpmControl* bpmControl);

  // Returns the current engine rate.  "reportScratching" is used to tell
  // the caller that the user is currently scratching, and this is used to
  // disable keylock.
  double calculateSpeed(
          double baserate,
          double speed,
          bool paused,
          std::size_t samplesPerBuffer,
          bool* pReportScratching,
          bool* pReportReverse);

  // Set rate change when perm rate button is pressed
  static void setPermanentRateChangeCoarseAmount(double v);
  static double getPermanentRateChangeCoarseAmount();
  // Set rate change when perm rate small button is pressed
  static void setPermanentRateChangeFineAmount(double v);
  static double getPermanentRateChangeFineAmount();
  bool isReverseButtonPressed();
  // ReadAheadManager::getNextSamples() notifies us each time the play position
  // wrapped around during one buffer process (beatloop or track repeat) so
  // PositionScratchController can correctly interpret the sample position delta.
  void notifyWrapAround(mixxx::audio::FramePos triggerPos,
          mixxx::audio::FramePos targetPos);

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
  double getJogFactor() const;
  double getWheelFactor() const;
  SyncMode getSyncMode() const;

  // Values used when temp and perm rate buttons are pressed
  static ControlValueAtomic<double> m_dPermanentRateChangeCoarse;
  static ControlValueAtomic<double> m_dPermanentRateChangeFine;

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

  // For pitch bending
  PitchBendControl* m_pPitchBendControl;

  // For Sync Lock
  BpmControl* m_pBpmControl;

  ControlProxy* m_pSyncMode;
  ControlProxy* m_pSlipEnabled;

  int m_wrapAroundCount;
  mixxx::audio::FramePos m_jumpPos;
  mixxx::audio::FramePos m_targetPos;

  // Factor applied to the deprecated "wheel" rate value.
  static const double kWheelMultiplier;
  // Factor applied to jogwheels when the track is paused to speed up seeking.
  static const double kPausedJogMultiplier;
};
