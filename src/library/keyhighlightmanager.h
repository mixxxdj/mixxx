#pragma once

#include <QList>
#include <QObject>
#include <array>

#include "proto/keys.pb.h"
#include "track/keyutils.h"

class ControlProxy;

namespace mixxx {

/// Coordinates the per-deck "harmonic key highlighter" for the library track
/// table. It watches `[App],num_decks` and, for each deck, the
/// `[ChannelN],key_highlight` toggle and the `[ChannelN],key` engine key. From
/// the set of enabled decks it precomputes a lookup table mapping every
/// ChromaticKey to a KeyUtils::KeyHighlightClass, so the model can colour each
/// row with an O(1) lookup.
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

  signals:
    /// Emitted whenever the classification table or the active state changes,
    /// so observers (the track table models) can repaint.
    void highlightChanged();

  private slots:
    void slotNumDecksChanged(double dNumDecks);

  private:
    /// Per-deck state, snapshotted to debounce continuous key changes. The
    /// ControlProxy pointers are owned by this QObject (Qt parent ownership),
    /// like VinylControlManager's deck proxies.
    struct DeckState {
        ControlProxy* pHighlight = nullptr;
        ControlProxy* pKey = nullptr;
        bool enabled = false;
        mixxx::track::io::key::ChromaticKey key =
                mixxx::track::io::key::INVALID;
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
    bool m_active = false;
    // Re-entrancy guard for recompute(): enforcing single-active-deck writes to
    // other decks' key_highlight controls, which loop back into recompute().
    bool m_recomputing = false;
};

} // namespace mixxx
