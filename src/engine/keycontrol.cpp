#include <QtDebug>
#include <QPair>

#include "engine/keycontrol.h"

#include "controlobject.h"
#include "controlpushbutton.h"
#include "controlpotmeter.h"
#include "engine/enginebuffer.h"
#include "track/keyutils.h"

KeyControl::KeyControl(QString group,
                       ConfigObject<ConfigValue>* pConfig)
        : EngineControl(group, pConfig),
          m_dOldRate(0.0),
          m_bOldKeylock(false),
          m_dPitchCompensation(0.0),
          m_dPitchCompensationOldPitch(0.0),
          m_speedSliderPitchRatio(1.0),
          m_pitchRatio(1.0) {
    m_pPitchAdjust = new ControlPotmeter(ConfigKey(group, "pitch_adjust"), -1.f, 1.f);
    m_pPitch = new ControlPotmeter(ConfigKey(group, "pitch"), -1.f, 1.f);
    // Course adjust by full step.
    m_pPitchAdjust->setStepCount(24);
    m_pPitch->setStepCount(24);
    // Fine adjust by half-step / semitone.
    m_pPitchAdjust->setSmallStepCount(48);
    m_pPitch->setSmallStepCount(48);

    connect(m_pPitch, SIGNAL(valueChanged(double)),
            this, SLOT(slotPitchChanged(double)),
            Qt::DirectConnection);

    m_pButtonSyncKey = new ControlPushButton(ConfigKey(group, "sync_key"));
    connect(m_pButtonSyncKey, SIGNAL(valueChanged(double)),
            this, SLOT(slotSyncKey(double)),
            Qt::DirectConnection);

    m_pFileKey = new ControlObject(ConfigKey(group, "file_key"));
    connect(m_pFileKey, SIGNAL(valueChanged(double)),
            this, SLOT(slotFileKeyChanged(double)),
            Qt::DirectConnection);

    m_pEngineKey = new ControlObject(ConfigKey(group, "key"));
    connect(m_pEngineKey, SIGNAL(valueChanged(double)),
            this, SLOT(slotSetEngineKey(double)),
            Qt::DirectConnection);

    m_pEngineKeyDistance = new ControlPotmeter(ConfigKey(group, "visual_key_distance"),
                                               -0.5, 0.5);

    m_pRateSlider = ControlObject::getControl(ConfigKey(group, "rate"));
    connect(m_pRateSlider, SIGNAL(valueChanged(double)),
            this, SLOT(slotRateChanged()),
            Qt::DirectConnection);
    connect(m_pRateSlider, SIGNAL(valueChangedFromEngine(double)),
            this, SLOT(slotRateChanged()),
            Qt::DirectConnection);

    m_pRateRange = ControlObject::getControl(ConfigKey(group, "rateRange"));
    connect(m_pRateRange, SIGNAL(valueChanged(double)),
            this, SLOT(slotRateChanged()),
            Qt::DirectConnection);
    connect(m_pRateRange, SIGNAL(valueChangedFromEngine(double)),
            this, SLOT(slotRateChanged()),
            Qt::DirectConnection);

    m_pRateDir = ControlObject::getControl(ConfigKey(group, "rate_dir"));
    connect(m_pRateDir, SIGNAL(valueChanged(double)),
            this, SLOT(slotRateChanged()),
            Qt::DirectConnection);
    connect(m_pRateDir, SIGNAL(valueChangedFromEngine(double)),
            this, SLOT(slotRateChanged()),
            Qt::DirectConnection);

    m_pKeylock = ControlObject::getControl(ConfigKey(group, "keylock"));
    connect(m_pKeylock, SIGNAL(valueChanged(double)),
            this, SLOT(slotRateChanged()),
            Qt::DirectConnection);
    connect(m_pKeylock, SIGNAL(valueChangedFromEngine(double)),
            this, SLOT(slotRateChanged()),
            Qt::DirectConnection);
}

KeyControl::~KeyControl() {
    delete m_pPitchAdjust;
    delete m_pPitch;
    delete m_pButtonSyncKey;
    delete m_pFileKey;
    delete m_pEngineKey;
    delete m_pEngineKeyDistance;
}

double KeyControl::getPitchRatio() const {
    return m_pitchRatio;
}

void KeyControl::resetPitchToLinear() {
    // TODO(DSC)
}

double KeyControl::getKey() {
    return m_pEngineKey->get();
}

void KeyControl::slotRateChanged() {
    // If rate is not 1.0 then we have to try and calculate the octave change
    // caused by it.
    double dRate = 1.0 + m_pRateDir->get() * m_pRateRange->get() * m_pRateSlider->get();
    bool bKeylock = m_pKeylock->toBool();
    int pitchAndKeylock = m_pConfig->getValueString(
            ConfigKey("[Controls]", "PitchAndKeylock"), "0").toInt();

    // |-----------------------|-----------------|
    //   SpeedSliderPitchRatio   pitchTweakRatio
    //
    // |-----------------------------------------|
    //   m_pitchRatio
    //
    //                         |-----------------|
    //                           m_pPitch (0)
    //
    // |-----------------------------------------|
    //   m_pPitch (1)

    // here is a possible race condition, if the pitch is changed in between.
    // but it cannot happen if rate and pitch is set from the same thread

    double pitchTweakRatio = m_pitchRatio / m_speedSliderPitchRatio;

    if (bKeylock) {
        if (!m_bOldKeylock) {
            // Enabling Keylock
            if (pitchAndKeylock) {
                // Lock at current pitch
                m_speedSliderPitchRatio = dRate;
            } else {
                // Lock at original track pitch
                m_speedSliderPitchRatio = 1.0;
            }
        }
    } else {
        // !bKeylock
        if (m_bOldKeylock) {
            // Disabling Keylock
            if (pitchAndKeylock) {
                // Adopt speedPitchRatio change as pitchTweakRatio
                pitchTweakRatio *= (m_speedSliderPitchRatio / dRate);
            }
        }
        m_speedSliderPitchRatio = dRate;
    }
    m_bOldKeylock = bKeylock;

    m_pitchRatio = pitchTweakRatio * m_speedSliderPitchRatio;

    double pitchOctaves = KeyUtils::powerOf2ToOctaveChange(m_pitchRatio);
    double dFileKey = m_pFileKey->get();
    updateKeyCOs(dFileKey, pitchOctaves);

    if (pitchAndKeylock) {
        // Pitch scale is always 0 at original pitch
        m_pPitch->set(pitchOctaves);
    }
}

void KeyControl::slotFileKeyChanged(double value) {
    updateKeyCOs(value, KeyUtils::powerOf2ToOctaveChange(m_pitchRatio));
}

void KeyControl::updateKeyCOs(double fileKeyNumeric, double pitch) {
    mixxx::track::io::key::ChromaticKey fileKey =
            KeyUtils::keyFromNumericValue(fileKeyNumeric);

    QPair<mixxx::track::io::key::ChromaticKey, double> adjusted =
            KeyUtils::scaleKeyOctaves(fileKey, pitch);
    m_pEngineKey->set(KeyUtils::keyToNumericValue(adjusted.first));
    double diff_to_nearest_full_key = adjusted.second;
    m_pEngineKeyDistance->set(diff_to_nearest_full_key);
}


void KeyControl::slotSetEngineKey(double key) {
    Q_UNUSED(key);
    // TODO(rryan): set m_pPitch to match the desired key.
}

void KeyControl::slotPitchChanged(double pitch) {
    int m_pitchAndKeylock = m_pConfig->getValueString(
            ConfigKey("[Controls]", "PitchAndKeylock"), "0").toInt();

    double pitchTweakRatio = KeyUtils::octaveChangeToPowerOf2(pitch);
    if (m_pitchAndKeylock == 0) {
        pitchTweakRatio *= m_speedSliderPitchRatio;
    }

    m_pitchRatio = pitchTweakRatio;

    double dFileKey = m_pFileKey->get();
    updateKeyCOs(dFileKey, KeyUtils::powerOf2ToOctaveChange(m_pitchRatio));
}

void KeyControl::slotSyncKey(double v) {
    if (v > 0) {
        EngineBuffer* pOtherEngineBuffer = pickSyncTarget();
        syncKey(pOtherEngineBuffer);
    }
}

bool KeyControl::syncKey(EngineBuffer* pOtherEngineBuffer) {
    if (!pOtherEngineBuffer) {
        return false;
    }

    mixxx::track::io::key::ChromaticKey thisFileKey =
            KeyUtils::keyFromNumericValue(m_pFileKey->get());

    // Get the sync target's effective key, since that is what we aim to match.
    ControlObject otherKeyControl(ConfigKey(pOtherEngineBuffer->getGroup(), "key"));
    mixxx::track::io::key::ChromaticKey otherKey =
            KeyUtils::keyFromNumericValue(otherKeyControl.get());

    if (thisFileKey == mixxx::track::io::key::INVALID ||
        otherKey == mixxx::track::io::key::INVALID) {
        return false;
    }

    int stepsToTake = KeyUtils::shortestStepsToCompatibleKey(thisFileKey, otherKey);
    double newPitch = KeyUtils::stepsToOctaveChange(stepsToTake);
    // Compensate for the existing rate adjustment.
    bool keylock_enabled = m_pKeylock->get() > 0;
    if (m_dOldRate != 1.0 && !keylock_enabled) {
        newPitch -= KeyUtils::powerOf2ToOctaveChange(m_dOldRate);
    }
    m_pPitchAdjust->set(newPitch);
    return true;
}

void KeyControl::collectFeatures(GroupFeatureState* pGroupFeatures) const {
    mixxx::track::io::key::ChromaticKey fileKey =
            KeyUtils::keyFromNumericValue(m_pFileKey->get());
    if (fileKey != mixxx::track::io::key::INVALID) {
        pGroupFeatures->has_file_key = true;
        pGroupFeatures->file_key = fileKey;
    }

    mixxx::track::io::key::ChromaticKey key =
            KeyUtils::keyFromNumericValue(m_pEngineKey->get());
    if (key != mixxx::track::io::key::INVALID) {
        pGroupFeatures->has_key = true;
        pGroupFeatures->key = key;
    }
}
