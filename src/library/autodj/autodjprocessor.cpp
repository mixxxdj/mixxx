#include "library/autodj/autodjprocessor.h"

#include "engine/channels/enginedeck.h"
#include "library/autodj/autodjconstants.h"
#include "mixer/basetrackplayer.h"
#include "mixer/playermanager.h"
#include "moc_autodjprocessor.cpp"
#include "track/track.h"
#include "util/math.h"

namespace {
const QString kConfigKey = QStringLiteral("[Auto DJ]");
const char* kTransitionPreferenceName = "Transition";
const char* kTransitionModePreferenceName = "TransitionMode";
constexpr double kTransitionPreferenceDefault = 10.0;
constexpr double kKeepPosition = AutoDJConstants::kKeepPosition;
constexpr double kSkipToNextTrack = AutoDJConstants::kSkipToNextTrack;

// A track needs to be longer than two callbacks to not stop AutoDJ
constexpr double kMinimumTrackDurationSec = 0.2;

constexpr bool sDebug = false;
} // anonymous namespace

AutoDJProcessor::AutoDJProcessor(
        QObject* pParent,
        UserSettingsPointer pConfig,
        PlayerManagerInterface* pPlayerManager,
        TrackCollectionManager* pTrackCollectionManager,
        int iAutoDJPlaylistId)
        : QObject(pParent),
          m_pConfig(pConfig),
          m_pAutoDJTableModel(nullptr),
          m_eState(ADJ_DISABLED),
          m_transitionProgress(0.0),
          m_transitionTime(kTransitionPreferenceDefault),
          m_pPlayerManager(pPlayerManager),
          m_coCrossfader(QStringLiteral("[Master]"), QStringLiteral("crossfader")),
          m_coCrossfaderReverse(QStringLiteral("[Mixer Profile]"), QStringLiteral("xFaderReverse")),
          m_shufflePlaylist(ConfigKey(kConfigKey, QStringLiteral("shuffle_playlist"))),
          m_skipNext(ConfigKey(kConfigKey, QStringLiteral("skip_next"))),
          m_addRandomTrack(ConfigKey(kConfigKey, QStringLiteral("add_random_track"))),
          m_fadeNow(ConfigKey(kConfigKey, QStringLiteral("fade_now"))),
          m_enabledAutoDJ(ConfigKey(kConfigKey, QStringLiteral("enabled"))) {
    m_pAutoDJTableModel = make_parented<PlaylistTableModel>(
            this, pTrackCollectionManager, "mixxx.db.model.autodj");
    m_pAutoDJTableModel->selectPlaylist(iAutoDJPlaylistId);
    m_pAutoDJTableModel->select();

    connect(&m_shufflePlaylist,
            &ControlPushButton::valueChanged,
            this,
            &AutoDJProcessor::controlShuffle);
    connect(&m_skipNext, &ControlObject::valueChanged, this, &AutoDJProcessor::controlSkipNext);
    connect(&m_addRandomTrack,
            &ControlObject::valueChanged,
            this,
            &AutoDJProcessor::controlAddRandomTrack);
    connect(&m_fadeNow, &ControlObject::valueChanged, this, &AutoDJProcessor::controlFadeNow);
    m_enabledAutoDJ.setButtonMode(mixxx::control::ButtonMode::Toggle);
    m_enabledAutoDJ.connectValueChangeRequest(this,
            &AutoDJProcessor::controlEnableChangeRequest);

    connect(pPlayerManager,
            &PlayerManagerInterface::numberOfDecksChanged,
            this,
            &AutoDJProcessor::slotNumberOfDecksChanged);
    slotNumberOfDecksChanged(pPlayerManager->numberOfDecks());

    QString str_autoDjTransition = m_pConfig->getValueString(
            ConfigKey(kConfigKey, kTransitionPreferenceName));
    if (!str_autoDjTransition.isEmpty()) {
        m_transitionTime = str_autoDjTransition.toDouble();
    }

    m_transitionMode = m_pConfig->getValue(
            ConfigKey(kConfigKey, kTransitionModePreferenceName), TransitionMode::FullIntroOutro);
}

void AutoDJProcessor::slotNumberOfDecksChanged(int decks) {
    m_decks.reserve(decks);
    // Add more decks if we have not all yet.
    // Mixxx does not support reducing the number of deck
    for (int i = static_cast<int>(m_decks.size()); i < decks; ++i) {
        BaseTrackPlayer* pPlayer = m_pPlayerManager->getDeckBase(i);
        // Shouldn't be possible.
        VERIFY_OR_DEBUG_ASSERT(pPlayer) {
            return;
        }
        m_decks.emplace_back(std::make_unique<DeckAttributes>(i, pPlayer));
    }
}

double AutoDJProcessor::getCrossfader() const {
    if (m_coCrossfaderReverse.toBool()) {
        return m_coCrossfader.get() * -1.0;
    }
    return m_coCrossfader.get();
}

void AutoDJProcessor::setCrossfader(double value) {
    if (m_coCrossfaderReverse.toBool()) {
        value *= -1.0;
    }
    m_coCrossfader.set(value);
}

AutoDJProcessor::AutoDJError AutoDJProcessor::shufflePlaylist(
        const QModelIndexList& selectedIndices) {
    QModelIndex exclude;
    if (m_eState != ADJ_DISABLED) {
        exclude = m_pAutoDJTableModel->index(0, 0);
    }
    m_pAutoDJTableModel->shuffleTracks(selectedIndices, exclude);
    return ADJ_OK;
}

void AutoDJProcessor::fadeNow() {
    if (m_eState != ADJ_IDLE) {
        // we cannot fade if AutoDj is disabled or already fading
        return;
    }

    double crossfader = getCrossfader();
    DeckAttributes* pLeftDeck = getLeftDeck();
    DeckAttributes* pRightDeck = getRightDeck();
    if (!pLeftDeck || !pRightDeck) {
        // User has changed the orientation, disable Auto DJ
        toggleAutoDJ(false);
        emit autoDJError(ADJ_NOT_TWO_DECKS);
        return;
    }

    DeckAttributes* pFromDeck;
    DeckAttributes* pToDeck;

    if (pLeftDeck->isPlaying() &&
            (!pRightDeck->isPlaying() || crossfader < 0.0)) {
        pFromDeck = pLeftDeck;
        pToDeck = pRightDeck;
    } else if (pRightDeck->isPlaying()) {
        pFromDeck = pRightDeck;
        pToDeck = pLeftDeck;
    } else {
        // Neither deck is playing. Fading now makes no sense.
        return;
    }

    pFromDeck->setRepeat(false);
    pFromDeck->isFromDeck = true;
    pToDeck->isFromDeck = false;

    const double fromDeckEndSecond = getEndSecond(*pFromDeck);
    const double toDeckEndSecond = getEndSecond(*pToDeck);
    // Since the end position is measured in seconds from 0:00 it is also
    // the track duration. Use this alias for better readability.
    const double fromDeckDuration = fromDeckEndSecond;
    const double toDeckDuration = toDeckEndSecond;
    if (toDeckDuration < kMinimumTrackDurationSec) {
        // Deck is empty or track too short, disable AutoDJ
        // This happens only if the user has changed deck orientation to such deck.
        toggleAutoDJ(false);
        emit autoDJError(ADJ_NOT_TWO_DECKS);
        return;
    }

    // playPosition() is in the range of 0..1
    const double fromDeckCurrentSecond = fromDeckDuration * pFromDeck->playPosition();
    const double toDeckCurrentSecond = toDeckDuration * pToDeck->playPosition();

    if (toDeckDuration - toDeckCurrentSecond < kMinimumTrackDurationSec) {
        // Remaining Track time is too short, user has has seeked near the end
        // Re-cue the track
        pToDeck->setPlayPosition(pToDeck->startPos);
    }

    pFromDeck->fadeBeginPos = fromDeckCurrentSecond;
    // Do not seek to a calculated start point; start the to deck from wherever
    // it is if the user has seeked since loading the track.
    pToDeck->startPos = toDeckCurrentSecond;

    // If the user presses "Fade now", assume they want to fade *now*, not later.
    // So if the spinbox time is negative, do not insert silence.
    double spinboxTime = fabs(m_transitionTime);

    double fadeTime;
    if (m_transitionMode == TransitionMode::FullIntroOutro ||
            m_transitionMode == TransitionMode::FadeAtOutroStart) {
        // Use the intro length as the transition time. If the user has seeked
        // away from the intro start since the track was loaded, start from
        // there and do not seek back to the intro start. If they have seeked
        // past the introEnd or the introEnd is not marked, fall back to the
        // spinbox time.
        double outroEnd = getOutroEndSecond(*pFromDeck);
        double introEnd = getIntroEndSecond(*pToDeck);
        double introStart = getIntroStartSecond(*pToDeck);
        double timeUntilOutroEnd = outroEnd - fromDeckCurrentSecond;

        // IntroStart ends up being equal to introEnd when pToDeck is
        // paused and its introEnd marker is not set. getIntroEndSecond returns
        // introStart thus the two end up having equal values
        if (toDeckCurrentSecond >= introStart &&
                toDeckCurrentSecond <= introEnd &&
                introStart != introEnd) {
            double timeUntilIntroEnd = introEnd - toDeckCurrentSecond;
            // The fade must end by the outro end at the latest.
            fadeTime = math_min(timeUntilIntroEnd, timeUntilOutroEnd);
        } else {
            // If this is true, the fade should have already been started
            // so the user should not have been able to press the Fade button.
            VERIFY_OR_DEBUG_ASSERT(timeUntilOutroEnd > 0) {
                timeUntilOutroEnd = 0;
            }
            fadeTime = math_min(spinboxTime, timeUntilOutroEnd);
        }
    } else {
        fadeTime = spinboxTime;
    }

    fadeTime = math_min(fadeTime, fromDeckEndSecond - fromDeckCurrentSecond);
    fadeTime = math_min(fadeTime,
            (toDeckEndSecond - toDeckCurrentSecond) / 2); // for fade in and out

    pFromDeck->fadeEndPos = fromDeckCurrentSecond + fadeTime;

    // These are expected to be a fraction of the track length.
    pFromDeck->fadeBeginPos /= fromDeckDuration;
    pFromDeck->fadeEndPos /= fromDeckDuration;
    pToDeck->startPos /= toDeckDuration;

    VERIFY_OR_DEBUG_ASSERT(pFromDeck->fadeBeginPos <= 1) {
        pFromDeck->fadeBeginPos = 1;
    }
}

AutoDJProcessor::AutoDJError AutoDJProcessor::skipNext() {
    if (m_eState == ADJ_DISABLED) {
        emit autoDJError(ADJ_IS_INACTIVE);
        return ADJ_IS_INACTIVE;
    }
    // Load the next song from the queue.
    DeckAttributes* pLeftDeck = getLeftDeck();
    DeckAttributes* pRightDeck = getRightDeck();
    if (!pLeftDeck || !pRightDeck) {
        // User has changed the orientation, disable Auto DJ
        toggleAutoDJ(false);
        emit autoDJError(ADJ_NOT_TWO_DECKS);
        return ADJ_NOT_TWO_DECKS;
    }

    if (!pLeftDeck->isPlaying()) {
        removeLoadedTrackFromTopOfQueue(*pLeftDeck);
        loadNextTrackFromQueue(*pLeftDeck);
    } else if (!pRightDeck->isPlaying()) {
        removeLoadedTrackFromTopOfQueue(*pRightDeck);
        loadNextTrackFromQueue(*pRightDeck);
    } else {
        // If both decks are playing remove next track in playlist
        TrackId nextId = m_pAutoDJTableModel->getTrackId(m_pAutoDJTableModel->index(0, 0));
        TrackId leftId = pLeftDeck->getLoadedTrack()->getId();
        TrackId rightId = pRightDeck->getLoadedTrack()->getId();
        if (nextId == leftId || nextId == rightId) {
        // One of the playing tracks is still on top of playlist, remove second item
            m_pAutoDJTableModel->removeTrack(m_pAutoDJTableModel->index(1, 0));
        } else {
            m_pAutoDJTableModel->removeTrack(m_pAutoDJTableModel->index(0, 0));
        }
        maybeFillRandomTracks();
    }
    return ADJ_OK;
}

AutoDJProcessor::AutoDJError AutoDJProcessor::toggleAutoDJ(bool enable) {
    if (enable) { // Enable Auto DJ
        DeckAttributes* pLeftDeck = getLeftDeck();
        DeckAttributes* pRightDeck = getRightDeck();
        if (!pLeftDeck || !pRightDeck) {
            // Keep the current state.
            emitAutoDJStateChanged(m_eState);
            emit autoDJError(ADJ_NOT_TWO_DECKS);
            return ADJ_NOT_TWO_DECKS;
        }

        bool leftDeckPlaying = pLeftDeck->isPlaying();
        bool rightDeckPlaying = pRightDeck->isPlaying();

        if (leftDeckPlaying && rightDeckPlaying) {
            qDebug() << "One deck must be stopped before enabling Auto DJ mode";
            // Keep the current state.
            emitAutoDJStateChanged(m_eState);
            emit autoDJError(ADJ_BOTH_DECKS_PLAYING);
            return ADJ_BOTH_DECKS_PLAYING;
        }
        // Auto-DJ needs at least two decks
        DEBUG_ASSERT(m_decks.size() > 1);

        // TODO: This is a total band aid for making Auto DJ work with four decks.
        // We should design a nicer way to handle this.
        for (const auto& pDeck : m_decks) {
            VERIFY_OR_DEBUG_ASSERT(pDeck) {
                continue;
            }
            if (pDeck.get() == pLeftDeck) {
                continue;
            }
            if (pDeck.get() == pRightDeck) {
                continue;
            }
            if (pDeck->isPlaying()) {
                // Keep the current state.
                emitAutoDJStateChanged(m_eState);
                emit autoDJError(ADJ_UNUSED_DECK_PLAYING);
                return ADJ_UNUSED_DECK_PLAYING;
            }
        }

        if (pLeftDeck->index > 1 || pRightDeck->index > 1) {
            // Left and/or right deck is deck 3/4 which may not be visible.
            // Make sure it is, if the current skin is a 4-deck skin.
            ControlObject::set(ConfigKey("[Skin]", "show_4decks"), 1);
        }

        // Never load the same track if it is already playing
        if (leftDeckPlaying) {
            removeLoadedTrackFromTopOfQueue(*pLeftDeck);
        } else if (rightDeckPlaying) {
            removeLoadedTrackFromTopOfQueue(*pRightDeck);
        } else {
            // If the first track is already cued at a position in the first
            // 2/3 in on of the Auto DJ decks, start it.
            // If the track is paused at a later position, it is probably too
            // close to the end. In this case it is loaded again at the stored
            // cue point.
            if (pLeftDeck->playPosition() < 0.66 &&
                    removeLoadedTrackFromTopOfQueue(*pLeftDeck)) {
                pLeftDeck->play();
                leftDeckPlaying = true;
            } else if (pRightDeck->playPosition() < 0.66 &&
                    removeLoadedTrackFromTopOfQueue(*pRightDeck)) {
                pRightDeck->play();
                rightDeckPlaying = true;
            }
        }

        TrackPointer nextTrack = getNextTrackFromQueue();
        if (!nextTrack) {
            qDebug() << "Queue is empty now, disable Auto DJ";
            m_enabledAutoDJ.setAndConfirm(0.0);
            emitAutoDJStateChanged(m_eState);
            emit autoDJError(ADJ_QUEUE_EMPTY);
            return ADJ_QUEUE_EMPTY;
        }

        // Track is available so GO
        m_enabledAutoDJ.setAndConfirm(1.0);
        qDebug() << "Auto DJ enabled";

        m_coCrossfader.connectValueChanged(this, &AutoDJProcessor::crossfaderChanged);

        connect(pLeftDeck,
                &DeckAttributes::playPositionChanged,
                this,
                &AutoDJProcessor::playerPositionChanged);
        connect(pRightDeck,
                &DeckAttributes::playPositionChanged,
                this,
                &AutoDJProcessor::playerPositionChanged);

        connect(pLeftDeck,
                &DeckAttributes::playChanged,
                this,
                &AutoDJProcessor::playerPlayChanged);
        connect(pRightDeck,
                &DeckAttributes::playChanged,
                this,
                &AutoDJProcessor::playerPlayChanged);

        connect(pLeftDeck,
                &DeckAttributes::introStartPositionChanged,
                this,
                &AutoDJProcessor::playerIntroStartChanged);
        connect(pRightDeck,
                &DeckAttributes::introStartPositionChanged,
                this,
                &AutoDJProcessor::playerIntroStartChanged);

        connect(pLeftDeck,
                &DeckAttributes::introEndPositionChanged,
                this,
                &AutoDJProcessor::playerIntroEndChanged);
        connect(pRightDeck,
                &DeckAttributes::introEndPositionChanged,
                this,
                &AutoDJProcessor::playerIntroEndChanged);

        connect(pLeftDeck,
                &DeckAttributes::outroStartPositionChanged,
                this,
                &AutoDJProcessor::playerOutroStartChanged);
        connect(pRightDeck,
                &DeckAttributes::outroStartPositionChanged,
                this,
                &AutoDJProcessor::playerOutroStartChanged);

        connect(pLeftDeck,
                &DeckAttributes::outroEndPositionChanged,
                this,
                &AutoDJProcessor::playerOutroEndChanged);
        connect(pRightDeck,
                &DeckAttributes::outroEndPositionChanged,
                this,
                &AutoDJProcessor::playerOutroEndChanged);

        connect(pLeftDeck,
                &DeckAttributes::trackLoaded,
                this,
                &AutoDJProcessor::playerTrackLoaded);
        connect(pRightDeck,
                &DeckAttributes::trackLoaded,
                this,
                &AutoDJProcessor::playerTrackLoaded);

        connect(pLeftDeck,
                &DeckAttributes::loadingTrack,
                this,
                &AutoDJProcessor::playerLoadingTrack);
        connect(pRightDeck,
                &DeckAttributes::loadingTrack,
                this,
                &AutoDJProcessor::playerLoadingTrack);

        connect(pLeftDeck,
                &DeckAttributes::playerEmpty,
                this,
                &AutoDJProcessor::playerEmpty);
        connect(pRightDeck,
                &DeckAttributes::playerEmpty,
                this,
                &AutoDJProcessor::playerEmpty);

        connect(pLeftDeck,
                &DeckAttributes::rateChanged,
                this,
                &AutoDJProcessor::playerRateChanged);
        connect(pRightDeck,
                &DeckAttributes::rateChanged,
                this,
                &AutoDJProcessor::playerRateChanged);
        connect(m_pAutoDJTableModel,
                &PlaylistTableModel::firstTrackChanged,
                this,
                &AutoDJProcessor::playlistFirstTrackChanged);

        if (!leftDeckPlaying && !rightDeckPlaying) {
            // Both decks are stopped. Load a track into deck 1 and start it
            // playing. Instruct playerPositionChanged to wait for a
            // playposition update from deck 1. playerPositionChanged for
            // ADJ_ENABLE_P1LOADED will set the crossfader left and remove the
            // loaded track from the queue and wait for the next call to
            // playerPositionChanged for deck1 after the track is loaded.
            m_eState = ADJ_ENABLE_P1LOADED;

            // Move crossfader to the left.
            setCrossfader(-1.0);

            // Load track into the left deck and play. Once it starts playing,
            // we will receive a playerPositionChanged update for deck 1 which
            // will load a track into the right deck and switch to IDLE mode.
            emitLoadTrackToPlayer(nextTrack, pLeftDeck->group, true);
        } else {
            // One of the two decks is playing. Switch into IDLE mode and wait
            // until the playing deck crosses posThreshold to start fading.
            m_eState = ADJ_IDLE;
            if (leftDeckPlaying) {
                // Load track into the right deck.
                emitLoadTrackToPlayer(nextTrack, pRightDeck->group, false);
                // Move crossfader to the left.
                setCrossfader(-1.0);
            } else {
                // Load track into the left deck.
                emitLoadTrackToPlayer(nextTrack, pLeftDeck->group, false);
                // Move crossfader to the right.
                setCrossfader(1.0);
            }
        }
        emitAutoDJStateChanged(m_eState);
    } else { // Disable Auto DJ
        m_enabledAutoDJ.setAndConfirm(0.0);
        qDebug() << "Auto DJ disabled";
        m_eState = ADJ_DISABLED;
        disconnect(&m_coCrossfader,
                &ControlProxy::valueChanged,
                this,
                &AutoDJProcessor::crossfaderChanged);
        for (const auto& pDeck : m_decks) {
            pDeck->disconnect(this);
        }
        if (m_pConfig->getValue<bool>(ConfigKey(kConfigKey,
                    QStringLiteral("center_xfader_when_disabling")))) {
            m_coCrossfader.set(0);
        }
        emitAutoDJStateChanged(m_eState);
    }
    return ADJ_OK;
}

void AutoDJProcessor::controlEnableChangeRequest(double value) {
    toggleAutoDJ(value > 0.0);
}

void AutoDJProcessor::controlFadeNow(double value) {
    if (value > 0.0) {
        fadeNow();
    }
}

void AutoDJProcessor::controlShuffle(double value) {
    if (value > 0.0) {
        shufflePlaylist(QModelIndexList());
    }
}

void AutoDJProcessor::controlSkipNext(double value) {
    if (value > 0.0) {
        skipNext();
    }
}

void AutoDJProcessor::controlAddRandomTrack(double value) {
    if (value > 0.0) {
        emit randomTrackRequested(1);
    }
}

void AutoDJProcessor::crossfaderChanged(double value) {
    if (m_eState == ADJ_IDLE) {
        // The user is changing the crossfader manually. If the user has
        // moved it all the way to the other side, make the deck faded away
        // from the new "to deck" by loading the next track into it.
        DeckAttributes* pFromDeck = getFromDeck();
        VERIFY_OR_DEBUG_ASSERT(pFromDeck) {
            // we have always a from deck in case of state IDLE
            return;
        }

        DeckAttributes* pToDeck = getOtherDeck(pFromDeck);
        if (!pToDeck) {
            // we have always a from deck in case of state IDLE
            // if the user has not changed the deck orientation
            return;
        }

        double crossfaderPosition = value * (m_coCrossfaderReverse.toBool() ? -1 : 1);
        if ((crossfaderPosition == 1.0 && pFromDeck->isLeft()) ||       // crossfader right
                (crossfaderPosition == -1.0 && pFromDeck->isRight())) { // crossfader left
            if (!pToDeck->isPlaying()) {
                if (getEndSecond(*pToDeck) >= kMinimumTrackDurationSec) {
                    // Re-cue the track if the user has seeked it to the very end
                    if (pToDeck->playPosition() >= pToDeck->fadeBeginPos) {
                        pToDeck->setPlayPosition(pToDeck->startPos);
                    }
                    pToDeck->play();
                } else {
                    // Track in toDeck was ejected manually, stop.
                    toggleAutoDJ(false);
                    return;
                }
            }
            pFromDeck->stop();

            // Now that we have started the other deck playing, remove the track
            // that was "on deck" from the top of the queue.
            removeLoadedTrackFromTopOfQueue(*pToDeck);
            loadNextTrackFromQueue(*pFromDeck);
        }
    }
}

void AutoDJProcessor::playerPositionChanged(DeckAttributes* pAttributes,
                                            double thisPlayPosition) {
    // qDebug() << "player" << pAttributes->group << "PositionChanged(" << value << ")";
    if (m_eState == ADJ_DISABLED) {
        // nothing to do
        return;
    }

    DeckAttributes* thisDeck = pAttributes;
    DeckAttributes* otherDeck = getOtherDeck(thisDeck);
    if (!otherDeck) {
        // This happens if this deck has no orientation or
        // there is no deck with the opposite orientation
        return;
    }

    // Note: this can be a delayed call of playerPositionChanged() where
    // the track was playing, but is now stopped.
    bool thisDeckPlaying = thisDeck->isPlaying();
    bool otherDeckPlaying = otherDeck->isPlaying();

    // To switch out of ADJ_ENABLE_P1LOADED we wait for a playposition update
    // for either deck.
    if (m_eState == ADJ_ENABLE_P1LOADED) {
        DeckAttributes* leftDeck;
        DeckAttributes* rightDeck;

        if (thisDeck->isLeft()) {
            leftDeck = thisDeck;
            DEBUG_ASSERT(otherDeck->isRight());
            rightDeck = otherDeck;
        } else {
            DEBUG_ASSERT(thisDeck->isRight());
            rightDeck = thisDeck;
            DEBUG_ASSERT(otherDeck->isLeft());
            leftDeck = otherDeck;
        }

        // Note: If a playing deck has reached the end the play state is already reset
        bool leftDeckPlaying = leftDeck->isPlaying();
        bool rightDeckPlaying = rightDeck->isPlaying();
        bool leftDeckReachesEnd = thisDeck->isLeft() && thisPlayPosition >= 1.0;

        if (leftDeckPlaying || rightDeckPlaying || leftDeckReachesEnd) {
            // One of left and right is playing. Switch to IDLE mode and make
            // sure our thresholds are configured (by calling calculateTransition
            // for the playing deck).
            m_eState = ADJ_IDLE;

            if (!rightDeckPlaying) {
                // Only left deck playing!
                // In ADJ_ENABLE_P1LOADED mode we wait until the left deck
                // successfully starts playing. We don't know in toggleAutoDJ
                // whether the track will load successfully so we have to
                // wait. If the track fails to load then playerTrackLoadFailed
                // will remove it from the top of the queue and request another
                // track. Remove the left deck's current track from the queue
                // since it is the track we requested in toggleAutoDJ.
                removeLoadedTrackFromTopOfQueue(*leftDeck);

                // Load the next track into the right player since it is not
                // playing.
                loadNextTrackFromQueue(*rightDeck);

                // Note: calculateTransition() is called in playerTrackLoaded()
            } else {
                // At least right deck is playing
                // Set crossfade thresholds for right deck.
                if constexpr (sDebug) {
                    qDebug() << this << "playerPositionChanged"
                             << "right deck playing";
                }
                calculateTransition(rightDeck, leftDeck, false);
            }
            emitAutoDJStateChanged(m_eState);
        }
        return;
    }

    // In FADING states, we expect that both tracks are playing.
    // Normally the the fading fromDeck stops after the transition is over and
    // we need to replace it with a new track from the queue.
    if (m_eState == ADJ_LEFT_FADING || m_eState == ADJ_RIGHT_FADING) {
        // Once P1 or P2 has stopped switch out of fading mode to idle.
        // If the user stops the toDeck during a fade, let the fade continue
        // and do not load the next track.
        if (!otherDeckPlaying && otherDeck->isFromDeck) {
            // Force crossfader all the way to the (non fading) toDeck.
            if (m_eState == ADJ_RIGHT_FADING) {
                setCrossfader(-1.0);
            } else {
                setCrossfader(1.0);
            }
            m_eState = ADJ_IDLE;
            // Invalidate threshold calculated for the old otherDeck
            // This avoids starting a fade back before the new track is
            // loaded into the otherDeck
            thisDeck->fadeBeginPos = 1.0;
            thisDeck->fadeEndPos = 1.0;
            otherDeck->isFromDeck = false;
            // Load the next track to otherDeck.
            loadNextTrackFromQueue(*otherDeck);
            emitAutoDJStateChanged(m_eState);
            return;
        }
    }

    if (m_eState == ADJ_IDLE) {
        if (!thisDeckPlaying && thisPlayPosition < 1) {
            // this is a cueing seek, recalculate the transition, from the
            // new position.
            // This can be our own seek to startPos or a random seek by a user.
            // we need to call calculateTransition() because we are not sure.
            // If using the full track mode with a transition time of 0,
            // thisDeckPlaying will be false but the transition should not be
            // recalculated here.
            // Don't adjust transition when reaching the end. In this case it is
            // always stopped.
            if constexpr (sDebug) {
                qDebug() << this << "playerPositionChanged"
                         << "cueing seek";
            }
            calculateTransition(otherDeck, thisDeck, false);
        } else if (thisDeck->isRepeat()) {
            // repeat pauses auto DJ
            return;
        }
    }

    // If we are past this deck's posThreshold then:
    // - transition into fading mode, play the other deck and fade to it.
    // - check if fading is done and stop the deck
    // - update the crossfader
    if (thisPlayPosition >= thisDeck->fadeBeginPos && thisDeck->isFromDeck && !otherDeck->loading) {
        if (m_eState == ADJ_IDLE) {
            if (thisDeckPlaying || thisPlayPosition >= 1.0) {
                // Set the state as FADING.
                m_eState = thisDeck->isLeft() ? ADJ_LEFT_FADING : ADJ_RIGHT_FADING;
                m_transitionProgress = 0.0;
                emitAutoDJStateChanged(m_eState);

                const double toDeckFadeDistance =
                        (thisDeck->fadeEndPos - thisDeck->fadeBeginPos) *
                        getEndSecond(*thisDeck) / getEndSecond(*otherDeck);
                // Re-cue the track if the user has seeked forward and will miss the fadeBeginPos
                if (otherDeck->playPosition() >= otherDeck->fadeBeginPos - toDeckFadeDistance) {
                    otherDeck->setPlayPosition(otherDeck->startPos);
                }

                if (m_crossfaderStartCenter) {
                    setCrossfader(0.0);
                } else if (thisDeck->fadeBeginPos >= thisDeck->fadeEndPos) {
                    setCrossfader(thisDeck->isLeft() ? 1.0 : -1.0);
                }

                if (!otherDeckPlaying) {
                    otherDeck->play();
                }

                // Now that we have started the other deck playing, remove the track
                // that was "on deck" from the top of the queue.
                // Note: This is a DB call and takes long.
                removeLoadedTrackFromTopOfQueue(*otherDeck);
            } else {
                if constexpr (sDebug) {
                    qDebug() << this << "playerPositionChanged()"
                             << pAttributes->group << thisPlayPosition
                             << "but not playing";
                }
            }
        }

        double crossfaderTarget;
        if (m_eState == ADJ_LEFT_FADING) {
            crossfaderTarget = 1.0;

        } else if (m_eState == ADJ_RIGHT_FADING) {
            crossfaderTarget = -1.0;
        } else {
            // this happens if the not playing track is cued into the outro region,
            // calculated for the swapped roles.
            return;
        }

        double currentCrossfader = getCrossfader();

        if (currentCrossfader == crossfaderTarget) {
            // We are done, the fading (from) track is silenced.
            // We don't handle mode switches here since that's handled by
            // the next playerPositionChanged call otherDeck (see the
            // P1/P2FADING case above).
            thisDeck->stop();
            m_transitionProgress = 1.0;
            // Note: If the user has stopped the toDeck during the transition.
            // this deck just stops as well. In this case a stopped AutoDJ is accepted
            // because the use did it intentionally
        } else {
            // We are in Fading state.
            // Calculate the current transitionProgress, the place between begin
            // and end position and the step we have taken since the last call
            double transitionProgress = (thisPlayPosition - thisDeck->fadeBeginPos) /
                    (thisDeck->fadeEndPos - thisDeck->fadeBeginPos);
            double transitionStep = transitionProgress - m_transitionProgress;
            if (transitionStep > 0.0) {
                // We have made progress.
                // Backward seeks pause the transitions; forward seeks speed up
                // the transitions. If there has been a seek beyond endPos, end
                // the transition immediately.
                double remainingCrossfader = crossfaderTarget - currentCrossfader;
                double adjustment = remainingCrossfader /
                        (1.0 - m_transitionProgress) * transitionStep;
                // we move the crossfader linearly with
                // movements in this track's play position.
                setCrossfader(currentCrossfader + adjustment);
            }
            m_transitionProgress = transitionProgress;
            // if we are at 1.0 here, we need an additional callback until the last
            // step is processed and we can stop the deck.
        }
    }
}

TrackPointer AutoDJProcessor::getNextTrackFromQueue() {
    // Get the track at the top of the playlist.
    bool randomQueueEnabled = m_pConfig->getValue<bool>(
            ConfigKey("[Auto DJ]", "EnableRandomQueue"));
    int minAutoDJCrateTracks = m_pConfig->getValueString(
            ConfigKey(kConfigKey, "RandomQueueMinimumAllowed")).toInt();
    int tracksToAdd = minAutoDJCrateTracks - m_pAutoDJTableModel->rowCount();
    // In case we start off with < minimum tracks
    if (randomQueueEnabled && (tracksToAdd > 0)) {
        emit randomTrackRequested(tracksToAdd);
    }

    while (true) {
        TrackPointer pNextTrack = m_pAutoDJTableModel->getTrack(
                m_pAutoDJTableModel->index(0, 0));

        if (pNextTrack) {
            if (pNextTrack->getFileInfo().checkFileExists()) {
                return pNextTrack;
            } else {
                // Remove missing track from auto DJ playlist.
                qWarning() << "Auto DJ: Skip missing track" << pNextTrack->getLocation();
                m_pAutoDJTableModel->removeTrack(
                        m_pAutoDJTableModel->index(0, 0));
                // Don't "Requeue" missing tracks to avoid andless loops
                maybeFillRandomTracks();
            }
        } else {
            // We're out of tracks. Return the null TrackPointer.
            return pNextTrack;
        }
    }
}

bool AutoDJProcessor::loadNextTrackFromQueue(const DeckAttributes& deck, bool play) {
    TrackPointer nextTrack = getNextTrackFromQueue();

    // We ran out of tracks in the queue.
    if (!nextTrack) {
        // Disable AutoDJ.
        toggleAutoDJ(false);

        // And eject track (nextTrack is null) as "End of auto DJ warning"
        emitLoadTrackToPlayer(nextTrack, deck.group, false);
        return false;
    }

    emitLoadTrackToPlayer(nextTrack, deck.group, play);
    return true;
}

bool AutoDJProcessor::removeLoadedTrackFromTopOfQueue(const DeckAttributes& deck) {
    return removeTrackFromTopOfQueue(deck.getLoadedTrack());
}

bool AutoDJProcessor::removeTrackFromTopOfQueue(TrackPointer pTrack) {
    // No track to test for.
    if (!pTrack) {
        return false;
    }

    TrackId trackId(pTrack->getId());

    // Loaded track is not a library track.
    if (!trackId.isValid()) {
        return false;
    }

    // Get the track id at the top of the playlist.
    TrackId nextId(m_pAutoDJTableModel->getTrackId(
            m_pAutoDJTableModel->index(0, 0)));

    // No track at the top of the queue.
    if (!nextId.isValid()) {
        return false;
    }

    // If the loaded track is not the next track in the queue then do nothing.
    if (trackId != nextId) {
        return false;
    }

    // Remove the top track.
    m_pAutoDJTableModel->removeTrack(m_pAutoDJTableModel->index(0, 0));

    // Re-queue if configured.
    if (m_pConfig->getValueString(ConfigKey(kConfigKey, "Requeue")).toInt()) {
        m_pAutoDJTableModel->appendTrack(nextId);
    }

    maybeFillRandomTracks();
    return true;
}

void AutoDJProcessor::maybeFillRandomTracks() {
    int minAutoDJCrateTracks = m_pConfig->getValueString(
            ConfigKey(kConfigKey, "RandomQueueMinimumAllowed")).toInt();
    bool randomQueueEnabled = (((m_pConfig->getValueString(
            ConfigKey("[Auto DJ]", "EnableRandomQueue")).toInt())) == 1);

    int tracksToAdd = minAutoDJCrateTracks - m_pAutoDJTableModel->rowCount();
    if (randomQueueEnabled && (tracksToAdd > 0)) {
        qDebug() << "Randomly adding tracks";
        emit randomTrackRequested(tracksToAdd);
    }
}

void AutoDJProcessor::playerPlayChanged(DeckAttributes* thisDeck, bool playing) {
    if constexpr (sDebug) {
        qDebug() << this << "playerPlayChanged" << thisDeck->group << playing;
    }

    if (m_eState != ADJ_IDLE) {
        // We don't want to recalculate a running transition
        return;
    }

    if (thisDeck->loading) {
        // Note: When loading a new deck this signal arrives before the
        // playerTrackLoaded();
        return;
    }

    DeckAttributes* otherDeck = getOtherDeck(thisDeck);
    if (!otherDeck) {
        // This happens if all decks have center orientation
        return;
    }

    if (playing) {
        if (!otherDeck->isPlaying()) {
            // In case both decks were stopped and now this one just started, make
            // this deck the "from deck".
            calculateTransition(thisDeck, getOtherDeck(thisDeck), false);
        }
    } else {
        // Deck paused
        // This may happen if the user has previously pressed play on the "to deck"
        // before fading, for example to adjust the intro/outro cues, and lets the
        // deck play until the end, seek back to the start point instead of keeping
        if (thisDeck->playPosition() >= 1.0 && !thisDeck->isFromDeck) {
            // toDeck has stopped at the end. Recalculate the transition, because
            // it has been done from a now irrelevant previous position.
            // This forces the other deck to be the fromDeck.
            thisDeck->startPos = kKeepPosition;
            calculateTransition(otherDeck, thisDeck, true);
            if (thisDeck->startPos != kKeepPosition) {
                // Note: this seek will trigger the playerPositionChanged slot
                // which may calls the calculateTransition() again without seek = true;
                thisDeck->setPlayPosition(thisDeck->startPos);
            }
        }
    }
}

void AutoDJProcessor::playerIntroStartChanged(DeckAttributes* pAttributes, double position) {
    if constexpr (sDebug) {
        qDebug() << this << "playerIntroStartChanged" << pAttributes->group << position;
    }
    // nothing to do, because we want not to re-cue the toDeck and the from
    // Deck has already passed the intro
}

void AutoDJProcessor::playerIntroEndChanged(DeckAttributes* pAttributes, double position) {
    if constexpr (sDebug) {
        qDebug() << this << "playerIntroEndChanged" << pAttributes->group << position;
    }

    if (m_eState != ADJ_IDLE) {
        // We don't want to recalculate a running transition
        return;
    }

    if (pAttributes->isFromDeck) {
        // We have already passed the intro
        return;
    }
    DeckAttributes* fromDeck = getFromDeck();
    if (!fromDeck) {
        return;
    }
    calculateTransition(fromDeck, getOtherDeck(fromDeck), false);
}

void AutoDJProcessor::playerOutroStartChanged(DeckAttributes* pAttributes, double position) {
    if constexpr (sDebug) {
        qDebug() << this << "playerOutroStartChanged" << pAttributes->group << position;
    }

    if (m_eState != ADJ_IDLE) {
        // We don't want to recalculate a running transition
        return;
    }

    DeckAttributes* fromDeck = getFromDeck();
    if (!fromDeck) {
        return;
    }
    calculateTransition(fromDeck, getOtherDeck(fromDeck), false);
}

void AutoDJProcessor::playerOutroEndChanged(DeckAttributes* pAttributes, double position) {
    if constexpr (sDebug) {
        qDebug() << this << "playerOutroEndChanged" << pAttributes->group << position;
    }

    if (m_eState != ADJ_IDLE) {
        // We don't want to recalculate a running transition
        return;
    }

    DeckAttributes* fromDeck = getFromDeck();
    if (!fromDeck) {
        return;
    }
    calculateTransition(fromDeck, getOtherDeck(fromDeck), false);
}

double AutoDJProcessor::getIntroStartSecond(const TrackOrDeckAttributes& track) {
    const mixxx::audio::FramePos trackEnd = track.trackEndPosition();
    const mixxx::audio::FramePos introStart = track.introStartPosition();
    const mixxx::audio::FramePos introEnd = track.introEndPosition();
    if (!introStart.isValid() || introStart > trackEnd) {
        double firstSoundSecond = getFirstSoundSecond(track);
        if (!introEnd.isValid() || introEnd > trackEnd) {
            // No intro start and intro end set, use First Sound.
            return firstSoundSecond;
        }
        double introEndSecond = framePositionToSeconds(introEnd, track);
        if (m_transitionTime >= 0) {
            return introEndSecond - m_transitionTime;
        }
        return introEndSecond;
    }
    return framePositionToSeconds(introStart, track);
}

double AutoDJProcessor::getIntroEndSecond(const TrackOrDeckAttributes& track) {
    const mixxx::audio::FramePos trackEnd = track.trackEndPosition();
    const mixxx::audio::FramePos introEnd = track.introEndPosition();
    if (!introEnd.isValid() || introEnd > trackEnd) {
        // Assume a zero length intro if introEnd is not set.
        // The introStart is automatically placed by AnalyzerSilence, so use
        // that as a fallback if the user has not placed outroStart. If it has
        // not been placed, getIntroStartPosition will return 0:00.
        return getIntroStartSecond(track);
    }
    return framePositionToSeconds(introEnd, track);
}

double AutoDJProcessor::getOutroStartSecond(const TrackOrDeckAttributes& track) {
    const mixxx::audio::FramePos trackEnd = track.trackEndPosition();
    const mixxx::audio::FramePos outroStart = track.outroStartPosition();
    if (!outroStart.isValid() || outroStart > trackEnd) {
        // Assume a zero length outro if outroStart is not set.
        // The outroEnd is automatically placed by AnalyzerSilence, so use
        // that as a fallback if the user has not placed outroStart. If it has
        // not been placed, getOutroEndPosition will return the end of the track.
        return getOutroEndSecond(track);
    }
    return framePositionToSeconds(outroStart, track);
}

double AutoDJProcessor::getOutroEndSecond(const TrackOrDeckAttributes& track) {
    const mixxx::audio::FramePos trackEnd = track.trackEndPosition();
    const mixxx::audio::FramePos outroStart = track.outroStartPosition();
    const mixxx::audio::FramePos outroEnd = track.outroEndPosition();
    if (!outroEnd.isValid() || outroEnd > trackEnd) {
        double lastSoundSecond = getLastSoundSecond(track);
        DEBUG_ASSERT(lastSoundSecond <= framePositionToSeconds(trackEnd, track));
        if (!outroStart.isValid() || outroStart > trackEnd) {
            // No outro start and outro end set, use Last Sound.
            return lastSoundSecond;
        }
        // Try to find a better Outro End using Outro Start and transition time
        double outroStartSecond = framePositionToSeconds(outroStart, track);
        if (m_transitionTime >= 0 && lastSoundSecond > outroStartSecond) {
            double outroEndFromTime = outroStartSecond + m_transitionTime;
            if (outroEndFromTime < lastSoundSecond) {
                // The outroEnd is automatically placed by AnalyzerSilence at the last sound
                // Here the user has removed it, but has placed a outro start.
                // Use the transition time instead of the dismissed last sound position.
                return outroEndFromTime;
            }
            return lastSoundSecond;
        }
        return outroStartSecond;
    }
    return framePositionToSeconds(outroEnd, track);
}

double AutoDJProcessor::getFirstSoundSecond(const TrackOrDeckAttributes& track) {
    TrackPointer trackData = track.getLoadedTrack();
    if (!trackData) {
        return 0.0;
    }

    CuePointer pFromTrackN60dBSound = trackData->findCueByType(mixxx::CueType::N60dBSound);
    if (pFromTrackN60dBSound) {
        const mixxx::audio::FramePos firstSound = pFromTrackN60dBSound->getPosition();
        if (firstSound.isValid()) {
            const mixxx::audio::FramePos trackEndPosition = track.trackEndPosition();
            if (firstSound <= trackEndPosition) {
                return framePositionToSeconds(firstSound, track);
            } else {
                qWarning() << "-60 dB Sound Cue starts after track end in:"
                           << trackData->getLocation()
                           << "Using the first sample instead.";
            }
        }
    }
    return 0.0;
}

double AutoDJProcessor::getLastSoundSecond(const TrackOrDeckAttributes& track) {
    TrackPointer trackData = track.getLoadedTrack();
    if (!trackData) {
        return 0.0;
    }

    const mixxx::audio::FramePos trackEnd = track.trackEndPosition();
    CuePointer pFromTrackN60dBSound = trackData->findCueByType(mixxx::CueType::N60dBSound);
    if (pFromTrackN60dBSound && pFromTrackN60dBSound->getLengthFrames() > 0.0) {
        const mixxx::audio::FramePos lastSound = pFromTrackN60dBSound->getEndPosition();
        if (lastSound > mixxx::audio::FramePos(0.0)) {
            if (lastSound <= trackEnd) {
                return framePositionToSeconds(lastSound, track);
            } else {
                qWarning() << "-60 dB Sound Cue ends after track end in:"
                           << trackData->getLocation()
                           << "Using the last sample instead.";
            }
        }
    }
    return framePositionToSeconds(trackEnd, track);
}

double AutoDJProcessor::getEndSecond(const TrackOrDeckAttributes& track) {
    if (track.isEmpty()) {
        return 0.0;
    }

    mixxx::audio::FramePos trackEnd = track.trackEndPosition();
    return framePositionToSeconds(trackEnd, track);
}

double AutoDJProcessor::framePositionToSeconds(
        mixxx::audio::FramePos position, const TrackOrDeckAttributes& track) {
    mixxx::audio::SampleRate sampleRate = track.sampleRate();
    if (!sampleRate.isValid() || !position.isValid()) {
        return 0.0;
    }

    return position.value() / sampleRate / track.rateRatio();
}

void AutoDJProcessor::calculateTransition(DeckAttributes* pFromDeck,
        DeckAttributes* pToDeck,
        bool seekToStartPoint) {
    VERIFY_OR_DEBUG_ASSERT(pFromDeck && pToDeck) {
        return;
    }
    if (pFromDeck->loading || pToDeck->loading) {
        // don't use halve new halve old data during
        // changing of tracks
        return;
    }

    // We require ADJ_IDLE to prevent changing the thresholds in the middle of a
    // fade.
    VERIFY_OR_DEBUG_ASSERT(m_eState == ADJ_IDLE) {
        return;
    }

    calculateTransitionImpl(*pFromDeck, *pToDeck, seekToStartPoint);

    if (pToDeck->startPos == kSkipToNextTrack) {
        // This is a safety measure to handle tracks that are too short.
        // Immediately skip to the next track instead.
        loadNextTrackFromQueue(*pToDeck, false);
        return;
    }

    if constexpr (sDebug) {
        qDebug() << this << "calculateTransition" << pFromDeck->group
                 << pFromDeck->fadeBeginPos << pFromDeck->fadeEndPos
                 << pToDeck->startPos;
    }
}

void AutoDJProcessor::calculateTransitionImpl(
        FadeableTrackOrDeckAttributes& fromTrack,
        FadeableTrackOrDeckAttributes& toTrack,
        bool seekToStartPoint) {
    // ===================================
    // Check for tracks that are too short
    // ===================================

    const double fromDeckEndSecond = getEndSecond(fromTrack);
    const double toDeckEndSecond = getEndSecond(toTrack);
    // Since the end position is measured in seconds from 0:00 it is also
    // the track duration. Use this alias for better readability.
    const double fromDeckDuration = fromDeckEndSecond;
    const double toDeckDuration = toDeckEndSecond;

    VERIFY_OR_DEBUG_ASSERT(fromDeckDuration >= kMinimumTrackDurationSec) {
        // Track has no duration or too short. This should not happen, because short
        // tracks are skipped after load. Play ToDeck immediately.
        fromTrack.fadeBeginPos = 0;
        fromTrack.fadeEndPos = 0;
        toTrack.startPos = kKeepPosition;
        return;
    }

    if (toDeckDuration == 0) {
        // This is a seek call to zero after ejecting the track
        // this signal is received before the track pointer becomes null
        return;
    }

    VERIFY_OR_DEBUG_ASSERT(toDeckDuration >= kMinimumTrackDurationSec) {
        // Track has no duration or too short. This should not happen, because short
        // tracks are skipped after load. Immediately pick next track from queue.
        toTrack.startPos = kSkipToNextTrack;
        return;
    }

    // ========================
    // Handle intros and outros
    // ========================

    // Within this function, the outro refers to the outro of the currently
    // playing track and the intro refers to the intro of the next track.

    double outroEndSecond = getOutroEndSecond(fromTrack);
    double outroStartSecond = getOutroStartSecond(fromTrack);
    const double fromDeckPosition = fromDeckDuration * fromTrack.playPosition();

    VERIFY_OR_DEBUG_ASSERT(outroEndSecond <= fromDeckEndSecond) {
        outroEndSecond = fromDeckEndSecond;
    }

    if (fromDeckPosition > outroStartSecond) {
        // We have already passed outroStart
        // This can happen if we have just enabled auto DJ
        outroStartSecond = fromDeckPosition;
        if (fromDeckPosition > outroEndSecond) {
            outroEndSecond = math_min(outroStartSecond + fabs(m_transitionTime), fromDeckEndSecond);
        }
    }
    double outroLength = outroEndSecond - outroStartSecond;

    double toDeckPositionSeconds = toDeckDuration * toTrack.playPosition();
    // Store here a possible fadeBeginPos for the transition after next
    // This is used to check if it will be possible or a re-cue is required.
    // here it is done for FullIntroOutro and FadeAtOutroStart.
    // It is adjusted below for the other modes.
    toTrack.fadeEndPos = getOutroEndSecond(toTrack);
    double toDeckOutroStartSecond = getOutroStartSecond(toTrack);
    if (toTrack.fadeEndPos == toDeckOutroStartSecond) {
        // outro not defined, use transition time.
        toDeckOutroStartSecond -= m_transitionTime;
    }
    toTrack.fadeBeginPos = toDeckOutroStartSecond;

    double toDeckStartSeconds = toDeckPositionSeconds;
    const double introStartSecond = getIntroStartSecond(toTrack);
    const double introEndSecond = getIntroEndSecond(toTrack);
    if (seekToStartPoint || toDeckPositionSeconds >= toTrack.fadeBeginPos) {
        // toDeckPosition >= toTrack.fadeBeginPos happens when the
        // user has seeked or played the to track behind fadeBeginPos of
        // the fade after the next.
        // In this case we recue the track just before the transition.
        toDeckStartSeconds = introStartSecond;
    }

    double introLength = 0;

    // introEnd is equal introStart in case it has not yet been set
    if (toDeckStartSeconds < introEndSecond && introStartSecond < introEndSecond) {
        // Limit the intro length that results from a revers seek
        // to a reasonable values. If the seek was too big, ignore it.
        introLength = introEndSecond - toDeckStartSeconds;
        if (introLength > (introEndSecond - introStartSecond) * 2 &&
                introLength > (introEndSecond - introStartSecond) + m_transitionTime &&
                introLength > outroLength) {
            introLength = 0;
        }
    }

    if constexpr (sDebug) {
        qDebug() << this << "calculateTransition"
                 << "introLength" << introLength
                 << "outroLength" << outroLength;
    }

    m_crossfaderStartCenter = false;
    switch (m_transitionMode) {
    case TransitionMode::FullIntroOutro: {
        // Use the outro or intro length for the transition time, whichever is
        // shorter. Let the full outro and intro play; do not cut off any part
        // of either.
        //
        // In the diagrams below,
        // - is part of a track outside the outro/intro,
        // o is part of the outro
        // i is part of the intro
        // | marks the boundaries of the transition
        //
        // When outro > intro:
        // ------ooo|ooo|
        //          |iii|------
        //
        // When outro < intro:
        // ------|ooo|
        //       |iii|iii-----
        //
        // If only the outro or intro length is marked but not both, use the one
        // that is marked for the transition time. If neither is marked, fall
        // back to the transition time from the spinbox.
        double transitionLength = introLength;
        if (outroLength > 0) {
            if (transitionLength <= 0 || transitionLength > outroLength) {
                // Use outro length when the intro is not defined or longer
                // than the outro.
                transitionLength = outroLength;
            }
        }
        if (transitionLength > 0) {
            const double transitionEnd = toDeckStartSeconds + transitionLength;
            if (transitionEnd > toTrack.fadeBeginPos) {
                // End intro before next outro starts
                transitionLength = toTrack.fadeBeginPos - toDeckStartSeconds;
                VERIFY_OR_DEBUG_ASSERT(transitionLength > 0) {
                    // We seek to intro start above in this case so this never happens
                    transitionLength = 1;
                }
            }
            fromTrack.fadeBeginPos = outroEndSecond - transitionLength;
            fromTrack.fadeEndPos = outroEndSecond;
            toTrack.startPos = toDeckStartSeconds;
        } else {
            useFixedFadeTime(fromTrack,
                    toTrack,
                    fromDeckPosition,
                    outroEndSecond,
                    toDeckStartSeconds);
        }
    } break;
    case TransitionMode::FadeAtOutroStart: {
        // Use the outro or intro length for the transition time, whichever is
        // shorter. If the outro is longer than the intro, cut off the end
        // of the outro.
        //
        // In the diagrams below,
        // - is part of a track outside the outro/intro,
        // o is part of the outro
        // i is part of the intro
        // | marks the boundaries of the transition
        //
        // When outro > intro:
        // ------|ooo|ooo
        //       |iii|------
        //
        // When outro < intro:
        // ------|ooo|
        //       |iii|iii-----
        //
        // If only the outro or intro length is marked but not both, use the one
        // that is marked for the transition time. If neither is marked, fall
        // back to the transition time from the spinbox.
        double transitionLength = outroLength;
        if (transitionLength > 0) {
            if (introLength > 0) {
                if (outroLength > introLength) {
                    // Cut off end of outro
                    transitionLength = introLength;
                }
            }
            const double transitionEnd = toDeckStartSeconds + transitionLength;
            if (transitionEnd > toTrack.fadeBeginPos) {
                // End intro before next outro starts
                transitionLength = toTrack.fadeBeginPos - toDeckStartSeconds;
                VERIFY_OR_DEBUG_ASSERT(transitionLength > 0) {
                    // We seek to intro start above in this case so this never happens
                    transitionLength = 1;
                }
            }
            fromTrack.fadeBeginPos = outroStartSecond;
            fromTrack.fadeEndPos = outroStartSecond + transitionLength;
            toTrack.startPos = toDeckStartSeconds;
        } else if (introLength > 0) {
            transitionLength = introLength;
            fromTrack.fadeBeginPos = outroEndSecond - transitionLength;
            fromTrack.fadeEndPos = outroEndSecond;
            toTrack.startPos = toDeckStartSeconds;
        } else {
            useFixedFadeTime(fromTrack,
                    toTrack,
                    fromDeckPosition,
                    outroEndSecond,
                    toDeckStartSeconds);
        }
    } break;
    case TransitionMode::FixedStartCenterSkipSilence:
        m_crossfaderStartCenter = true;
        // fall through intended!
        [[fallthrough]];
    case TransitionMode::FixedSkipSilence: {
        double toDeckStartSecond;
        toTrack.fadeBeginPos = getLastSoundSecond(toTrack);
        if (seekToStartPoint || toDeckPositionSeconds >= toTrack.fadeBeginPos) {
            // toDeckPosition >= toTrack.fadeBeginPos happens when the
            // user has seeked or played the to track behind fadeBeginPos of
            // the fade after the next.
            // In this case we recue the track just before the transition.
            toDeckStartSecond = getFirstSoundSecond(toTrack);
        } else {
            toDeckStartSecond = toDeckPositionSeconds;
        }
        useFixedFadeTime(
                fromTrack,
                toTrack,
                fromDeckPosition,
                getLastSoundSecond(fromTrack),
                toDeckStartSecond);
    } break;
    case TransitionMode::FixedFullTrack:
    default: {
        double startPoint;
        toTrack.fadeBeginPos = toDeckEndSecond;
        if (seekToStartPoint || toDeckPositionSeconds >= toTrack.fadeBeginPos) {
            // toDeckPosition >= toTrack.fadeBeginPos happens when the
            // user has seeked or played the to track behind fadeBeginPos of
            // the fade after the next.
            // In this case we recue the track just before the transition.
            startPoint = 0.0;
        } else {
            startPoint = toDeckPositionSeconds;
        }
        useFixedFadeTime(fromTrack, toTrack, fromDeckPosition, fromDeckEndSecond, startPoint);
        }
    }

    // The positions are expected to be a fraction of the track length.
    fromTrack.fadeBeginPos /= fromDeckDuration;
    fromTrack.fadeEndPos /= fromDeckDuration;
    toTrack.startPos /= toDeckDuration;
    toTrack.fadeBeginPos /= toDeckDuration;
    toTrack.fadeEndPos /= toDeckDuration;

    fromTrack.isFromDeck = true;
    toTrack.isFromDeck = false;

    VERIFY_OR_DEBUG_ASSERT(fromTrack.fadeBeginPos <= 1) {
        fromTrack.fadeBeginPos = 1;
    }
}

void AutoDJProcessor::useFixedFadeTime(
        FadeableTrackOrDeckAttributes& fromTrack,
        FadeableTrackOrDeckAttributes& toTrack,
        double fromDeckSecond,
        double fadeEndSecond,
        double toDeckStartSecond) {
    if (m_transitionTime > 0.0) {
        // Guard against the next track being too short. This transition must finish
        // before the next transition starts.
        double toDeckOutroStart = toTrack.fadeBeginPos;
        if (toTrack.fadeBeginPos >= toTrack.fadeEndPos) {
            // no outro defined, the toDeck will also use the transition time
            toDeckOutroStart -= m_transitionTime;
        }
        if (toDeckOutroStart <= toDeckStartSecond + kMinimumTrackDurationSec) {
            // we have already passed the outro start
            // Check OutroEnd as alternative, which is for all transition mode
            // better than directly defaulting to duration()
            double end = getOutroEndSecond(toTrack);
            if (end <= toDeckStartSecond + kMinimumTrackDurationSec) {
                // we have also passed the outro end
                end = getEndSecond(toTrack);
                VERIFY_OR_DEBUG_ASSERT(end > toDeckStartSecond + kMinimumTrackDurationSec) {
                    // as last resort move start point
                    // The caller makes sure that this never happens
                    toDeckStartSecond = end - kMinimumTrackDurationSec;
                }
            }
            // use the remaining time for fading
            toDeckOutroStart = (end - toDeckStartSecond) / 2 + toDeckStartSecond;
        }
        double transitionTime = math_min(toDeckOutroStart - toDeckStartSecond,
                m_transitionTime);
        VERIFY_OR_DEBUG_ASSERT(transitionTime >= kMinimumTrackDurationSec / 2) {
            transitionTime = kMinimumTrackDurationSec / 2;
        }
        // Note: fromDeck.fadeBeginPos >= fromDeck.fadeEndPos is handled in
        // playerPositionChanged() causing a jump cut.
        fromTrack.fadeBeginPos = math_max(fadeEndSecond - transitionTime, fromDeckSecond);
        fromTrack.fadeEndPos = fadeEndSecond;
        toTrack.startPos = toDeckStartSecond;
    } else {
        fromTrack.fadeBeginPos = fadeEndSecond;
        fromTrack.fadeEndPos = fadeEndSecond;
        toTrack.startPos = toDeckStartSecond + m_transitionTime;
    }
}

void AutoDJProcessor::playerTrackLoaded(DeckAttributes* pDeck, TrackPointer pTrack) {
    if constexpr (sDebug) {
        qDebug() << this << "playerTrackLoaded" << pDeck->group
                 << (pTrack ? pTrack->getLocation() : "(null)");
    }

    pDeck->loading = false;

    // Since the end position is measured in seconds from 0:00 it is also
    // the track duration.
    double duration = getEndSecond(*pDeck);
    if (duration < kMinimumTrackDurationSec) {
        qWarning() << "Skip track with" << duration << "Duration"
                   << pTrack->getLocation();
        // Remove Track with duration smaller than two callbacks
        removeTrackFromTopOfQueue(pTrack);

        // Load the next track. If we are the first AutoDJ track
        // (ADJ_ENABLE_P1LOADED state) then play the track.
        loadNextTrackFromQueue(*pDeck, m_eState == ADJ_ENABLE_P1LOADED);
    } else if (m_eState == ADJ_IDLE) {
        // this deck has just changed the track so it becomes the toDeck
        DeckAttributes* fromDeck = getOtherDeck(pDeck);
        // check if this deck has suitable alignment
        if (fromDeck && getOtherDeck(fromDeck) != pDeck) {
            if constexpr (sDebug) {
                qDebug() << this << "playerTrackLoaded()" << pDeck->group << "but not a toDeck";
            }
            // User has changed the orientation, disable Auto DJ
            toggleAutoDJ(false);
            emit autoDJError(ADJ_NOT_TWO_DECKS);
            return;
        }
        pDeck->startPos = kKeepPosition;
        calculateTransition(fromDeck, pDeck, true);
        if (pDeck->startPos != kKeepPosition) {
            // Note: this seek will trigger the playerPositionChanged slot
            // which may call the calculateTransition() again without seek = true;
            pDeck->setPlayPosition(pDeck->startPos);
        }
        // we are here in the relative domain 0..1
        if (!fromDeck->isPlaying() && fromDeck->playPosition() >= 1.0) {
            // repeat a probably missed update
            playerPositionChanged(fromDeck, 1.0);
        }
    } else if (m_eState == ADJ_LEFT_FADING) {
        if (pDeck == getRightDeck()) {
            // restore the play state lost during loading
            pDeck->play();
        }
    } else if (m_eState == ADJ_RIGHT_FADING) {
        if (pDeck == getLeftDeck()) {
            // restore the play state lost during loading
            pDeck->play();
        }
    }
}

void AutoDJProcessor::playerLoadingTrack(DeckAttributes* pDeck,
        TrackPointer pNewTrack, TrackPointer pOldTrack) {
    if constexpr (sDebug) {
        qDebug() << this << "playerLoadingTrack" << pDeck->group
                 << "new:" << (pNewTrack ? pNewTrack->getLocation() : "(null)")
                 << "old:" << (pOldTrack ? pOldTrack->getLocation() : "(null)");
    }

    pDeck->loading = true;

    // The Deck is loading an new track

    // There are four conditions under which we load a track.
    // 1) We are enabling AutoDJ and no decks are playing. Mode is
    //    ADJ_ENABLE_P1LOADED.
    // 2) After #1, we load a track into the other deck. Mode is ADJ_IDLE.
    // 3) We are enabling AutoDJ and a single deck is playing. Mode is ADJ_IDLE.
    // 4) We have just completed fading from one deck to another. Mode is
    //    ADJ_IDLE.

    if (!pNewTrack) {
        // If a track is ejected because of a manual eject command or a load failure
        // this track seems to be undesired. Remove the bad track from the queue.
        removeTrackFromTopOfQueue(pOldTrack);

        // Wait until the track is fully unloaded and the playerEmpty()
        // slot is called before loading an alternative track.
    }
}

void AutoDJProcessor::playerEmpty(DeckAttributes* pDeck) {
    if constexpr (sDebug) {
        qDebug() << this << "playerEmpty()" << pDeck->group;
    }

    // The Deck has ejected a track and no new one is loaded.
    // This happens if loading fails or the user manually ejected the track
    // and would normally stop the AutoDJ flow, which is not desired.
    // It should be safe to load a new track from the queue. The only case where
    // we request a load-and-play is case #1 currently so we can easily test for
    // this based on the mode.

    // Load the next track. If we are the first AutoDJ track
    // (ADJ_ENABLE_P1LOADED state) then play the track.
    loadNextTrackFromQueue(*pDeck, m_eState == ADJ_ENABLE_P1LOADED);
}

void AutoDJProcessor::playerRateChanged(DeckAttributes* pAttributes) {
    if constexpr (sDebug) {
        qDebug() << this << "playerRateChanged" << pAttributes->group;
    }

    if (m_eState != ADJ_IDLE) {
        // We don't want to recalculate a running transition
        return;
    }

    DeckAttributes* fromDeck = getFromDeck();
    if (!fromDeck) {
        return;
    }
    calculateTransition(fromDeck, getOtherDeck(fromDeck), false);
}

void AutoDJProcessor::playlistFirstTrackChanged() {
    if constexpr (sDebug) {
        qDebug() << this << "playlistFirstTrackChanged";
    }
    if (m_eState != ADJ_DISABLED) {
        DeckAttributes* pLeftDeck = getLeftDeck();
        DeckAttributes* pRightDeck = getRightDeck();

        if (!pLeftDeck->isPlaying()) {
            loadNextTrackFromQueue(*pLeftDeck);
        } else if (!pRightDeck->isPlaying()) {
            loadNextTrackFromQueue(*pRightDeck);
        }
    }
}

void AutoDJProcessor::setTransitionTime(int time) {
    if constexpr (sDebug) {
        qDebug() << this << "setTransitionTime" << time;
    }

    // Update the transition time first.
    m_pConfig->set(ConfigKey(kConfigKey, kTransitionPreferenceName),
                   ConfigValue(time));
    m_transitionTime = time;

    // Then re-calculate fade thresholds for the decks.
    if (m_eState == ADJ_IDLE) {
        DeckAttributes* pLeftDeck = getLeftDeck();
        DeckAttributes* pRightDeck = getRightDeck();
        if (!pLeftDeck || !pRightDeck) {
            // User has changed the orientation, disable Auto DJ
            toggleAutoDJ(false);
            emit autoDJError(ADJ_NOT_TWO_DECKS);
            return;
        }
        if (pLeftDeck->isPlaying()) {
            calculateTransition(pLeftDeck, pRightDeck, false);
        }
        if (pRightDeck->isPlaying()) {
            calculateTransition(pRightDeck, pLeftDeck, false);
        }
    }
}

void AutoDJProcessor::setTransitionMode(TransitionMode newMode) {
    m_pConfig->set(ConfigKey(kConfigKey, kTransitionModePreferenceName),
            ConfigValue(static_cast<int>(newMode)));
    m_transitionMode = newMode;

    if (m_eState != ADJ_IDLE) {
        // We don't want to recalculate a running transition
        return;
    }

    // Then re-calculate fade thresholds for the decks.
    DeckAttributes* pLeftDeck = getLeftDeck();
    DeckAttributes* pRightDeck = getRightDeck();

    if (!pLeftDeck || !pRightDeck) {
        // User has changed the orientation, disable Auto DJ
        toggleAutoDJ(false);
        emit autoDJError(ADJ_NOT_TWO_DECKS);
        return;
    }

    if (pLeftDeck->isPlaying() && !pRightDeck->isPlaying()) {
        calculateTransition(pLeftDeck, pRightDeck, true);
        if (pRightDeck->startPos != kKeepPosition) {
            // Note: this seek will trigger the playerPositionChanged slot
            // which may calls the calculateTransition() again without seek = true;
            pRightDeck->setPlayPosition(pRightDeck->startPos);
        }
    } else if (pRightDeck->isPlaying() && pLeftDeck->isPlaying()) {
        calculateTransition(pRightDeck, pLeftDeck, true);
        if (pLeftDeck->startPos != kKeepPosition) {
            // Note: this seek will trigger the playerPositionChanged slot
            // which may calls the calculateTransition() again without seek = true;
            pLeftDeck->setPlayPosition(pLeftDeck->startPos);
        }
    } else {
        // user has manually started the other deck or stopped both.
        // don't know what to do.
    }
}

DeckAttributes* AutoDJProcessor::getLeftDeck() {
    // find first left deck
    for (const auto& pDeck : m_decks) {
        if (pDeck->isLeft()) {
            return pDeck.get();
        }
    }
    return nullptr;
}

DeckAttributes* AutoDJProcessor::getRightDeck() {
    // find first right deck
    for (const auto& pDeck : m_decks) {
        if (pDeck->isRight()) {
            return pDeck.get();
        }
    }
    return nullptr;
}

DeckAttributes* AutoDJProcessor::getOtherDeck(
        const DeckAttributes* pThisDeck) {
    if (pThisDeck->isLeft()) {
        return getRightDeck();
    }
    if (pThisDeck->isRight()) {
        return getLeftDeck();
    }
    return nullptr;
}

DeckAttributes* AutoDJProcessor::getFromDeck() {
    for (const auto& pDeck : m_decks) {
        if (pDeck->isFromDeck) {
            return pDeck.get();
        }
    }
    return nullptr;
}

bool AutoDJProcessor::nextTrackLoaded() {
    if (m_eState == ADJ_DISABLED) {
        // AutoDJ always loads the top track (again) if enabled
        return false;
    }

    DeckAttributes* pLeftDeck = getLeftDeck();
    DeckAttributes* pRightDeck = getRightDeck();
    if (!pLeftDeck || !pRightDeck) {
        return false;
    }

    bool leftDeckPlaying = pLeftDeck->isPlaying();
    bool rightDeckPlaying = pRightDeck->isPlaying();

    // Calculate idle deck
    TrackPointer loadedTrack;
    if (leftDeckPlaying && !rightDeckPlaying) {
        loadedTrack = pRightDeck->getLoadedTrack();
    } else if (!leftDeckPlaying && rightDeckPlaying) {
        loadedTrack = pLeftDeck->getLoadedTrack();
    } else if (getCrossfader() < 0.0) {
        loadedTrack = pRightDeck->getLoadedTrack();
    } else {
        loadedTrack = pLeftDeck->getLoadedTrack();
    }

    return loadedTrack == getNextTrackFromQueue();
}
