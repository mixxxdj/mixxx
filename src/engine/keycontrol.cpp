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
          m_dPitchCompensationOldPitch(0.0) {
    m_pPitch = new ControlPotmeter(ConfigKey(group, "pitch"), -1.f, 1.f);
    // Course adjust by full step.
    m_pPitch->setStepCount(24);
    // Fine adjust by half-step / semitone.
    m_pPitch->setSmallStepCount(48);

    connect(m_pPitch, SIGNAL(valueChanged(double)),
            this, SLOT(slotPitchChanged(double)),
            Qt::DirectConnection);
    connect(m_pPitch, SIGNAL(valueChangedFromEngine(double)),
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
    delete m_pPitch;
    delete m_pButtonSyncKey;
    delete m_pFileKey;
    delete m_pEngineKey;
    delete m_pEngineKeyDistance;
}

double KeyControl::getPitchAdjustOctaves() {
    return m_pPitch->get();
}

double KeyControl::getKey() {
    return m_pEngineKey->get();
}

void KeyControl::slotRateChanged() {
    // If rate is non-1.0 then we have to try and calculate the octave change
    // caused by it.
    double dRate = 1.0 + m_pRateDir->get() * m_pRateRange->get() * m_pRateSlider->get();
    bool bKeylock = m_pKeylock->get() > 0;

    // If we just turned keylock on or off, adjust the pitch so that the
    // effective key stays the same. This is only relevant when m_dOldRate !=
    // 1.0 because that's the only case when rate adjustment causes pitch
    // change.
    if (bKeylock && !m_bOldKeylock) {
        double pitch = m_pPitch->get();
        m_dPitchCompensation = pitch + KeyUtils::powerOf2ToOctaveChange(m_dOldRate);
        m_dPitchCompensationOldPitch = pitch;
        m_pPitch->set(m_dPitchCompensation);
    } else if (!bKeylock && m_bOldKeylock) {
        double pitch = m_pPitch->get();

        // The pitch has not changed since we enabled keylock. Restore the
        // old pitch.
        if (pitch == m_dPitchCompensation) {
            m_pPitch->set(m_dPitchCompensationOldPitch);
        } else {
            // Otherwise, compensate in the opposite direction to prevent
            // pitch change. We know the user has a pitch control because
            // they changed it.
            double pitchAdjust = KeyUtils::powerOf2ToOctaveChange(m_dOldRate);
            m_pPitch->set(pitch - pitchAdjust);
        }

        m_dPitchCompensationOldPitch = 0.0;
        m_dPitchCompensation = 0.0;
    }

    if (m_dOldRate != dRate || bKeylock != m_bOldKeylock) {
        m_dOldRate = dRate;
        m_bOldKeylock = bKeylock;
        double dFileKey = m_pFileKey->get();
        slotFileKeyChanged(dFileKey);
    }
}

void KeyControl::slotFileKeyChanged(double value) {
    mixxx::track::io::key::ChromaticKey key =
            KeyUtils::keyFromNumericValue(value);

    // The pitch adjust in octaves.
    double pitch_adjust = m_pPitch->get();
    bool keylock_enabled = m_pKeylock->get() > 0;

    // If keylock is enabled then rate only affects the tempo and not the pitch.
    if (m_dOldRate != 1.0 && !keylock_enabled) {
        pitch_adjust += KeyUtils::powerOf2ToOctaveChange(m_dOldRate);
    }

    QPair<mixxx::track::io::key::ChromaticKey, double> adjusted =
            KeyUtils::scaleKeyOctaves(key, pitch_adjust);
    m_pEngineKey->set(KeyUtils::keyToNumericValue(adjusted.first));
    m_pEngineKeyDistance->set(adjusted.second);
}

void KeyControl::slotSetEngineKey(double key) {
    Q_UNUSED(key);
    // TODO(rryan): set m_pPitch to match the desired key.
}

void KeyControl::slotPitchChanged(double) {
    double dFileKey = m_pFileKey->get();
    slotFileKeyChanged(dFileKey);
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
    m_pPitch->set(newPitch);
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
