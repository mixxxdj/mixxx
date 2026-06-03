#include "library/keyhighlightmanager.h"

#include <QString>
#include <algorithm>

#include "control/controlproxy.h"
#include "mixer/playermanager.h"
#include "moc_keyhighlightmanager.cpp"
#include "util/defs.h"

namespace mixxx {

KeyHighlightManager::KeyHighlightManager(QObject* pParent)
        : QObject(pParent),
          m_pNumDecks(new ControlProxy(
                  QStringLiteral("[App]"),
                  QStringLiteral("num_decks"),
                  this)) {
    m_table.fill(KeyUtils::KeyHighlightClass::None);
    m_pNumDecks->connectValueChanged(
            this, &KeyHighlightManager::slotNumDecksChanged);
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
        deck.pHighlight->connectValueChanged(
                this, [this] { recompute(); });
        deck.pKey->connectValueChanged(
                this, [this] { recompute(); });
        m_decks.append(deck);
    }
    recompute();
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
        if (enabled != deck.enabled || key != deck.key) {
            discreteChanged = true;
            deck.enabled = enabled;
            deck.key = key;
        }
    }
    if (!discreteChanged) {
        m_recomputing = false;
        return;
    }

    // Collect the reference keys of all enabled decks with a valid key.
    QList<mixxx::track::io::key::ChromaticKey> refKeys;
    for (const DeckState& deck : m_decks) {
        if (deck.enabled && deck.key != mixxx::track::io::key::INVALID) {
            refKeys.append(deck.key);
        }
    }

    const bool active = !refKeys.isEmpty();

    // Rebuild the table: for each possible track key, take the best (lowest
    // severity) class across all reference keys. KeyHighlightClass is ordered
    // so that the numerically smallest value is the most mixable.
    for (int i = 0; i < static_cast<int>(m_table.size()); ++i) {
        const auto trackKey =
                static_cast<mixxx::track::io::key::ChromaticKey>(i);
        KeyUtils::KeyHighlightClass best = KeyUtils::KeyHighlightClass::None;
        for (const auto refKey : refKeys) {
            const auto c = KeyUtils::classifyAgainst(trackKey, refKey);
            if (static_cast<int>(c) < static_cast<int>(best)) {
                best = c;
            }
        }
        m_table[i] = best;
    }

    m_active = active;
    m_recomputing = false;
    emit highlightChanged();
}

} // namespace mixxx
