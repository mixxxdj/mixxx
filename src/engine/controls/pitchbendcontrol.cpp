#include "engine/controls/pitchbendcontrol.h"

#include <QtDebug>

#include "control/controlobject.h"
#include "control/controlpushbutton.h"
#include "engine/controls/enginecontrol.h"
#include "moc_pitchbendcontrol.cpp"

namespace {
constexpr int kRateSensitivityMin = 100;
constexpr int kRateSensitivityMax = 2500;
} // namespace

// Static default values for rate buttons (percents)
ControlValueAtomic<double> PitchBendControl::m_dTemporaryRateChangeCoarse;
ControlValueAtomic<double> PitchBendControl::m_dTemporaryRateChangeFine;
int PitchBendControl::m_iRateRampSensitivity;
PitchBendControl::RampMode PitchBendControl::m_eRateRampMode;

PitchBendControl::PitchBendControl(const QString& group,
        UserSettingsPointer pConfig)
        : EngineControl(group, pConfig),
          m_bTempStarted(false),
          m_tempRateRatio(0.0),
          m_dRateTempRampChange(0.0) {
    // We need the sample rate so we can guesstimate something close
    // what latency is.
    m_pSampleRate = ControlObject::getControl(
            ConfigKey(QStringLiteral("[App]"), QStringLiteral("samplerate")));

    // Temporary rate-change buttons
    m_pButtonRateTempDown =
            new ControlPushButton(ConfigKey(group, "rate_temp_down"));
    m_pButtonRateTempDownSmall =
            new ControlPushButton(ConfigKey(group, "rate_temp_down_small"));
    m_pButtonRateTempUp =
            new ControlPushButton(ConfigKey(group, "rate_temp_up"));
    m_pButtonRateTempUpSmall =
            new ControlPushButton(ConfigKey(group, "rate_temp_up_small"));

    //     // Update Internal Settings
    //     // Set Pitchbend Mode
    //     m_eRateRampMode = static_cast<RampMode>(
    //         getConfig()->getValue(ConfigKey("[Controls]","RateRamp"),
    //                               static_cast<int>(RampMode::Stepping)));

    //     // Set the Sensitivity
    //     m_iRateRampSensitivity =
    //             getConfig()->getValueString(ConfigKey("[Controls]","RateRampSensitivity")).toInt();
}

PitchBendControl::~PitchBendControl() {
    delete m_pButtonRateTempDown;
    delete m_pButtonRateTempDownSmall;
    delete m_pButtonRateTempUp;
    delete m_pButtonRateTempUpSmall;
}

// static
void PitchBendControl::setRateRampMode(RampMode mode) {
    m_eRateRampMode = mode;
}

// static
RateControl::RampMode PitchBendControl::getRateRampMode() {
    return m_eRateRampMode;
}

// static
void PitchBendControl::setRateRampSensitivity(int sense) {
    // Reverse the actual sensitivity value passed.
    // That way the gui works in an intuitive manner.
    sense = kRateSensitivityMax - sense + kRateSensitivityMin;
    if (sense < kRateSensitivityMin) {
        m_iRateRampSensitivity = kRateSensitivityMin;
    } else if (sense > kRateSensitivityMax) {
        m_iRateRampSensitivity = kRateSensitivityMax;
    } else {
        m_iRateRampSensitivity = sense;
    }
}

// static
void PitchBendControl::setTemporaryRateChangeCoarseAmount(double v) {
    m_dTemporaryRateChangeCoarse.setValue(v);
}

// static
void PitchBendControl::setTemporaryRateChangeFineAmount(double v) {
    m_dTemporaryRateChangeFine.setValue(v);
}

// static
double PitchBendControl::getTemporaryRateChangeCoarseAmount() {
    return m_dTemporaryRateChangeCoarse.getValue();
}

// static
double PitchBendControl::getTemporaryRateChangeFineAmount() {
    return m_dTemporaryRateChangeFine.getValue();
}

double PitchBendControl::getTempRate() {
    // qDebug() << m_tempRateRatio;
    return m_tempRateRatio;
}

void PitchBendControl::setRateTemp(double v) {
    m_tempRateRatio = v;
    if (m_tempRateRatio < -1.0) {
        m_tempRateRatio = -1.0;
    } else if (m_tempRateRatio > 1.0) {
        m_tempRateRatio = 1.0;
    } else if (util_isnan(m_tempRateRatio)) {
        m_tempRateRatio = 0;
    }
}

void PitchBendControl::addRateTemp(double v) {
    setRateTemp(m_tempRateRatio + v);
}

void PitchBendControl::subRateTemp(double v) {
    setRateTemp(m_tempRateRatio - v);
}

void PitchBendControl::resetRateTemp(void) {
    setRateTemp(0.0);
}

void PitchBendControl::processTempRate(const std::size_t bufferSamples, const double rateRange) {
    // Code to handle temporary rate change buttons.
    // We support two behaviors, the standard ramped pitch bending
    // and pitch shift stepping, which is the old behavior.

    RampDirection rampDirection = RampDirection::None;
    if (m_pButtonRateTempUp->toBool()) {
        rampDirection = RampDirection::Up;
    } else if (m_pButtonRateTempDown->toBool()) {
        rampDirection = RampDirection::Down;
    } else if (m_pButtonRateTempUpSmall->toBool()) {
        rampDirection = RampDirection::UpSmall;
    } else if (m_pButtonRateTempDownSmall->toBool()) {
        rampDirection = RampDirection::DownSmall;
    }

    if (rampDirection != RampDirection::None) {
        if (m_eRateRampMode == RampMode::Stepping) {
            if (!m_bTempStarted) {
                m_bTempStarted = true;
                // old temporary pitch shift behavior
                double change = m_dTemporaryRateChangeCoarse.getValue() / 100.0;
                double csmall = m_dTemporaryRateChangeFine.getValue() / 100.0;

                switch (rampDirection) {
                case RampDirection::Up:
                    setRateTemp(change);
                    break;
                case RampDirection::Down:
                    setRateTemp(-change);
                    break;
                case RampDirection::UpSmall:
                    setRateTemp(csmall);
                    break;
                case RampDirection::DownSmall:
                    setRateTemp(-csmall);
                    break;
                case RampDirection::None:
                default:
                    DEBUG_ASSERT(false);
                }
            }
        } else if (m_eRateRampMode == RampMode::Linear) {
            if (!m_bTempStarted) {
                m_bTempStarted = true;
                double latrate = bufferSamples / m_pSampleRate->get();
                m_dRateTempRampChange = latrate / (m_iRateRampSensitivity / 100.0);
            }

            switch (rampDirection) {
            case RampDirection::Up:
            case RampDirection::UpSmall:
                addRateTemp(m_dRateTempRampChange * rateRange);
                break;
            case RampDirection::Down:
            case RampDirection::DownSmall:
                subRateTemp(m_dRateTempRampChange * rateRange);
                break;
            case RampDirection::None:
            default:
                DEBUG_ASSERT(false);
            }
        }
    } else if (m_bTempStarted) {
        m_bTempStarted = false;
        resetRateTemp();
    }
}
