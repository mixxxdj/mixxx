#pragma once

#include <QObject>

#include "control/controlobject.h"
#include "engine/controls/enginecontrol.h"
#include "engine/sync/syncable.h"
#include "preferences/usersettings.h"

class ControlObject;
class ControlPushButton;

// PitchBendControl is an EngineControl that is in charge of managing the rate of
// playback of a given channel of audio in the Mixxx engine. Using input from
// various controls, RateControl will calculate the current rate.
class PitchBendControl : public EngineControl {
    Q_OBJECT
  public:
    PitchBendControl(const QString& group, UserSettingsPointer pConfig);
    PitchBendControl();

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

    // Set rate change when temp rate button is pressed
    static void setTemporaryRateChangeCoarseAmount(double v);
    static double getTemporaryRateChangeCoarseAmount();

    // Set rate change when temp rate small button is pressed
    static void setTemporaryRateChangeFineAmount(double v);
    static double getTemporaryRateChangeFineAmount();

    // Set Rate Ramp Mode
    static void setRateRampMode(RampMode mode);
    static RampMode getRateRampMode();

    // Set Rate Ramp Sensitivity
    static void setRateRampSensitivity(int);
    static int getRateRampSensitivity();

    // Invoked by RateControl::calculateSpeed
    void processTempRate(const std::size_t bufferSamples, const double rateRange);

    // Get the 'Raw' Temp Rate
    double getTempRate(void);

  private:
    // Set rate change of the temporary pitch rate
    void setRateTemp(double v);
    // Add a value to the temporary pitch rate
    void addRateTemp(double v);
    // Subtract a value from the temporary pitch rate
    void subRateTemp(double v);
    // Reset the temporary pitch rate
    void resetRateTemp(void);

    // Values used when temp rate buttons are pressed
    static ControlValueAtomic<double> m_dTemporaryRateChangeCoarse;
    static ControlValueAtomic<double> m_dTemporaryRateChangeFine;

    ControlPushButton* m_pButtonRateTempDown;
    ControlPushButton* m_pButtonRateTempDownSmall;
    ControlPushButton* m_pButtonRateTempUp;
    ControlPushButton* m_pButtonRateTempUpSmall;

    ControlObject* m_pSampleRate;

    // This is true if we've already started to ramp the rate
    bool m_bTempStarted;
    // Set the Temporary Rate Change Mode
    static RampMode m_eRateRampMode;
    // The Rate Temp Sensitivity, the higher it is the slower it gets
    static int m_iRateRampSensitivity;
    // Temporary pitchrate, added to the permanent rate for calculateRate
    double m_tempRateRatio;
    // Speed for temporary rate change
    double m_dRateTempRampChange;
};
