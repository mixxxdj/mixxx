#pragma once

#include <QObject>
#include <QString>
#include <memory>
#include <vector>

#include "proto/keys.pb.h"
#include "track/track_decl.h"
#include "util/singleton.h"

class ControlProxy;
class ControlPushButton;

namespace mixxx {

/// Owns the per-deck `[ChannelN],key_highlight` toggles. While one is on, that
/// deck's playing key and tempo are the reference for highlighting compatible
/// tracks in the library. Enabling one deck turns the others off.
class KeyHighlightManager : public QObject, public Singleton<KeyHighlightManager> {
    Q_OBJECT
  public:
    /// How a track's key relates to the reference deck's playing key.
    enum class KeyMatch {
        None,
        /// The same key or its relative major/minor.
        Perfect,
        /// A fifth up or down, in either mode.
        Neighbour,
        /// Compatible once the track is pitched up a semitone.
        ShiftUp,
        /// Compatible once the track is pitched down a semitone.
        ShiftDown,
        /// Compatible once the track is pitched a semitone either way.
        ShiftEither,
    };

    /// How a track's BPM relates to the reference deck's BPM.
    enum class BpmMatch {
        None,
        /// Within the tolerance of the reference BPM.
        Direct,
        /// Within the tolerance of half or double the reference BPM.
        HalfDouble,
    };

    /// The default BPM tolerance in percent. It equals the default of the
    /// library search's fuzzy BPM range, but the two are separate settings.
    static constexpr double kBpmRangePercentDefault = 6.0;
    /// From about 33 % the window would overlap the half/double ones.
    static constexpr double kBpmRangePercentMax = 20.0;

    /// Whether a deck is highlighting and its playing key is known.
    bool isKeyActive() const {
        return m_referenceKey != mixxx::track::io::key::INVALID;
    }

    /// Whether a deck is highlighting, its BPM is known and the tolerance is
    /// not zero. A deck without a key still highlights by BPM.
    bool isBpmActive() const {
        return m_referenceBpm > 0.0 && m_bpmRangePercent > 0.0;
    }

    /// Classifies trackKey against referenceKey. The compatible keys are those
    /// of KeyUtils::getCompatibleKeys(), and the shifts are the ones `sync_key`
    /// would apply. Returns None if either key is INVALID.
    static KeyMatch classifyKey(
            mixxx::track::io::key::ChromaticKey trackKey,
            mixxx::track::io::key::ChromaticKey referenceKey);

    /// Classifies trackBpm against referenceBpm with a +/- tolerancePercent
    /// window around the reference and around half and double of it. Direct
    /// wins over HalfDouble. Returns None if any input is not positive.
    static BpmMatch classifyBpm(
            double trackBpm,
            double referenceBpm,
            double tolerancePercent);

    /// The match of a track key against the reference deck's playing key.
    KeyMatch keyMatch(mixxx::track::io::key::ChromaticKey trackKey) const {
        return classifyKey(trackKey, m_referenceKey);
    }

    /// The match of a track BPM against the reference deck's BPM.
    BpmMatch bpmMatch(double trackBpm) const;

    /// Sets the BPM tolerance in percent, clamped to [0, kBpmRangePercentMax].
    /// 0 or a non-finite value turns the BPM highlight off.
    void setBpmRange(double percent);

    /// The reference deck's playing key if the pitch moved it away from the
    /// loaded track's key, otherwise INVALID.
    mixxx::track::io::key::ChromaticKey pitchedKey() const;

    /// The track loaded on the reference deck, or nullptr.
    TrackPointer referenceTrack() const;

  signals:
    /// The reference key or the pitched key changed.
    void keyHighlightChanged();
    /// The reference BPM or the tolerance changed.
    void bpmHighlightChanged();
    /// The reference deck changed, or the track loaded on it.
    void referenceTrackChanged();

  protected:
    KeyHighlightManager();
    ~KeyHighlightManager() override;
    friend class Singleton<KeyHighlightManager>;

  private slots:
    void slotNumDecksChanged(double value);
    void slotHighlightToggled(int deckIndex, double value);
    void slotTrackChanged(const QString& group,
            TrackPointer pNewTrack,
            TrackPointer pOldTrack);

  private:
    struct Deck {
        QString group;
        std::unique_ptr<ControlPushButton> pHighlight;
        std::unique_ptr<ControlProxy> pKey;
        std::unique_ptr<ControlProxy> pFileKey;
        std::unique_ptr<ControlProxy> pVisualBpm;
    };

    void addDeck();
    void setReferenceDeck(int deckIndex);
    void updateReferenceKey();
    void updateReferenceBpm();

    std::unique_ptr<ControlProxy> m_pNumDecks;
    std::vector<Deck> m_decks;

    // The deck whose highlight is on, or -1.
    int m_referenceDeck = -1;
    mixxx::track::io::key::ChromaticKey m_referenceKey =
            mixxx::track::io::key::INVALID;
    mixxx::track::io::key::ChromaticKey m_referenceFileKey =
            mixxx::track::io::key::INVALID;
    double m_referenceBpm = 0.0;
    double m_bpmRangePercent = kBpmRangePercentDefault;
};

} // namespace mixxx
