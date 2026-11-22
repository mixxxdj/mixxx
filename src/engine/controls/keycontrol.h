#pragma once

#include "engine/controls/enginecontrol.h"
#include "preferences/usersettings.h"
#include "util/parented_ptr.h"

// Decks preference. Default off. Item name matches the other [Controls] keys.
inline const ConfigKey kAutoHarmonizeOnLoadConfigKey(
        QStringLiteral("[Controls]"),
        QStringLiteral("AutoHarmonizeOnLoad"));

// Decks preference. Default off, so match stays Circle-of-Fifths compatible.
inline const ConfigKey kExactKeyMatchConfigKey(
        QStringLiteral("[Controls]"),
        QStringLiteral("ExactKeyMatch"));

class ControlObject;
class ControlProxy;
class ControlPotmeter;
class ControlPushButton;

class KeyControl : public EngineControl {
    Q_OBJECT
  public:
    struct PitchTempoRatio {
        // This is the calculated value used by engine buffer for pitch.
        // By default it is equal to the tempoRatio set by the speed slider
        double pitchRatio;
        // This is the value of the speed slider and speed slider
        // affecting controls at the moment of calculation
        double tempoRatio;
        // the offeset factor to the natural pitch set by the rate slider
        double pitchTweakRatio;
        bool keylock;
    };

    KeyControl(const QString& group, UserSettingsPointer pConfig);

    // Returns a struct, with the results of the last pitch and tempo calculations
    KeyControl::PitchTempoRatio getPitchTempoRatio();

    double getKey();

    /// Writes `deckGroup`'s pitch the same way `sync_key` does, using
    /// `otherDeckGroup`'s effective key (`key` + `visual_key_distance`).
    /// Exact versus a Circle-of-Fifths compatible key comes from the
    /// Decks Exact key checkbox. Returns false without changing pitch
    /// when either key is invalid.
    static bool applyMatchKeyPitch(const QString& deckGroup,
            const QString& otherDeckGroup,
            const UserSettingsPointer& pConfig);

    /// True when the Decks checkbox is on. If that key has never been
    /// written, either retired checkbox still counts.
    static bool autoHarmonizeOnLoadEnabled(const UserSettingsPointer& pConfig);

    /// True when the Decks Exact key checkbox is on. Unset stays off.
    static bool exactKeyMatchEnabled(const UserSettingsPointer& pConfig);

    /// Another primary deck wearing the key crown, or empty when none does.
    /// A higher crown value still outranks 1. Opposite left/right wins a tie.
    static QString keyLeaderGroup(const QString& excludeGroup);

    /// This deck is the key reference, so loading a track must not retune it.
    static bool isKeyMatchReference(const QString& deckGroup);

    /// Global preference or a latched sync_key, and this deck is not the
    /// key reference.
    static bool shouldHarmonizeOnLoad(
            const QString& deckGroup, const UserSettingsPointer& pConfig);

  signals:
    // Posted off the engine thread when this crowned deck's key changes.
    void leaderKeyFollowRequested();

  private slots:
    void slotSetEngineKey(double);
    void slotSetEngineKeyDistance(double);
    void slotFileKeyChanged(double);
    void slotPitchChanged(double);
    void slotPitchAdjustChanged(double);
    void slotRateChanged();
    // One-shot match for match_key. The Sync button uses slotSyncKeyButton.
    void slotSyncKey(double);
    void slotSyncKeyButton(double);
    void slotSyncKeyLeader(double);
    void slotResetKey(double);
    void slotLeaderKeyFollow();

  private:
    void setEngineKey(double key, double key_distance);
    bool syncKey(EngineBuffer* pOtherEngineBuffer);
    void updateKeyCOs(double fileKeyNumeric, double pitchOctaves);
    void updatePitch();
    void updatePitchAdjust();
    void updateRate();
    // Latched decks match this deck after its effective key changes.
    void queueOrApplyLeaderKeyToFollowers();
    void applyLeaderKeyToFollowers();
    // Own ControlObject::set does not emit valueChanged back here.
    // Store crown 0, then promote another synced deck when nobody else
    // is crowned.
    void dropKeyCrown();

    // ControlObjects that come from EngineBuffer
    parented_ptr<ControlProxy> m_pRateRatio;

    parented_ptr<ControlProxy> m_pVCRate;
    parented_ptr<ControlProxy> m_pVCEnabled;

    parented_ptr<ControlProxy> m_pKeylock;
    std::unique_ptr<ControlPotmeter> m_pPitch;
    std::unique_ptr<ControlPotmeter> m_pPitchAdjust;
    std::unique_ptr<ControlPushButton> m_pButtonSyncKey;
    // One-shot match for the note button. It must not write sync_key,
    // because that is the Sync button's light.
    std::unique_ptr<ControlPushButton> m_pButtonMatchKey;
    std::unique_ptr<ControlPushButton> m_pButtonSyncKeyLeader;
    std::unique_ptr<ControlPushButton> m_pButtonResetKey;
    std::unique_ptr<ControlPushButton> m_keylockMode;
    std::unique_ptr<ControlPushButton> m_keyunlockMode;

    // The current loaded file's detected key
    std::unique_ptr<ControlObject> m_pFileKey;

    // The current effective key of the engine
    std::unique_ptr<ControlObject> m_pEngineKey;
    std::unique_ptr<ControlPotmeter> m_pEngineKeyDistance;

    struct PitchTempoRatio m_pitchRateInfo;
    QAtomicInt m_updatePitchRequest;
    QAtomicInt m_updatePitchAdjustRequest;
    QAtomicInt m_updateRateRequest;
    // True while a crown click is latching sync_key, so that write does not
    // retune this deck or rewrite the crown.
    bool m_applyingCrownSync = false;
    // Coalesce engine-thread key changes into one GUI follow.
    QAtomicInt m_leaderFollowQueued;
    QAtomicInt m_leaderFollowValid;
    QAtomicInt m_lastFollowedKey;
    QAtomicInt m_lastFollowedDistanceMicro;
};
