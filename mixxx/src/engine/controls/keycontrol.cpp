#include "engine/controls/keycontrol.h"

#include <QPair>
#include <QtDebug>

#include "control/controlobject.h"
#include "control/controlpotmeter.h"
#include "control/controlproxy.h"
#include "control/controlpushbutton.h"
#include "engine/enginebuffer.h"
#include "mixer/playermanager.h"
#include "moc_keycontrol.cpp"
#include "track/keyutils.h"

constexpr bool kEnableDebugOutput = false;

static const double kLockCurrentKey = 1;
static const double kKeepUnlockedKey = 1;

KeyControl::KeyControl(const QString& group,
        UserSettingsPointer pConfig)
        : EngineControl(group, pConfig),
          // pitch is the distance to the original pitch in semitones
          // knob in semitones; 9.4 ct per midi step allowOutOfBounds = true;
          m_pPitch(std::make_unique<ControlPotmeter>(
                  ConfigKey(group, "pitch"), -6.0, 6.0, true)),
          // pitch_adjust is the distance to the linear pitch in semitones
          // set by the speed slider or to the locked key.
          // pitch_adjust knob in semitones; 4.7 ct per midi step; allowOutOfBounds = true;
          m_pPitchAdjust(std::make_unique<ControlPotmeter>(
                  ConfigKey(group, "pitch_adjust"), -3.0, 3.0, true)),
          m_pButtonSyncKey(std::make_unique<ControlPushButton>(
                  ConfigKey(group, "sync_key"))),
          m_pButtonResetKey(std::make_unique<ControlPushButton>(
                  ConfigKey(group, "reset_key"))),
          m_keylockMode(std::make_unique<ControlPushButton>(ConfigKey(group, "keylockMode"))),
          m_keyunlockMode(std::make_unique<ControlPushButton>(ConfigKey(group, "keyunlockMode"))),
          m_pFileKey(std::make_unique<ControlObject>(ConfigKey(group, "file_key"))),
          m_pEngineKey(std::make_unique<ControlObject>(ConfigKey(group, "key"))),
          m_pEngineKeyDistance(std::make_unique<ControlPotmeter>(
                  ConfigKey(group, "visual_key_distance"), -0.5, 0.5)) {
    m_pitchRateInfo.pitchRatio = 1.0;
    m_pitchRateInfo.tempoRatio = 1.0;
    m_pitchRateInfo.pitchTweakRatio = 1.0;
    m_pitchRateInfo.keylock = false;

    // Coarse adjust by full semitone steps.
    m_pPitch->setStepCount(12);
    // Fine adjust with semitone / 10 = 10 ct;.
    m_pPitch->setSmallStepCount(120);

    // Coarse adjust by full semitone steps.
    m_pPitchAdjust->setStepCount(6);
    // Fine adjust with semitone / 10 = 10 ct;.
    m_pPitchAdjust->setSmallStepCount(60);

    m_keylockMode->setButtonMode(mixxx::control::ButtonMode::Toggle);

    m_keyunlockMode->setButtonMode(mixxx::control::ButtonMode::Toggle);

    connect(m_pPitch.get(),
            &ControlObject::valueChanged,
            this,
            &KeyControl::slotPitchChanged,
            Qt::DirectConnection);

    connect(m_pPitchAdjust.get(),
            &ControlObject::valueChanged,
            this,
            &KeyControl::slotPitchAdjustChanged,
            Qt::DirectConnection);

    connect(m_pButtonSyncKey.get(),
            &ControlObject::valueChanged,
            this,
            &KeyControl::slotSyncKey,
            Qt::DirectConnection);

    connect(m_pButtonResetKey.get(),
            &ControlObject::valueChanged,
            this,
            &KeyControl::slotResetKey,
            Qt::DirectConnection);

    connect(m_pFileKey.get(),
            &ControlObject::valueChanged,
            this,
            &KeyControl::slotFileKeyChanged,
            Qt::DirectConnection);

    connect(m_pEngineKey.get(),
            &ControlObject::valueChanged,
            this,
            &KeyControl::slotSetEngineKey,
            Qt::DirectConnection);

    connect(m_pEngineKeyDistance.get(),
            &ControlObject::valueChanged,
            this,
            &KeyControl::slotSetEngineKeyDistance,
            Qt::DirectConnection);

    // In case of vinyl control "rate_ratio" is a filtered mean value for display
    m_pRateRatio = make_parented<ControlProxy>(ConfigKey(group, "rate_ratio"), this);
    m_pRateRatio->connectValueChanged(this, &KeyControl::slotRateChanged,
            Qt::DirectConnection);

    // VinylControl is only available on main decks
    if (PlayerManager::isDeckGroup(group)) {
        m_pVCEnabled = make_parented<ControlProxy>(group, "vinylcontrol_enabled", this);
        m_pVCEnabled->connectValueChanged(this, &KeyControl::slotRateChanged, Qt::DirectConnection);

        m_pVCRate = make_parented<ControlProxy>(group, "vinylcontrol_rate", this);
        m_pVCRate->connectValueChanged(this, &KeyControl::slotRateChanged, Qt::DirectConnection);
    }

    m_pKeylock = make_parented<ControlProxy>(group, "keylock", this);
    m_pKeylock->connectValueChanged(this, &KeyControl::slotRateChanged,
            Qt::DirectConnection);

    // m_pitchRateInfo members are initialized with default values, only keylock
    // is persistent and needs to be updated from config
    m_pitchRateInfo.keylock = m_pKeylock->toBool();
}

KeyControl::PitchTempoRatio KeyControl::getPitchTempoRatio() {
    // TODO(XXX) remove code duplication by adding this
    // "Update pending" atomic flag to the ControlObject API
    if (m_updatePitchRequest.fetchAndStoreAcquire(0)) {
        updatePitch();
    }
    if (m_updatePitchAdjustRequest.fetchAndStoreAcquire(0)) {
        updatePitchAdjust();
    }
    if (m_updateRateRequest.fetchAndStoreAcquire(0)) {
        updateRate();
    }
    return m_pitchRateInfo;
}

double KeyControl::getKey() {
    return m_pEngineKey->get();
}

void KeyControl::slotRateChanged() {
    m_updateRateRequest = 1;
    updateRate();
}

// This is called when rate_ratio, vinylcontrol_rate, vinylcontrol_enabled or
// keylock are changed, but also when EngineBuffer::processTrackLocked requests
// m_pitchRateInfo struct while rate, pitch or pitch_adjust were just updated.
void KeyControl::updateRate() {
    if (m_pVCEnabled && m_pVCRate && m_pVCEnabled->toBool()) {
        m_pitchRateInfo.tempoRatio = m_pVCRate->get();
    } else {
        m_pitchRateInfo.tempoRatio = m_pRateRatio->get();
    }

    if (m_pitchRateInfo.tempoRatio == 0) {
        // no transport, no pitch
        // so we can skip pitch calculation
        return;
    }

    if constexpr (kEnableDebugOutput) {
        qDebug() << "   .";
        qDebug() << "   KeyControl::updateRate";
        qDebug() << "   |  tempoRatio     " << m_pitchRateInfo.tempoRatio;
        qDebug() << "   |  pitchRatio     " << m_pitchRateInfo.pitchRatio;
        qDebug() << "   |  pitchTweakRatio" << m_pitchRateInfo.pitchTweakRatio;
        qDebug() << "   |  keyLock " << m_pitchRateInfo.keylock << ">>"
                 << m_pKeylock->toBool();
        qDebug() << "   |";
    }

    // |-----------------------|-----------------|
    //   SpeedSliderPitchRatio   pitchTweakRatio
    //
    // |-----------------------------------------|
    //   m_pitchRatio
    //
    //                         |-----------------|
    //                           m_pPitchAdjust
    //
    // |-----------------------------------------|
    //   m_pPitch

    double speedSliderPitchRatio =
            m_pitchRateInfo.pitchRatio / m_pitchRateInfo.pitchTweakRatio;

    if (m_pKeylock->toBool()) {
        if (!m_pitchRateInfo.keylock) {                    // Enabling keylock
            if (m_keylockMode->get() == kLockCurrentKey) { // Lock at current pitch
                speedSliderPitchRatio = m_pitchRateInfo.tempoRatio;
                if constexpr (kEnableDebugOutput) {
                    qDebug() << "   LOCKING current key";
                    qDebug() << "   | speedSliderPitchRatio = tempoRatio ="
                             << speedSliderPitchRatio;
                    qDebug() << "   |";
                }
            } else { // Lock at original track pitch, reset pitch_adjust
                speedSliderPitchRatio = 1.0;
                m_pitchRateInfo.pitchTweakRatio = 1.0;
                m_pPitchAdjust->set(0);
                if constexpr (kEnableDebugOutput) {
                    qDebug() << "   LOCK original key, reset pitch_adjust";
                    qDebug() << "   | speedSliderPitchRatio =" << speedSliderPitchRatio;
                    qDebug() << "   | pitchTweakRatio       =" << m_pitchRateInfo.pitchTweakRatio;
                    qDebug() << "   |";
                }
            }
            m_pitchRateInfo.keylock = true;
        } else { // Key already locked, nothing to do
            if constexpr (kEnableDebugOutput) {
                qDebug() << "   LOCKED";
                qDebug() << "   | speedSliderPitchRatio =";
                qDebug() << "   |   pitchRatio       " << m_pitchRateInfo.pitchRatio;
                qDebug() << "   |   / pitchTweakRatio" << m_pitchRateInfo.pitchTweakRatio;
                qDebug() << "   | =" << qSetRealNumberPrecision(18) << speedSliderPitchRatio;
                qDebug() << "   |";
            }
        }
    } else {                           // !m_pKeylock
        if (m_pitchRateInfo.keylock) { // Disabling Keylock
            if (m_keyunlockMode->get() == kKeepUnlockedKey) {
                // adopt speedSliderPitchRatio change as pitchTweakRatio
                m_pitchRateInfo.pitchTweakRatio *=
                        (speedSliderPitchRatio / m_pitchRateInfo.tempoRatio);
                if constexpr (kEnableDebugOutput) {
                    qDebug() << "   UNLOCKING keep current key";
                    qDebug() << "   | speedSliderPitchRatio =";
                    qDebug() << "   |   pitchRatio       " << m_pitchRateInfo.pitchRatio;
                    qDebug() << "   |   / pitchTweakRatio" << m_pitchRateInfo.pitchTweakRatio;
                    qDebug() << "   | =" << qSetRealNumberPrecision(18) << speedSliderPitchRatio;
                    qDebug() << "   |";
                    qDebug() << "   | pitchTweakRatio *=";
                    qDebug() << "   |   speedSliderPitchRatio" << speedSliderPitchRatio;
                    qDebug() << "   |   / tempoRatio        " << m_pitchRateInfo.tempoRatio;
                    qDebug() << "   | =" << qSetRealNumberPrecision(18)
                             << m_pitchRateInfo.pitchTweakRatio;
                    qDebug() << "   |";
                }

                // adopt pitch_adjust now so that it doesn't jump and resets key
                // when touching pitch_adjust knob after unlock with offset key
                m_pPitchAdjust->set(
                        KeyUtils::powerOf2ToSemitoneChange(m_pitchRateInfo.pitchTweakRatio));
            } else { // Unlock and reset to linear pitch (orig. key + pitch fader offset)
                m_pitchRateInfo.pitchTweakRatio = 1.0;
                m_pPitchAdjust->set(0);
                if constexpr (kEnableDebugOutput) {
                    qDebug() << "   UNLOCKING reset to linear pitch";
                    qDebug() << "   : pitchTweakRatio = 1.0";
                    qDebug() << "   |";
                }
            }
            m_pitchRateInfo.keylock = false;
        } else { // already unlocked
            if constexpr (kEnableDebugOutput) {
                qDebug() << "   UNLOCKED";
            }
        }
        speedSliderPitchRatio = m_pitchRateInfo.tempoRatio;
        if constexpr (kEnableDebugOutput) {
            qDebug() << "   | speedSliderPitchRatio = tempoRatio";
            qDebug() << "   | =" << speedSliderPitchRatio;
            qDebug() << "   |";
        }
    }

    m_pitchRateInfo.pitchRatio = m_pitchRateInfo.pitchTweakRatio * speedSliderPitchRatio;
    if constexpr (kEnableDebugOutput) {
        qDebug() << "   | pitchRatio =";
        qDebug() << "   | pitchTweakRatio        " << m_pitchRateInfo.pitchTweakRatio;
        qDebug() << "   | * speedSliderPitchRatio" << speedSliderPitchRatio;
        qDebug() << "   | =" << qSetRealNumberPrecision(18) << m_pitchRateInfo.pitchRatio;
        qDebug() << "   | =" << m_pitchRateInfo.pitchRatio;
        qDebug() << "   .";
    }

    // If we just unlocked the original key with speed != +-1.0 we may encounter wrong
    // decimals (e.g. 1 - 1.73123e-09) after pitchRatio calculation round-trip.
    // Round to 1.0 to avoid false positive pitch offset (pitchOctaves != 0).
    double pitchRatioDiffTo1 = fabs(1.0 - m_pitchRateInfo.pitchRatio);
    if (0.0 < pitchRatioDiffTo1 && pitchRatioDiffTo1 < pow(10, -9)) { // 0.000000001
        m_pitchRateInfo.pitchRatio = 1.0;
        // recalculating doesn't make sense here, the rounding offset will
        // occur again after updatePitch request from enginebuffer
        if constexpr (kEnableDebugOutput) {
            qDebug() << "   0.0 < pitchRatioDiffTo1 < 0.000000001";
            qDebug() << "   reset pitchRatio to 1.0";
            qDebug() << "   .";
        }
    }

    double pitchOctaves = KeyUtils::powerOf2ToOctaveChange(m_pitchRateInfo.pitchRatio);
    double dFileKey = m_pFileKey->get();
    updateKeyCOs(dFileKey, pitchOctaves);
}

void KeyControl::slotFileKeyChanged(double value) {
    updateKeyCOs(value,  m_pPitch->get() / 12);
}

void KeyControl::updateKeyCOs(double fileKeyNumeric, double pitchOctaves) {
    mixxx::track::io::key::ChromaticKey fileKey =
            KeyUtils::keyFromNumericValue(fileKeyNumeric);

    QPair<mixxx::track::io::key::ChromaticKey, double> adjusted =
            KeyUtils::scaleKeyOctaves(fileKey, pitchOctaves);
    m_pEngineKey->set(KeyUtils::keyToNumericValue(adjusted.first));
    double diff_to_nearest_full_key = adjusted.second;
    m_pEngineKeyDistance->set(diff_to_nearest_full_key);
    m_pPitch->set(pitchOctaves * 12);
    if constexpr (kEnableDebugOutput) {
        qDebug() << "       .";
        qDebug() << "       KeyControl::updateKeyCOs";
        qDebug() << "       | octaves       " << qSetRealNumberPrecision(18) << pitchOctaves;
        qDebug() << "       | diff          " << diff_to_nearest_full_key;
        qDebug() << "       : m_pPitch      " << m_pPitch->get();
        qDebug() << "       | m_pPitchAdjust" << m_pPitchAdjust->get();
        qDebug() << "       .";
    }
}

void KeyControl::slotSetEngineKey(double key) {
    // Always set to a full key, reset key_distance
    setEngineKey(key, 0.0);
}

void KeyControl::slotSetEngineKeyDistance(double key_distance) {
    setEngineKey(m_pEngineKey->get(), key_distance);
}

void KeyControl::setEngineKey(double key, double key_distance) {
    mixxx::track::io::key::ChromaticKey thisFileKey =
            KeyUtils::keyFromNumericValue(m_pFileKey->get());
    mixxx::track::io::key::ChromaticKey newKey =
            KeyUtils::keyFromNumericValue(key);

    if (thisFileKey == mixxx::track::io::key::INVALID ||
        newKey == mixxx::track::io::key::INVALID) {
        return;
    }

    int stepsToTake = KeyUtils::shortestStepsToKey(thisFileKey, newKey);
    double pitchToTakeOctaves = (stepsToTake + key_distance) / 12.0;

    m_pPitch->set(pitchToTakeOctaves * 12);
    slotPitchChanged(pitchToTakeOctaves * 12);
    return;
}

void KeyControl::slotPitchChanged(double pitch) {
    Q_UNUSED(pitch)
    m_updatePitchRequest = 1;
    updatePitch();
}

void KeyControl::updatePitch() {
    const double pitch = m_pPitch->get();

    if constexpr (kEnableDebugOutput) {
        qDebug() << "   .";
        qDebug() << "   KeyControl::updatePitch:" << pitch;
        qDebug() << "   | tempoRatio      " << m_pitchRateInfo.tempoRatio;
        qDebug() << "   | pitchRatio      " << m_pitchRateInfo.pitchRatio;
        qDebug() << "   | pitchTweakRatio " << m_pitchRateInfo.pitchTweakRatio;
        qDebug() << "   | m_pPitchAdjust  " << m_pPitchAdjust->get();
    }

    // speedSliderPitchRatio must be unchanged
    const double speedSliderPitchRatio =
            m_pitchRateInfo.pitchRatio / m_pitchRateInfo.pitchTweakRatio;
    m_pitchRateInfo.pitchRatio = KeyUtils::semitoneChangeToPowerOf2(pitch);
    m_pitchRateInfo.pitchTweakRatio =
            m_pitchRateInfo.pitchRatio / speedSliderPitchRatio;

    const double dFileKey = m_pFileKey->get();
    m_pPitchAdjust->set(
            KeyUtils::powerOf2ToSemitoneChange(m_pitchRateInfo.pitchTweakRatio));
    updateKeyCOs(dFileKey, KeyUtils::powerOf2ToOctaveChange(m_pitchRateInfo.pitchRatio));

    if constexpr (kEnableDebugOutput) {
        qDebug() << "   --after KeyControl::updatePitch";
        qDebug() << "   | pitchRatio      " << m_pitchRateInfo.pitchRatio;
        qDebug() << "   | pitchTweakRatio " << m_pitchRateInfo.pitchTweakRatio;
        qDebug() << "   | m_pPitchAdjust  " << m_pPitchAdjust->get();
        qDebug() << "   .";
    }
}

void KeyControl::slotPitchAdjustChanged(double pitchAdjust) {
    Q_UNUSED(pitchAdjust);
    m_updatePitchAdjustRequest = 1;
    updatePitchAdjust();
}

void KeyControl::updatePitchAdjust() {
    const double pitchAdjust = m_pPitchAdjust->get();

    if constexpr (kEnableDebugOutput) {
        qDebug() << "   .";
        qDebug() << "   KeyControl::updatePitchAdjust:" << pitchAdjust;
        qDebug() << "   | tempoRatio      " << m_pitchRateInfo.tempoRatio;
        qDebug() << "   | pitchRatio      " << m_pitchRateInfo.pitchRatio;
        qDebug() << "   | pitchTweakRatio " << m_pitchRateInfo.pitchTweakRatio;
        qDebug() << "   | m_pPitch        " << m_pPitch->get();
    }

    // speedSliderPitchRatio must be unchanged
    const double speedSliderPitchRatio =
            m_pitchRateInfo.pitchRatio / m_pitchRateInfo.pitchTweakRatio;
    m_pitchRateInfo.pitchTweakRatio = KeyUtils::semitoneChangeToPowerOf2(pitchAdjust);

    // pitch_adjust is an offset to the pitch set by the speed controls
    // calc absolute pitch
    m_pitchRateInfo.pitchRatio = m_pitchRateInfo.pitchTweakRatio * speedSliderPitchRatio;

    const double dFileKey = m_pFileKey->get();
    updateKeyCOs(dFileKey, KeyUtils::powerOf2ToOctaveChange(m_pitchRateInfo.pitchRatio));

    if constexpr (kEnableDebugOutput) {
        qDebug() << "   --after KeyControl::updatePitchAdjust";
        qDebug() << "   | pitchRatio      " << m_pitchRateInfo.pitchRatio;
        qDebug() << "   | pitchTweakRatio " << m_pitchRateInfo.pitchTweakRatio;
        qDebug() << "   | m_pPitch          " << m_pPitch->get();
        qDebug() << "   .";
    }
}

void KeyControl::slotSyncKey(double v) {
    if (v > 0) {
        EngineBuffer* pOtherEngineBuffer = pickSyncTarget();
        syncKey(pOtherEngineBuffer);
    }
}

void KeyControl::slotResetKey(double v) {
    if (v > 0) {
        if constexpr (kEnableDebugOutput) {
            qDebug() << "   .";
            qDebug() << "   KeyControl::slotResetKey";
            qDebug() << "   .";
        }
        m_pPitch->set(0);
        slotPitchChanged(0);
    }
}

bool KeyControl::syncKey(EngineBuffer* pOtherEngineBuffer) {
    if (!pOtherEngineBuffer) {
        return false;
    }

    mixxx::track::io::key::ChromaticKey thisFileKey =
            KeyUtils::keyFromNumericValue(m_pFileKey->get());

    // Get the sync target's effective key, since that is what we aim to match.
    double dKey = ControlObject::get(ConfigKey(pOtherEngineBuffer->getGroup(), "key"));
    mixxx::track::io::key::ChromaticKey otherKey =
            KeyUtils::keyFromNumericValue(dKey);
    double otherDistance = ControlObject::get(
            ConfigKey(pOtherEngineBuffer->getGroup(), "visual_key_distance"));

    if (thisFileKey == mixxx::track::io::key::INVALID ||
        otherKey == mixxx::track::io::key::INVALID) {
        return false;
    }

    int stepsToTake = KeyUtils::shortestStepsToCompatibleKey(thisFileKey, otherKey);
    double pitchToTakeOctaves = (stepsToTake + otherDistance) / 12.0;

    m_pPitch->set(pitchToTakeOctaves * 12);
    slotPitchChanged(pitchToTakeOctaves * 12);
    return true;
}
