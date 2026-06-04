#include "library/keyhighlightmanager.h"

#include <QString>
#include <algorithm>
#include <utility>

#include "control/controlproxy.h"
#include "mixer/playerinfo.h"
#include "mixer/playermanager.h"
#include "moc_keyhighlightmanager.cpp"
#include "track/track.h"
#include "util/defs.h"

namespace mixxx {

KeyHighlightManager::KeyHighlightManager(QObject* pParent)
        : QObject(pParent),
          m_pNumDecks(new ControlProxy(
                  QStringLiteral("[App]"),
                  QStringLiteral("num_decks"),
                  this)) {
    m_table.fill(KeyUtils::KeyHighlightClass::None);
    m_directionTable.fill(KeyUtils::YellowShift::None);
    m_pNumDecks->connectValueChanged(
            this, &KeyHighlightManager::slotNumDecksChanged);
    // Follow track load/eject on every deck so the playing-key suffix can be
    // scoped to the loaded deck's row(s).
    connect(&PlayerInfo::instance(),
            &PlayerInfo::trackChanged,
            this,
            &KeyHighlightManager::slotTrackChanged);
    slotNumDecksChanged(m_pNumDecks->get());
}

KeyUtils::KeyHighlightClass KeyHighlightManager::classOf(
        mixxx::track::io::key::ChromaticKey trackKey) const {
    if (!m_active) {
        return KeyUtils::KeyHighlightClass::None;
    }
    const int index = static_cast<int>(trackKey);
    if (index < 0 || index >= static_cast<int>(m_table.size())) {
        return KeyUtils::KeyHighlightClass::None;
    }
    return m_table[index];
}

KeyUtils::YellowShift KeyHighlightManager::directionOf(
        mixxx::track::io::key::ChromaticKey trackKey) const {
    if (!m_active) {
        return KeyUtils::YellowShift::None;
    }
    const int index = static_cast<int>(trackKey);
    if (index < 0 || index >= static_cast<int>(m_directionTable.size())) {
        return KeyUtils::YellowShift::None;
    }
    return m_directionTable[index];
}

mixxx::track::io::key::ChromaticKey KeyHighlightManager::playingKeyForTrack(
        TrackId trackId) const {
    using mixxx::track::io::key::INVALID;
    // The suffix is part of the highlighter feature: only show it while a deck
    // is actively highlighting, and only for a track loaded on an enabled deck.
    if (!m_active || !trackId.isValid()) {
        return INVALID;
    }
    for (const DeckState& deck : std::as_const(m_decks)) {
        if (!deck.enabled || deck.loadedTrackId != trackId) {
            continue;
        }
        // Only a discrete pitch shift (playing key differs from the stored key)
        // warrants the parenthetical; an unpitched deck shows just the key.
        if (deck.key != INVALID && deck.fileKey != INVALID &&
                deck.key != deck.fileKey) {
            return deck.key;
        }
        // Mutual exclusion: at most one enabled deck, and it holds this track.
        return INVALID;
    }
    return INVALID;
}

void KeyHighlightManager::slotNumDecksChanged(double dNumDecks) {
    int numDecks = static_cast<int>(dNumDecks);
    if (numDecks > kMaxNumberOfDecks) {
        numDecks = kMaxNumberOfDecks;
    }
    m_numDecks = numDecks;
    // Only ever grow the set of watched proxies; mirrors VinylControlManager.
    // num_decks can also shrink, in which case the surplus DeckStates remain in
    // m_decks but are excluded from classification by the index < m_numDecks
    // guards in recompute().
    for (int i = m_decks.size(); i < numDecks; ++i) {
        const QString group = PlayerManager::groupForDeck(i);
        DeckState deck;
        deck.pHighlight = new ControlProxy(
                group, QStringLiteral("key_highlight"), this);
        deck.pKey = new ControlProxy(
                group, QStringLiteral("key"), this);
        deck.pFileKey = new ControlProxy(
                group, QStringLiteral("file_key"), this);
        deck.pHighlight->connectValueChanged(
                this, [this] { recompute(); });
        deck.pKey->connectValueChanged(
                this, [this] { recompute(); });
        // file_key changes on (re)analysis; keep the stored-key snapshot fresh.
        deck.pFileKey->connectValueChanged(
                this, [this] { recompute(); });
        m_decks.append(deck);
    }
    recompute();
}

void KeyHighlightManager::slotTrackChanged(
        const QString& group,
        TrackPointer pNewTrack,
        TrackPointer pOldTrack) {
    Q_UNUSED(pOldTrack);
    int deckNumber = -1;
    if (!PlayerManager::isDeckGroup(group, &deckNumber)) {
        return;
    }
    // isDeckGroup() yields a 1-based deck number; DeckState is 0-indexed.
    const int deckIndex = deckNumber - 1;
    if (deckIndex < 0 || deckIndex >= m_decks.size()) {
        return;
    }
    const TrackId newTrackId = pNewTrack ? pNewTrack->getId() : TrackId();
    if (m_decks[deckIndex].loadedTrackId == newTrackId) {
        return;
    }
    m_decks[deckIndex].loadedTrackId = newTrackId;
    // The set of pitched/loaded rows changed; refresh so the suffix follows.
    emit highlightChanged();
}

void KeyHighlightManager::recompute() {
    // Guard against re-entrancy: enforceSingleActiveDeck() below clears the
    // key_highlight control on other decks, and each such write loops back here
    // via the connected proxies. We only need the outermost pass to rebuild.
    if (m_recomputing) {
        return;
    }
    m_recomputing = true;

    // m_decks only grows, but num_decks can shrink; only decks below the current
    // count participate. Surplus decks are forced inactive in the snapshot below.
    const int activeDecks =
            std::min(static_cast<int>(m_decks.size()), m_numDecks);

    // The highlighter is mutually exclusive across decks: enabling one deck's
    // highlight turns any other deck's off. We enforce this by writing 0 to the
    // losing decks' key_highlight controls (rather than only suppressing them in
    // our internal state) so a mapped controller LED / skin button reflects the
    // true state. Detect a deck that has *just* been enabled (off in our
    // snapshot, on now) and treat it as the winner.
    int newlyEnabled = -1;
    for (int i = 0; i < activeDecks; ++i) {
        if (!m_decks[i].enabled && m_decks[i].pHighlight->toBool()) {
            newlyEnabled = i;
            // Last writer wins if (improbably) two flip on in the same cycle.
        }
    }
    if (newlyEnabled >= 0) {
        for (int i = 0; i < activeDecks; ++i) {
            if (i != newlyEnabled && m_decks[i].pHighlight->toBool()) {
                m_decks[i].pHighlight->set(0.0);
            }
        }
    }

    // Re-read every deck's enable flag and discrete key, and detect whether
    // anything discrete actually changed (debounce against continuous pitch).
    // Decks at or beyond the current num_decks are treated as disabled so a
    // shrink stops them from contributing.
    bool discreteChanged = false;
    for (int i = 0; i < m_decks.size(); ++i) {
        DeckState& deck = m_decks[i];
        const bool enabled = i < activeDecks && deck.pHighlight->toBool();
        const auto key = KeyUtils::keyFromNumericValue(deck.pKey->get());
        const auto fileKey =
                KeyUtils::keyFromNumericValue(deck.pFileKey->get());
        if (enabled != deck.enabled || key != deck.key ||
                fileKey != deck.fileKey) {
            discreteChanged = true;
            deck.enabled = enabled;
            deck.key = key;
            deck.fileKey = fileKey;
        }
    }
    if (!discreteChanged) {
        m_recomputing = false;
        return;
    }

    // Collect the reference keys of all enabled decks with a valid key.
    QList<mixxx::track::io::key::ChromaticKey> refKeys;
    for (const DeckState& deck : std::as_const(m_decks)) {
        if (deck.enabled && deck.key != mixxx::track::io::key::INVALID) {
            refKeys.append(deck.key);
        }
    }

    const bool active = !refKeys.isEmpty();

    // Rebuild the table: for each possible track key, take the best (lowest
    // severity) class across all reference keys. KeyHighlightClass is ordered
    // so that the numerically smallest value is the most mixable. We also track
    // which reference key produced that best class, so the transpose direction
    // we store corresponds to the same deck whose colour the track gets (the
    // direction is meaningless otherwise). Today mutual exclusion guarantees a
    // single reference key, but capturing the winner keeps this correct if that
    // is ever relaxed.
    for (int i = 0; i < static_cast<int>(m_table.size()); ++i) {
        const auto trackKey =
                static_cast<mixxx::track::io::key::ChromaticKey>(i);
        KeyUtils::KeyHighlightClass best = KeyUtils::KeyHighlightClass::None;
        auto winningRefKey = mixxx::track::io::key::INVALID;
        for (const auto refKey : refKeys) {
            const auto c = KeyUtils::classifyAgainst(trackKey, refKey);
            if (static_cast<int>(c) < static_cast<int>(best)) {
                best = c;
                winningRefKey = refKey;
            }
        }
        m_table[i] = best;
        // Only Yellow tracks carry a direction; force None for everything else
        // so a stale arrow never leaks onto a green or red cell.
        m_directionTable[i] = (best == KeyUtils::KeyHighlightClass::Yellow)
                ? KeyUtils::yellowShiftDirection(trackKey, winningRefKey)
                : KeyUtils::YellowShift::None;
    }

    m_active = active;
    m_recomputing = false;
    emit highlightChanged();
}

} // namespace mixxx
