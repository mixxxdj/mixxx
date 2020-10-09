#include "engine/controls/keycontrol.h"

#include <QPair>
#include <QtDebug>

#include "control/controlobject.h"
#include "control/controlpotmeter.h"
#include "control/controlproxy.h"
#include "control/controlpushbutton.h"
#include "engine/enginebuffer.h"
#include "track/keyutils.h"

//static const double kLockOriginalKey = 0;
static const double kLockCurrentKey = 1;
static const double kKeepUnlockedKey = 1;

KeyControl::KeyControl(QString group,
                       UserSettingsPointer pConfig)
        : EngineControl(group, pConfig) {
    m_pitchRateInfo.pitchRatio = 1.0;
    m_pitchRateInfo.tempoRatio = 1.0;
    m_pitchRateInfo.pitchTweakRatio = 1.0;
    m_pitchRateInfo.keylock = false;

    // pitch is the distance to the original pitch in semitones
    // knob in semitones; 9.4 ct per midi step allowOutOfBounds = true;
    m_pPitch = new ControlPotmeter(ConfigKey(group, "pitch"), -6.0, 6.0, true);
    // Coarse adjust by full semitone steps.
    m_pPitch->setStepCount(12);
    // Fine adjust with semitone / 10 = 10 ct;.
    m_pPitch->setSmallStepCount(120);
    connect(m_pPitch, &ControlObject::valueChanged,
            this, &KeyControl::slotPitchChanged,
            Qt::DirectConnection);

    // pitch_adjust is the distance to the linear pitch in semitones
    // set by the speed slider or to the locked key.
    // pitch_adjust knob in semitones; 4.7 ct per midi step; allowOutOfBounds = true;
    m_pPitchAdjust = new ControlPotmeter(ConfigKey(group, "pitch_adjust"), -3.0, 3.0, true);
    // Coarse adjust by full semitone steps.
    m_pPitchAdjust->setStepCount(6);
    // Fine adjust with semitone / 10 = 10 ct;.
    m_pPitchAdjust->setSmallStepCount(60);
    connect(m_pPitchAdjust, &ControlObject::valueChanged,
            this, &KeyControl::slotPitchAdjustChanged,
            Qt::DirectConnection);

    m_pButtonSyncKey = new ControlPushButton(ConfigKey(group, "sync_key"));
    connect(m_pButtonSyncKey, &ControlObject::valueChanged,
            this, &KeyControl::slotSyncKey,
            Qt::DirectConnection);

    m_pButtonResetKey = new ControlPushButton(ConfigKey(group, "reset_key"));
    connect(m_pButtonResetKey, &ControlObject::valueChanged,
            this, &KeyControl::slotResetKey,
            Qt::DirectConnection);

    m_pFileKey = new ControlObject(ConfigKey(group, "file_key"));
    connect(m_pFileKey, &ControlObject::valueChanged,
            this, &KeyControl::slotFileKeyChanged,
            Qt::DirectConnection);

    m_pEngineKey = new ControlObject(ConfigKey(group, "key"));
    connect(m_pEngineKey, &ControlObject::valueChanged,
            this, &KeyControl::slotSetEngineKey,
            Qt::DirectConnection);

    m_pEngineKeyDistance = new ControlPotmeter(ConfigKey(group, "visual_key_distance"),
                                               -0.5, 0.5);
    connect(m_pEngineKeyDistance, &ControlObject::valueChanged,
            this, &KeyControl::slotSetEngineKeyDistance,
            Qt::DirectConnection);

    m_keylockMode = new ControlPushButton(ConfigKey(group, "keylockMode"));
    m_keylockMode->setButtonMode(ControlPushButton::TOGGLE);

    m_keyunlockMode = new ControlPushButton(ConfigKey(group, "keyunlockMode"));
    m_keyunlockMode->setButtonMode(ControlPushButton::TOGGLE);

    // In case of vinyl control "rate_ratio" is a filtered mean value for display
    m_pRateRatio = make_parented<ControlProxy>(ConfigKey(group, "rate_ratio"), this);
    m_pRateRatio->connectValueChanged(this, &KeyControl::slotRateChanged,
            Qt::DirectConnection);

    m_pVCEnabled = new ControlProxy(group, "vinylcontrol_enabled", this);
    m_pVCEnabled->connectValueChanged(this, &KeyControl::slotRateChanged,
            Qt::DirectConnection);

    m_pVCRate = new ControlProxy(group, "vinylcontrol_rate", this);
    m_pVCRate->connectValueChanged(this, &KeyControl::slotRateChanged,
                Qt::DirectConnection);

    m_pKeylock = new ControlProxy(group, "keylock", this);
    m_pKeylock->connectValueChanged(this, &KeyControl::slotRateChanged,
            Qt::DirectConnection);
}

KeyControl::~KeyControl() {
    delete m_pPitch;
    delete m_pPitchAdjust;
    delete m_pButtonSyncKey;
    delete m_pButtonResetKey;
    delete m_pFileKey;
    delete m_pEngineKey;
    delete m_pEngineKeyDistance;
    delete m_keylockMode;
    delete m_keyunlockMode;
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

void KeyControl::updateRate() {
    //qDebug() << "KeyControl::slotRateChanged 1" << m_pitchRateInfo.pitchRatio;

    // If rate is not 1.0 then we have to try and calculate the octave change
    // caused by it.

    if(m_pVCEnabled->toBool()) {
        m_pitchRateInfo.tempoRatio = m_pVCRate->get();
    } else {
        m_pitchRateInfo.tempoRatio = m_pRateRatio->get();
    }

    if (m_pitchRateInfo.tempoRatio == 0) {
        // no transport, no pitch
        // so we can skip pitch calculation
        return;
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
        if (!m_pitchRateInfo.keylock) {
            // Enabling Keylock
            if (m_keylockMode->get() == kLockCurrentKey) {
                // Lock at current pitch
                speedSliderPitchRatio = m_pitchRateInfo.tempoRatio;
            } else {
                // kOffsetScaleLockOriginalKey
                // Lock at original track pitch
                speedSliderPitchRatio = 1.0;
            }
            m_pitchRateInfo.keylock = true;
        }
    } else {
        // !bKeylock
        if (m_pitchRateInfo.keylock) {
            // Disabling Keylock
            // If "Keep unlocked key" is enabled
            if (m_keyunlockMode->get() == kKeepUnlockedKey) {
                // don't reset to linear pitch, instead adopt speedSliderPitchRatio
                // change as pitchTweakRatio
                m_pitchRateInfo.pitchTweakRatio *=
                        (speedSliderPitchRatio / m_pitchRateInfo.tempoRatio);
                // adopt pitch_adjust now so that it doesn't jump and resets key
                // when touching pitch_adjust knob after unlock with offset key
                m_pPitchAdjust->set(
                        KeyUtils::powerOf2ToSemitoneChange(m_pitchRateInfo.pitchTweakRatio));
            } else {
                // If 'current' aka 'not original' key was locked
                if (m_keylockMode->get() == kLockCurrentKey) {
                    // reset to linear pitch
                    m_pitchRateInfo.pitchTweakRatio = 1.0;
                }
            }
            m_pitchRateInfo.keylock = false;
        }
        speedSliderPitchRatio = m_pitchRateInfo.tempoRatio;
    }

    m_pitchRateInfo.pitchRatio = m_pitchRateInfo.pitchTweakRatio * speedSliderPitchRatio;

    double pitchOctaves = KeyUtils::powerOf2ToOctaveChange(m_pitchRateInfo.pitchRatio);
    double dFileKey = m_pFileKey->get();
    updateKeyCOs(dFileKey, pitchOctaves);

    // qDebug() << "KeyControl::slotRateChanged 2" << m_pitchRatio << m_speedSliderPitchRatio;
}

void KeyControl::slotFileKeyChanged(double value) {
    updateKeyCOs(value,  m_pPitch->get() / 12);
}

void KeyControl::updateKeyCOs(double fileKeyNumeric, double pitchOctaves) {
    //qDebug() << "updateKeyCOs 1" << pitchOctaves;
    mixxx::track::io::key::ChromaticKey fileKey =
            KeyUtils::keyFromNumericValue(fileKeyNumeric);

    QPair<mixxx::track::io::key::ChromaticKey, double> adjusted =
            KeyUtils::scaleKeyOctaves(fileKey, pitchOctaves);
    m_pEngineKey->set(KeyUtils::keyToNumericValue(adjusted.first));
    double diff_to_nearest_full_key = adjusted.second;
    m_pEngineKeyDistance->set(diff_to_nearest_full_key);
    m_pPitch->set(pitchOctaves * 12);
    //qDebug() << "updateKeyCOs 2" << diff_to_nearest_full_key;
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
    double pitch = m_pPitch->get();

    //qDebug() << "KeyControl::slotPitchChanged 1" << pitch <<
    //        m_pitchRateInfo.pitchRatio <<
    //        m_pitchRateInfo.pitchTweakRatio <<
    //        m_pitchRateInfo.tempoRatio;

    double speedSliderPitchRatio =
            m_pitchRateInfo.pitchRatio / m_pitchRateInfo.pitchTweakRatio;
    // speedSliderPitchRatio must be unchanged
    double pitchKnobRatio = KeyUtils::semitoneChangeToPowerOf2(pitch);
    m_pitchRateInfo.pitchRatio = pitchKnobRatio;
    m_pitchRateInfo.pitchTweakRatio = pitchKnobRatio / speedSliderPitchRatio;

    double dFileKey = m_pFileKey->get();
    m_pPitchAdjust->set(
            KeyUtils::powerOf2ToSemitoneChange(m_pitchRateInfo.pitchTweakRatio));
    updateKeyCOs(dFileKey, KeyUtils::powerOf2ToOctaveChange(pitchKnobRatio));

    //qDebug() << "KeyControl::slotPitchChanged 2" << pitch <<
    //        m_pitchRateInfo.pitchRatio <<
    //        m_pitchRateInfo.pitchTweakRatio <<
    //        m_pitchRateInfo.tempoRatio;
}

void KeyControl::slotPitchAdjustChanged(double pitchAdjust) {
    Q_UNUSED(pitchAdjust);
    m_updatePitchAdjustRequest = 1;
    updatePitchAdjust();
}

void KeyControl::updatePitchAdjust() {
    double pitchAdjust = m_pPitchAdjust->get();

    //qDebug() << "KeyControl::slotPitchAdjustChanged 1" << pitchAdjust <<
    //        m_pitchRateInfo.pitchRatio <<
    //        m_pitchRateInfo.pitchTweakRatio <<
    //        m_pitchRateInfo.tempoRatio;

    double speedSliderPitchRatio = m_pitchRateInfo.pitchRatio / m_pitchRateInfo.pitchTweakRatio;
    // speedSliderPitchRatio must be unchanged
    double pitchAdjustKnobRatio = KeyUtils::semitoneChangeToPowerOf2(pitchAdjust);

    // pitch_adjust is an offset to the pitch set by the speed controls
    // calc absolute pitch
    m_pitchRateInfo.pitchTweakRatio = pitchAdjustKnobRatio;
    m_pitchRateInfo.pitchRatio = pitchAdjustKnobRatio * speedSliderPitchRatio;

    double dFileKey = m_pFileKey->get();
    updateKeyCOs(dFileKey, KeyUtils::powerOf2ToOctaveChange(m_pitchRateInfo.pitchRatio));

    //qDebug() << "KeyControl::slotPitchAdjustChanged 2" << pitchAdjust <<
    //        m_pitchRateInfo.pitchRatio <<
    //        m_pitchRateInfo.pitchTweakRatio <<
    //        m_pitchRateInfo.tempoRatio;
}

void KeyControl::slotSyncKey(double v) {
    if (v > 0) {
        EngineBuffer* pOtherEngineBuffer = pickSyncTarget();
        syncKey(pOtherEngineBuffer);
    }
}

void KeyControl::slotResetKey(double v) {
    if (v > 0) {
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
