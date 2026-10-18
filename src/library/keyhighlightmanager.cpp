#include "library/keyhighlightmanager.h"

#include <algorithm>
#include <cmath>

#include "control/controlproxy.h"
#include "control/controlpushbutton.h"
#include "mixer/playerinfo.h"
#include "mixer/playermanager.h"
#include "moc_keyhighlightmanager.cpp"
#include "track/keyutils.h"
#include "util/defs.h"
#include "util/fpclassify.h"
#include "util/math.h"

namespace mixxx {

KeyHighlightManager::KeyHighlightManager()
        : m_pNumDecks(std::make_unique<ControlProxy>(
                  QStringLiteral("[App]"), QStringLiteral("num_decks"))) {
    m_pNumDecks->connectValueChanged(this, &KeyHighlightManager::slotNumDecksChanged);
    connect(&PlayerInfo::instance(),
            &PlayerInfo::trackChanged,
            this,
            &KeyHighlightManager::slotTrackChanged);
    slotNumDecksChanged(m_pNumDecks->get());
}

KeyHighlightManager::~KeyHighlightManager() = default;

// static
KeyHighlightManager::KeyMatch KeyHighlightManager::classifyKey(
        mixxx::track::io::key::ChromaticKey trackKey,
        mixxx::track::io::key::ChromaticKey referenceKey) {
    // shortestStepsToCompatibleKey() returns 0 steps for an invalid key.
    if (trackKey == mixxx::track::io::key::INVALID ||
            referenceKey == mixxx::track::io::key::INVALID) {
        return KeyMatch::None;
    }
    const int steps = KeyUtils::shortestStepsToCompatibleKey(trackKey, referenceKey);
    switch (steps) {
    case 0:
        return KeyUtils::keyToOpenKeyNumber(trackKey) ==
                        KeyUtils::keyToOpenKeyNumber(referenceKey)
                ? KeyMatch::Perfect
                : KeyMatch::Neighbour;
    case 1:
    case -1:
        // A tritone away from a compatible key, both directions work.
        if (KeyUtils::shortestStepsToCompatibleKey(
                    KeyUtils::scaleKeySteps(trackKey, -steps), referenceKey) == 0) {
            return KeyMatch::ShiftEither;
        }
        return steps > 0 ? KeyMatch::ShiftUp : KeyMatch::ShiftDown;
    default:
        return KeyMatch::None;
    }
}

// static
KeyHighlightManager::BpmMatch KeyHighlightManager::classifyBpm(
        double trackBpm,
        double referenceBpm,
        double tolerancePercent) {
    if (!(trackBpm > 0.0) || !(referenceBpm > 0.0) || !(tolerancePercent > 0.0)) {
        return BpmMatch::None;
    }
    // Scale the difference by 100 instead of dividing the percentage, so that
    // window edges like 100 BPM +/- 6 % compare exactly.
    const auto withinTolerance = [trackBpm, tolerancePercent](double targetBpm) {
        return std::abs(trackBpm - targetBpm) * 100.0 <= targetBpm * tolerancePercent;
    };
    if (withinTolerance(referenceBpm)) {
        return BpmMatch::Direct;
    }
    if (withinTolerance(referenceBpm / 2.0) || withinTolerance(referenceBpm * 2.0)) {
        return BpmMatch::HalfDouble;
    }
    return BpmMatch::None;
}

KeyHighlightManager::BpmMatch KeyHighlightManager::bpmMatch(double trackBpm) const {
    if (!isBpmActive()) {
        return BpmMatch::None;
    }
    return classifyBpm(trackBpm, m_referenceBpm, m_bpmRangePercent);
}

void KeyHighlightManager::setBpmRange(double percent) {
    // The value comes from the user's config file, which may have been edited
    // by hand.
    percent = util_isfinite(percent)
            ? math_clamp(percent, 0.0, kBpmRangePercentMax)
            : 0.0;
    if (percent == m_bpmRangePercent) {
        return;
    }
    m_bpmRangePercent = percent;
    emit bpmHighlightChanged();
}

mixxx::track::io::key::ChromaticKey KeyHighlightManager::pitchedKey() const {
    if (m_referenceFileKey == mixxx::track::io::key::INVALID ||
            m_referenceKey == m_referenceFileKey) {
        return mixxx::track::io::key::INVALID;
    }
    return m_referenceKey;
}

TrackPointer KeyHighlightManager::referenceTrack() const {
    if (m_referenceDeck < 0) {
        return TrackPointer();
    }
    return PlayerInfo::instance().getTrackInfo(m_decks[m_referenceDeck].group);
}

void KeyHighlightManager::slotNumDecksChanged(double value) {
    // PlayerManager never removes decks.
    const int numDecks = std::min(static_cast<int>(value), kMaxNumberOfDecks);
    while (static_cast<int>(m_decks.size()) < numDecks) {
        addDeck();
    }
}

void KeyHighlightManager::addDeck() {
    const int deckIndex = static_cast<int>(m_decks.size());
    Deck deck;
    deck.group = PlayerManager::groupForDeck(deckIndex);
    deck.pHighlight = std::make_unique<ControlPushButton>(
            ConfigKey(deck.group, QStringLiteral("key_highlight")));
    deck.pHighlight->setButtonMode(mixxx::control::ButtonMode::Toggle);
    connect(deck.pHighlight.get(),
            &ControlObject::valueChanged,
            this,
            [this, deckIndex](double value) {
                slotHighlightToggled(deckIndex, value);
            });

    const auto updateKey = [this, deckIndex] {
        if (deckIndex == m_referenceDeck) {
            updateReferenceKey();
        }
    };
    deck.pKey = std::make_unique<ControlProxy>(deck.group, QStringLiteral("key"));
    deck.pKey->connectValueChanged(this, updateKey);
    deck.pFileKey = std::make_unique<ControlProxy>(deck.group, QStringLiteral("file_key"));
    deck.pFileKey->connectValueChanged(this, updateKey);
    // The BPM the deck displays, updated at GUI rate rather than on every
    // movement of the pitch fader.
    deck.pVisualBpm = std::make_unique<ControlProxy>(deck.group, QStringLiteral("visual_bpm"));
    deck.pVisualBpm->connectValueChanged(this, [this, deckIndex] {
        if (deckIndex == m_referenceDeck) {
            updateReferenceBpm();
        }
    });
    m_decks.push_back(std::move(deck));
}

void KeyHighlightManager::slotHighlightToggled(int deckIndex, double value) {
    if (value > 0.0) {
        // Our own writes don't loop back here.
        for (int i = 0; i < static_cast<int>(m_decks.size()); ++i) {
            if (i != deckIndex && m_decks[i].pHighlight->toBool()) {
                m_decks[i].pHighlight->set(0.0);
            }
        }
        setReferenceDeck(deckIndex);
    } else if (deckIndex == m_referenceDeck) {
        setReferenceDeck(-1);
    }
}

void KeyHighlightManager::slotTrackChanged(const QString& group,
        TrackPointer pNewTrack,
        TrackPointer pOldTrack) {
    Q_UNUSED(pNewTrack);
    Q_UNUSED(pOldTrack);
    if (m_referenceDeck >= 0 && group == m_decks[m_referenceDeck].group) {
        emit referenceTrackChanged();
    }
}

void KeyHighlightManager::setReferenceDeck(int deckIndex) {
    if (deckIndex == m_referenceDeck) {
        return;
    }
    m_referenceDeck = deckIndex;
    updateReferenceKey();
    updateReferenceBpm();
    emit referenceTrackChanged();
}

void KeyHighlightManager::updateReferenceKey() {
    auto key = mixxx::track::io::key::INVALID;
    auto fileKey = mixxx::track::io::key::INVALID;
    if (m_referenceDeck >= 0) {
        const Deck& deck = m_decks[m_referenceDeck];
        key = KeyUtils::keyFromNumericValue(deck.pKey->get());
        fileKey = KeyUtils::keyFromNumericValue(deck.pFileKey->get());
    }
    if (key == m_referenceKey && fileKey == m_referenceFileKey) {
        return;
    }
    m_referenceKey = key;
    m_referenceFileKey = fileKey;
    emit keyHighlightChanged();
}

void KeyHighlightManager::updateReferenceBpm() {
    const double bpm = m_referenceDeck >= 0
            ? m_decks[m_referenceDeck].pVisualBpm->get()
            : 0.0;
    if (bpm == m_referenceBpm) {
        return;
    }
    m_referenceBpm = bpm;
    emit bpmHighlightChanged();
}

} // namespace mixxx
