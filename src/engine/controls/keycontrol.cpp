#include "engine/controls/keycontrol.h"

#include <cmath>

#include <QPair>
#include <QThread>
#include <QtDebug>

#include "control/controlobject.h"
#include "control/controlpotmeter.h"
#include "control/controlproxy.h"
#include "control/controlpushbutton.h"
#include "engine/channels/enginechannel.h"
#include "engine/defs_keylock.h"
#include "engine/enginebuffer.h"
#include "mixer/playermanager.h"
#include "moc_keycontrol.cpp"
#include "track/keyutils.h"

constexpr bool kEnableDebugOutput = false;

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
          m_pButtonMatchKey(std::make_unique<ControlPushButton>(
                  ConfigKey(group, "match_key"))),
          m_pButtonSyncKeyLeader(std::make_unique<ControlPushButton>(
                  ConfigKey(group, "sync_key_leader"))),
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

    // Same hold-to-latch behavior as sync_enabled. The first deck to turn
    // this on lights the crown and keeps its key. A later deck matches that
    // leader. Turning sync off clears the crown. A long press stays on so a
    // follower keeps matching the leader on later loads. Clicking the crown
    // latches sync as well.
    m_pButtonSyncKey->setButtonMode(mixxx::control::ButtonMode::LongPressLatching);
    // Same three-state control as the beat crown. Explicit is not used: a
    // value of 2 is folded back to 1 (mixxx#11788).
    m_pButtonSyncKeyLeader->setBehavior(mixxx::control::ButtonMode::Toggle, 3);

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
            &KeyControl::slotSyncKeyButton,
            Qt::DirectConnection);

    // Push mode: press matches once and release returns to 0. sync_key is
    // left alone, so the Sync button does not light.
    connect(m_pButtonMatchKey.get(),
            &ControlObject::valueChanged,
            this,
            &KeyControl::slotSyncKey,
            Qt::DirectConnection);

    connect(m_pButtonSyncKeyLeader.get(),
            &ControlObject::valueChanged,
            this,
            &KeyControl::slotSyncKeyLeader,
            Qt::DirectConnection);

    connect(m_pButtonResetKey.get(),
            &ControlObject::valueChanged,
            this,
            &KeyControl::slotResetKey,
            Qt::DirectConnection);

    connect(this,
            &KeyControl::leaderKeyFollowRequested,
            this,
            &KeyControl::slotLeaderKeyFollow,
            Qt::QueuedConnection);

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
    m_pRateRatio->connectValueChanged(this, &KeyControl::slotRateChanged, Qt::DirectConnection);

    // VinylControl is only available on main decks
    if (PlayerManager::isDeckGroup(group)) {
        m_pVCEnabled = make_parented<ControlProxy>(group, "vinylcontrol_enabled", this);
        m_pVCEnabled->connectValueChanged(this, &KeyControl::slotRateChanged, Qt::DirectConnection);

        m_pVCRate = make_parented<ControlProxy>(group, "vinylcontrol_rate", this);
        m_pVCRate->connectValueChanged(this, &KeyControl::slotRateChanged, Qt::DirectConnection);
    }

    m_pKeylock = make_parented<ControlProxy>(group, "keylock", this);
    m_pKeylock->connectValueChanged(this, &KeyControl::slotRateChanged, Qt::DirectConnection);

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

/// This is called when rate_ratio, vinylcontrol_rate, vinylcontrol_enabled or
/// keylock are changed, but also when EngineBuffer::processTrackLocked requests
/// m_pitchRateInfo struct while rate, pitch or pitch_adjust were just updated.
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
        if (!m_pitchRateInfo.keylock) { // Enabling keylock
            if (m_keylockMode->get() ==
                    static_cast<double>(KeylockMode::LockCurrentKey)) {
                // Lock at current pitch
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
            if (m_keyunlockMode->get() == static_cast<double>(KeyunlockMode::KeepLockedKey)) {
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

/// This is called when the file key is changed (by analysis or user input),
/// or when a track with a different key is loaded
void KeyControl::slotFileKeyChanged(double value) {
    updateKeyCOs(value, m_pPitch->get() / 12);
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
    // The pitch write above can re-enter. Follow after the key COs are current.
    queueOrApplyLeaderKeyToFollowers();
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

/// This is called when pitch is changed either by user interaction,
/// or when BaseTrackPlayerImpl resets the pitch on track load per configuration
/// AND key is locked with KeylockMode::LockCurrentKey
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

/// This is called when pitch_adjust is changed either by user interaction,
/// or when BaseTrackPlayerImpl resets the pitch on track load per configuration
/// AND key is unlocked or locked with LockOriginalKey
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
        // The note. A crowned deck is the reference. Otherwise keep the
        // BPM sync target this one-shot button has always used.
        const QString leader = keyLeaderGroup(getGroup());
        if (!leader.isEmpty()) {
            if (applyMatchKeyPitch(getGroup(), leader, getConfig())) {
                slotPitchChanged(m_pPitch->get());
            }
            return;
        }
        EngineBuffer* pOtherEngineBuffer = pickSyncTarget();
        syncKey(pOtherEngineBuffer);
    }
}

namespace {

void clearOtherKeyCrowns(const QString& group) {
    const int numDecks = static_cast<int>(ControlObject::get(
            ConfigKey(QStringLiteral("[App]"), QStringLiteral("num_decks"))));
    for (int i = 0; i < numDecks; ++i) {
        const QString other = PlayerManager::groupForDeck(i);
        if (other == group) {
            continue;
        }
        const ConfigKey key(other, QStringLiteral("sync_key_leader"));
        // ControlPushButton does not ignore unchanged writes.
        if (ControlObject::get(key) >= 1.0) {
            ControlObject::set(key, 0.0);
        }
    }
}

// Another synced primary deck becomes the leader and keeps its own key.
// Prefer the opposite left/right deck, the same tie break as keyLeaderGroup.
void promoteOtherSyncedKeyLeader(const QString& group) {
    const int numDecks = static_cast<int>(ControlObject::get(
            ConfigKey(QStringLiteral("[App]"), QStringLiteral("num_decks"))));
    const double thisOrientation = ControlObject::get(
            ConfigKey(group, QStringLiteral("orientation")));
    QString bestGroup;
    bool bestOpposite = false;
    for (int i = 0; i < numDecks; ++i) {
        const QString other = PlayerManager::groupForDeck(i);
        if (other == group) {
            continue;
        }
        if (ControlObject::get(ConfigKey(other, QStringLiteral("sync_key"))) <= 0.0) {
            continue;
        }
        if (ControlObject::get(ConfigKey(other, QStringLiteral("sync_key_leader"))) >= 1.0) {
            continue;
        }
        const double orientation = ControlObject::get(
                ConfigKey(other, QStringLiteral("orientation")));
        const bool opposite =
                (thisOrientation == static_cast<double>(EngineChannel::LEFT) &&
                        orientation == static_cast<double>(EngineChannel::RIGHT)) ||
                (thisOrientation == static_cast<double>(EngineChannel::RIGHT) &&
                        orientation == static_cast<double>(EngineChannel::LEFT));
        if (bestGroup.isEmpty() || (opposite && !bestOpposite)) {
            bestGroup = other;
            bestOpposite = opposite;
        }
    }
    if (!bestGroup.isEmpty()) {
        ControlObject::set(
                ConfigKey(bestGroup, QStringLiteral("sync_key_leader")), 1.0);
    }
}

} // namespace

void KeyControl::slotSyncKeyButton(double v) {
    // A crown click latches sync. That write must not retune or clear the crown.
    if (m_applyingCrownSync) {
        return;
    }
    if (v <= 0.0) {
        // Leaving sync clears the crown, the same as beat sync mode None.
        // The crown write does not re-enter slotSyncKeyLeader.
        dropKeyCrown();
        return;
    }
    // This deck is already the reference. A click must not retune it.
    if (isKeyMatchReference(getGroup())) {
        return;
    }
    const QString leader = keyLeaderGroup(getGroup());
    if (!leader.isEmpty()) {
        if (applyMatchKeyPitch(getGroup(), leader, getConfig())) {
            slotPitchChanged(m_pPitch->get());
        }
        return;
    }
    // Nobody else is crowned. Light the crown and keep this key.
    // The crown write does not re-enter slotSyncKeyLeader, so follow here.
    if (m_pButtonSyncKeyLeader->get() < 1.0) {
        m_pButtonSyncKeyLeader->set(1.0);
        queueOrApplyLeaderKeyToFollowers();
    }
}

void KeyControl::dropKeyCrown() {
    if (m_pButtonSyncKeyLeader->get() < 1.0) {
        return;
    }
    // This set does not re-enter slotSyncKeyLeader.
    m_pButtonSyncKeyLeader->set(0.0);
    if (keyLeaderGroup(getGroup()).isEmpty()) {
        promoteOtherSyncedKeyLeader(getGroup());
    }
}

void KeyControl::queueOrApplyLeaderKeyToFollowers() {
    // Uncrowned decks never drive followers. get() is safe on the engine thread.
    if (m_pButtonSyncKeyLeader->get() < 1.0) {
        return;
    }
    if (QThread::currentThread() != thread()) {
        const int key = static_cast<int>(std::lround(m_pEngineKey->get()));
        const int distanceMicro = static_cast<int>(
                std::lround(m_pEngineKeyDistance->get() * 1000000.0));
        if (m_leaderFollowValid.loadAcquire() != 0 &&
                key == m_lastFollowedKey.loadAcquire() &&
                distanceMicro == m_lastFollowedDistanceMicro.loadAcquire()) {
            return;
        }
        if (m_leaderFollowQueued.testAndSetOrdered(0, 1)) {
            emit leaderKeyFollowRequested();
        }
        return;
    }
    applyLeaderKeyToFollowers();
}

void KeyControl::slotLeaderKeyFollow() {
    m_leaderFollowQueued.storeRelease(0);
    applyLeaderKeyToFollowers();
}

void KeyControl::applyLeaderKeyToFollowers() {
    if (!isKeyMatchReference(getGroup())) {
        return;
    }
    const int key = static_cast<int>(std::lround(m_pEngineKey->get()));
    const int distanceMicro = static_cast<int>(
            std::lround(m_pEngineKeyDistance->get() * 1000000.0));
    const int numDecks = static_cast<int>(ControlObject::get(
            ConfigKey(QStringLiteral("[App]"), QStringLiteral("num_decks"))));
    for (int i = 0; i < numDecks; ++i) {
        const QString group = PlayerManager::groupForDeck(i);
        if (group == getGroup()) {
            continue;
        }
        // Sync latched, and not itself the reference.
        if (ControlObject::get(ConfigKey(group, QStringLiteral("sync_key"))) <= 0.0 ||
                isKeyMatchReference(group)) {
            continue;
        }
        applyMatchKeyPitch(group, getGroup(), getConfig());
    }
    m_lastFollowedKey.storeRelease(key);
    m_lastFollowedDistanceMicro.storeRelease(distanceMicro);
    m_leaderFollowValid.storeRelease(1);
}

void KeyControl::slotSyncKeyLeader(double v) {
    // Explicit is disabled. A request above the crown stays at 1, matching
    // the beat crown (mixxx#11788). This set does not re-enter the slot.
    if (v >= 2.0) {
        m_pButtonSyncKeyLeader->set(1.0);
        v = 1.0;
    }
    if (v >= 1.0) {
        // One crown. Clear the others before follow so this deck is the reference.
        clearOtherKeyCrowns(getGroup());
        // A lit crown turns key sync on and leaves it on. This deck's key stays.
        if (m_pButtonSyncKey->get() <= 0.0) {
            m_applyingCrownSync = true;
            m_pButtonSyncKey->set(1.0);
            m_applyingCrownSync = false;
        }
        queueOrApplyLeaderKeyToFollowers();
        return;
    }
    // Crown off. Sync stays on. If another deck already has the crown, this
    // one was cleared because they took it.
    if (!keyLeaderGroup(getGroup()).isEmpty()) {
        return;
    }
    promoteOtherSyncedKeyLeader(getGroup());
}

void KeyControl::slotResetKey(double v) {
    if (v > 0) {
        if constexpr (kEnableDebugOutput) {
            qDebug() << "   .";
            qDebug() << "   KeyControl::slotResetKey";
            qDebug() << "   .";
        }
        // A follower must leave sync before pitch returns to 0, or the
        // leader would pull it back. Own set() does not re-enter the
        // sync slot, so drop a crown here too.
        // The leader keeps sync and the crown. Pitch 0 then runs the
        // same follow as any other pitch change, so synced decks match
        // this original key.
        if (!isKeyMatchReference(getGroup())) {
            if (m_pButtonSyncKey->get() > 0.0) {
                m_pButtonSyncKey->set(0.0);
            }
            dropKeyCrown();
        }
        m_pPitch->set(0);
        slotPitchChanged(0);
    }
}

bool KeyControl::applyMatchKeyPitch(const QString& deckGroup,
        const QString& otherDeckGroup,
        const UserSettingsPointer& pConfig) {
    if (deckGroup.isEmpty() || otherDeckGroup.isEmpty() || deckGroup == otherDeckGroup) {
        return false;
    }

    const mixxx::track::io::key::ChromaticKey thisFileKey =
            KeyUtils::keyFromNumericValue(ControlObject::get(ConfigKey(deckGroup, "file_key")));
    // Effective key of the deck we are matching, including its current pitch.
    const mixxx::track::io::key::ChromaticKey otherKey =
            KeyUtils::keyFromNumericValue(ControlObject::get(ConfigKey(otherDeckGroup, "key")));
    const double otherDistance = ControlObject::get(
            ConfigKey(otherDeckGroup, "visual_key_distance"));

    const std::optional<double> pitch = KeyUtils::matchKeyPitchSemitones(
            thisFileKey, otherKey, otherDistance, exactKeyMatchEnabled(pConfig));
    if (!pitch.has_value()) {
        return false;
    }

    // `pitch` is in semitones. Setting the control runs KeyControl::slotPitchChanged
    // for the deck that owns it, which is the same write `sync_key` performs.
    ControlObject::set(ConfigKey(deckGroup, "pitch"), *pitch);
    return true;
}

bool KeyControl::autoHarmonizeOnLoadEnabled(const UserSettingsPointer& pConfig) {
    if (!pConfig) {
        return false;
    }
    if (pConfig->exists(kAutoHarmonizeOnLoadConfigKey)) {
        return pConfig->getValue<bool>(kAutoHarmonizeOnLoadConfigKey, false);
    }
    // Draft builds stored two checkboxes. Either one still opts in until
    // the Decks page writes the single key.
    const bool retiredAutoDj = pConfig->getValue<bool>(
            ConfigKey(QStringLiteral("[Auto DJ]"),
                    QStringLiteral("AutoHarmonizeToPrevious")),
            false);
    const bool retiredManual = pConfig->getValue<bool>(
            ConfigKey(QStringLiteral("[Key]"),
                    QStringLiteral("AutoHarmonizeOnManualLoad")),
            false);
    return retiredAutoDj || retiredManual;
}

bool KeyControl::exactKeyMatchEnabled(const UserSettingsPointer& pConfig) {
    if (!pConfig) {
        return false;
    }
    // No retired key. An unset preference stays on the compatible match.
    return pConfig->getValue<bool>(kExactKeyMatchConfigKey, false);
}

QString KeyControl::keyLeaderGroup(const QString& excludeGroup) {
    const int numDecks = static_cast<int>(ControlObject::get(
            ConfigKey(QStringLiteral("[App]"), QStringLiteral("num_decks"))));
    const double thisOrientation = ControlObject::get(
            ConfigKey(excludeGroup, QStringLiteral("orientation")));
    QString bestGroup;
    double bestState = 0.0;
    bool bestOpposite = false;
    for (int i = 0; i < numDecks; ++i) {
        const QString group = PlayerManager::groupForDeck(i);
        if (group == excludeGroup) {
            continue;
        }
        const double state = ControlObject::get(
                ConfigKey(group, QStringLiteral("sync_key_leader")));
        if (state < 1.0) {
            continue;
        }
        const double orientation = ControlObject::get(
                ConfigKey(group, QStringLiteral("orientation")));
        const bool opposite =
                (thisOrientation == static_cast<double>(EngineChannel::LEFT) &&
                        orientation == static_cast<double>(EngineChannel::RIGHT)) ||
                (thisOrientation == static_cast<double>(EngineChannel::RIGHT) &&
                        orientation == static_cast<double>(EngineChannel::LEFT));
        const bool betterState = state > bestState;
        const bool betterSide = state == bestState && opposite && !bestOpposite;
        if (bestGroup.isEmpty() || betterState || betterSide) {
            bestGroup = group;
            bestState = state;
            bestOpposite = opposite;
        }
    }
    return bestGroup;
}

bool KeyControl::isKeyMatchReference(const QString& deckGroup) {
    const double mine = ControlObject::get(
            ConfigKey(deckGroup, QStringLiteral("sync_key_leader")));
    if (mine < 1.0) {
        return false;
    }
    // A value of 2 is the reference even if another deck is also lit.
    // KeyControl folds that back to 1. A bare control can still sit at 2.
    if (mine >= 2.0) {
        return true;
    }
    // The crown is the reference only when nothing else is crowned.
    return keyLeaderGroup(deckGroup).isEmpty();
}

bool KeyControl::shouldHarmonizeOnLoad(
        const QString& deckGroup, const UserSettingsPointer& pConfig) {
    if (deckGroup.isEmpty() || isKeyMatchReference(deckGroup)) {
        return false;
    }
    const bool latched =
            ControlObject::get(ConfigKey(deckGroup, QStringLiteral("sync_key"))) > 0.0;
    return latched || autoHarmonizeOnLoadEnabled(pConfig);
}

bool KeyControl::syncKey(EngineBuffer* pOtherEngineBuffer) {
    if (!pOtherEngineBuffer) {
        return false;
    }

    if (!applyMatchKeyPitch(getGroup(), pOtherEngineBuffer->getGroup(), getConfig())) {
        return false;
    }
    // set() does not emit when the value is unchanged. Repeat the pitch update
    // so ratios stay consistent, matching the previous sync_key behavior.
    slotPitchChanged(m_pPitch->get());
    return true;
}
