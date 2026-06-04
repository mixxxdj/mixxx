#pragma once

#include <QList>
#include <QObject>
#include <QString>
#include <array>

#include "proto/keys.pb.h"
#include "track/keyutils.h"
#include "track/track_decl.h"
#include "track/trackid.h"

class ControlProxy;

namespace mixxx {

/// Coordinates the per-deck "harmonic key highlighter" for the library track
/// table. It watches `[App],num_decks` and, for each deck, the
/// `[ChannelN],key_highlight` toggle, the `[ChannelN],key` engine (playing) key
/// and the `[ChannelN],file_key` stored key. From the set of enabled decks it
/// precomputes a lookup table mapping every ChromaticKey to a
/// KeyUtils::KeyHighlightClass, so the model can colour each row with an O(1)
/// lookup. It also tracks each deck's loaded track so the model can show the
/// pitched playing key beside the stored key on that track's row.
///
/// Recomputation is debounced on the *discrete* key: the engine key control
/// updates continuously as the pitch changes, but the classification only
/// depends on the rounded ChromaticKey, so the table is only rebuilt (and
/// highlightChanged() only emitted) when a deck's discrete key or enable flag
/// actually changes.
class KeyHighlightManager : public QObject {
    Q_OBJECT
  public:
    explicit KeyHighlightManager(QObject* pParent = nullptr);
    ~KeyHighlightManager() override = default;

    /// Whether at least one deck has the highlighter enabled and a valid key,
    /// i.e. whether the library should currently be tinted at all.
    bool isActive() const {
        return m_active;
    }

    /// The highlight class for a track key given the currently active decks.
    /// O(1) table lookup. Returns None for INVALID keys or when inactive.
    KeyUtils::KeyHighlightClass classOf(
            mixxx::track::io::key::ChromaticKey trackKey) const;

    /// For a track key classified Yellow, the direction it must be transposed by
    /// one semitone to become compatible. O(1) table lookup; the stored value
    /// comes from the same reference key that produced classOf()'s class, so the
    /// hint always matches the colour. Returns None for non-Yellow keys or when
    /// inactive.
    KeyUtils::YellowShift directionOf(
            mixxx::track::io::key::ChromaticKey trackKey) const;

    /// The currently playing (pitch-shifted) key to display in parentheses next
    /// to a track's stored key, or INVALID when no suffix should be shown.
    /// Returns a valid key only when the highlighter is active, an enabled deck
    /// has this track loaded, and that deck's discrete playing key differs from
    /// the track's stored (file) key - i.e. the deck is pitched far enough to
    /// change the discrete key. Mutual exclusion means at most one enabled deck
    /// today, so the first match wins.
    mixxx::track::io::key::ChromaticKey playingKeyForTrack(
            TrackId trackId) const;

  signals:
    /// Emitted whenever the classification table or the active state changes,
    /// so observers (the track table models) can repaint.
    void highlightChanged();

  private slots:
    void slotNumDecksChanged(double dNumDecks);
    /// Tracks which track is loaded on each deck so playingKeyForTrack() can
    /// scope the "(playing key)" suffix to the loaded deck's row(s).
    void slotTrackChanged(
            const QString& group,
            TrackPointer pNewTrack,
            TrackPointer pOldTrack);

  private:
    /// Per-deck state, snapshotted to debounce continuous key changes. The
    /// ControlProxy pointers are owned by this QObject (Qt parent ownership),
    /// like VinylControlManager's deck proxies.
    struct DeckState {
        ControlProxy* pHighlight = nullptr;
        ControlProxy* pKey = nullptr;
        ControlProxy* pFileKey = nullptr;
        bool enabled = false;
        mixxx::track::io::key::ChromaticKey key =
                mixxx::track::io::key::INVALID;
        // The loaded track's stored/analysed key, snapshotted alongside `key`.
        // Compared against `key` to detect a pitch shift worth displaying.
        mixxx::track::io::key::ChromaticKey fileKey =
                mixxx::track::io::key::INVALID;
        // The track currently loaded on this deck (invalid if none), updated by
        // slotTrackChanged(). Used to map a library row to a pitched deck.
        TrackId loadedTrackId;
    };

    /// Re-reads every deck's enable flag and discrete key. If anything discrete
    /// changed, rebuilds the table and emits highlightChanged().
    void recompute();

    ControlProxy* m_pNumDecks;
    QList<DeckState> m_decks;
    // The current deck count. m_decks only ever grows (proxies persist), but
    // num_decks can shrink; decks at index >= m_numDecks are inactive and must
    // not contribute reference keys. See recompute().
    int m_numDecks = 0;

    // Lookup table indexed by ChromaticKey (0 == INVALID .. 24). Entry 0 is
    // always None.
    std::array<KeyUtils::KeyHighlightClass,
            mixxx::track::io::key::ChromaticKey_ARRAYSIZE>
            m_table;
    // Parallel table of transpose directions, only meaningful for keys whose
    // m_table entry is Yellow; None elsewhere. Filled from the reference key
    // that won the best-class comparison in recompute().
    std::array<KeyUtils::YellowShift,
            mixxx::track::io::key::ChromaticKey_ARRAYSIZE>
            m_directionTable;
    bool m_active = false;
    // Re-entrancy guard for recompute(): enforcing single-active-deck writes to
    // other decks' key_highlight controls, which loop back into recompute().
    bool m_recomputing = false;
};

} // namespace mixxx
