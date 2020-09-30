#include "library/autodj/autodjprocessor.h"

#include "control/controlproxy.h"
#include "control/controlpushbutton.h"
#include "engine/engine.h"
#include "library/trackcollection.h"
#include "mixer/basetrackplayer.h"
#include "mixer/playermanager.h"
#include "track/track.h"
#include "util/math.h"

#define kConfigKey "[Auto DJ]"
namespace {
const char* kTransitionPreferenceName = "Transition";
const char* kTransitionModePreferenceName = "TransitionMode";
const double kTransitionPreferenceDefault = 10.0;
const double kKeepPosition = -1.0;

const mixxx::audio::ChannelCount kChannelCount = mixxx::kEngineChannelCount;

static const bool sDebug = false;
} // anonymous namespace

DeckAttributes::DeckAttributes(int index,
        BaseTrackPlayer* pPlayer,
        EngineChannel::ChannelOrientation orientation)
        : index(index),
          group(pPlayer->getGroup()),
          startPos(kKeepPosition),
          fadeBeginPos(1.0),
          fadeEndPos(1.0),
          isFromDeck(false),
          loading(false),
          m_orientation(orientation),
          m_playPos(group, "playposition"),
          m_play(group, "play"),
          m_repeat(group, "repeat"),
          m_introStartPos(group, "intro_start_position"),
          m_introEndPos(group, "intro_end_position"),
          m_outroStartPos(group, "outro_start_position"),
          m_outroEndPos(group, "outro_end_position"),
          m_trackSamples(group, "track_samples"),
          m_sampleRate(group, "track_samplerate"),
          m_rateRatio(group, "rate_ratio"),
          m_pPlayer(pPlayer) {
    connect(m_pPlayer, &BaseTrackPlayer::newTrackLoaded,
            this, &DeckAttributes::slotTrackLoaded);
    connect(m_pPlayer, &BaseTrackPlayer::loadingTrack,
            this, &DeckAttributes::slotLoadingTrack);
    connect(m_pPlayer, &BaseTrackPlayer::playerEmpty,
            this, &DeckAttributes::slotPlayerEmpty);
    m_playPos.connectValueChanged(this, &DeckAttributes::slotPlayPosChanged);
    m_play.connectValueChanged(this, &DeckAttributes::slotPlayChanged);
    m_introStartPos.connectValueChanged(this, &DeckAttributes::slotIntroStartPositionChanged);
    m_introEndPos.connectValueChanged(this, &DeckAttributes::slotIntroEndPositionChanged);
    m_outroStartPos.connectValueChanged(this, &DeckAttributes::slotOutroStartPositionChanged);
    m_outroEndPos.connectValueChanged(this, &DeckAttributes::slotOutroEndPositionChanged);
    m_rateRatio.connectValueChanged(this, &DeckAttributes::slotRateChanged);
}

DeckAttributes::~DeckAttributes() {
}

void DeckAttributes::slotPlayChanged(double v) {
    emit playChanged(this, v > 0.0);
}

void DeckAttributes::slotPlayPosChanged(double v) {
    emit playPositionChanged(this, v);
}

void DeckAttributes::slotIntroStartPositionChanged(double v) {
    emit introStartPositionChanged(this, v);
}

void DeckAttributes::slotIntroEndPositionChanged(double v) {
    emit introEndPositionChanged(this, v);
}

void DeckAttributes::slotOutroStartPositionChanged(double v) {
    emit outroStartPositionChanged(this, v);
}

void DeckAttributes::slotOutroEndPositionChanged(double v) {
    emit outroEndPositionChanged(this, v);
}

void DeckAttributes::slotTrackLoaded(TrackPointer pTrack) {
    emit trackLoaded(this, pTrack);
}

void DeckAttributes::slotLoadingTrack(TrackPointer pNewTrack, TrackPointer pOldTrack) {
    //qDebug() << "DeckAttributes::slotLoadingTrack";
    emit loadingTrack(this, pNewTrack, pOldTrack);
}

void DeckAttributes::slotPlayerEmpty() {
    emit playerEmpty(this);
}

void DeckAttributes::slotRateChanged(double v) {
    Q_UNUSED(v);
    emit rateChanged(this);
}

TrackPointer DeckAttributes::getLoadedTrack() const {
    return m_pPlayer != NULL ? m_pPlayer->getLoadedTrack() : TrackPointer();
}

AutoDJProcessor::AutoDJProcessor(
        QObject* pParent,
        UserSettingsPointer pConfig,
        PlayerManagerInterface* pPlayerManager,
        TrackCollectionManager* pTrackCollectionManager,
        int iAutoDJPlaylistId)
        : QObject(pParent),
          m_pConfig(pConfig),
          m_pPlayerManager(pPlayerManager),
          m_pAutoDJTableModel(NULL),
          m_eState(ADJ_DISABLED),
          m_transitionProgress(0.0),
          m_transitionTime(kTransitionPreferenceDefault) {
    m_pAutoDJTableModel = new PlaylistTableModel(this, pTrackCollectionManager,
                                                 "mixxx.db.model.autodj");
    m_pAutoDJTableModel->setTableModel(iAutoDJPlaylistId);

    m_pShufflePlaylist = new ControlPushButton(
            ConfigKey("[AutoDJ]", "shuffle_playlist"));
    connect(m_pShufflePlaylist, &ControlPushButton::valueChanged,
            this, &AutoDJProcessor::controlShuffle);

    m_pSkipNext = new ControlPushButton(
            ConfigKey("[AutoDJ]", "skip_next"));
    connect(m_pSkipNext, &ControlObject::valueChanged,
            this, &AutoDJProcessor::controlSkipNext);

    m_pFadeNow = new ControlPushButton(
            ConfigKey("[AutoDJ]", "fade_now"));
    connect(m_pFadeNow, &ControlObject::valueChanged,
            this, &AutoDJProcessor::controlFadeNow);

    m_pEnabledAutoDJ = new ControlPushButton(
            ConfigKey("[AutoDJ]", "enabled"));
    m_pEnabledAutoDJ->setButtonMode(ControlPushButton::TOGGLE);
    connect(m_pEnabledAutoDJ, &ControlObject::valueChanged,
            this, &AutoDJProcessor::controlEnable);

    // TODO(rryan) listen to signals from PlayerManager and add/remove as decks
    // are created.
    for (unsigned int i = 0; i < pPlayerManager->numberOfDecks(); ++i) {
        QString group = PlayerManager::groupForDeck(i);
        BaseTrackPlayer* pPlayer = pPlayerManager->getPlayer(group);
        // Shouldn't be possible.
        if (pPlayer == NULL) {
            qWarning() << "PROGRAMMING ERROR deck does not exist" << i;
            continue;
        }
        EngineChannel::ChannelOrientation orientation =
                (i % 2 == 0) ? EngineChannel::LEFT : EngineChannel::RIGHT;
        m_decks.append(new DeckAttributes(i, pPlayer, orientation));
    }
    // Auto-DJ needs at least two decks
    DEBUG_ASSERT(m_decks.length() > 1);

    m_pCOCrossfader = new ControlProxy("[Master]", "crossfader");
    m_pCOCrossfaderReverse = new ControlProxy("[Mixer Profile]", "xFaderReverse");

    QString str_autoDjTransition = m_pConfig->getValueString(
            ConfigKey(kConfigKey, kTransitionPreferenceName));
    if (!str_autoDjTransition.isEmpty()) {
        m_transitionTime = str_autoDjTransition.toDouble();
    }

    int configuredTransitionMode = m_pConfig->getValue(
            ConfigKey(kConfigKey, kTransitionModePreferenceName),
            static_cast<int>(TransitionMode::FullIntroOutro));
    m_transitionMode = static_cast<TransitionMode>(configuredTransitionMode);
}

AutoDJProcessor::~AutoDJProcessor() {
    qDeleteAll(m_decks);
    m_decks.clear();
    delete m_pCOCrossfader;
    delete m_pCOCrossfaderReverse;

    delete m_pSkipNext;
    delete m_pShufflePlaylist;
    delete m_pEnabledAutoDJ;
    delete m_pFadeNow;

    delete m_pAutoDJTableModel;
}

double AutoDJProcessor::getCrossfader() const {
    if (m_pCOCrossfaderReverse->toBool()) {
        return m_pCOCrossfader->get() * -1.0;
    }
    return m_pCOCrossfader->get();
}

void AutoDJProcessor::setCrossfader(double value) {
    if (m_pCOCrossfaderReverse->toBool()) {
        value *= -1.0;
    }
    m_pCOCrossfader->set(value);
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
    // Auto-DJ needs at least two decks
    VERIFY_OR_DEBUG_ASSERT(m_decks.length() > 1) {
        return;
    }
    if (m_eState != ADJ_IDLE) {
        // we cannot fade if AutoDj is disabled or already fading
        return;
    }

    double crossfader = getCrossfader();
    DeckAttributes* pLeftDeck = m_decks[0];
    DeckAttributes* pRightDeck = m_decks[1];
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

    const double fromDeckEndSecond = getEndSecond(pFromDeck);
    const double toDeckEndSecond = getEndSecond(pToDeck);
    // Since the end position is measured in seconds from 0:00 it is also
    // the track duration. Use this alias for better readability.
    const double fromDeckDuration = fromDeckEndSecond;
    const double toDeckDuration = toDeckEndSecond;
    // playPosition() is in the range of 0..1
    const double fromDeckCurrentSecond = fromDeckDuration * pFromDeck->playPosition();
    const double toDeckCurrentSecond = toDeckDuration * pToDeck->playPosition();

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
        double outroEnd = getOutroEndSecond(pFromDeck);
        double introEnd = getIntroEndSecond(pToDeck);
        double introStart = getIntroStartSecond(pToDeck);
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

    double timeUntilEndOfFromTrack = fromDeckEndSecond - fromDeckCurrentSecond;
    fadeTime = math_min(fadeTime, timeUntilEndOfFromTrack);
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
        return ADJ_IS_INACTIVE;
    }

    // Auto-DJ needs at least two decks
    VERIFY_OR_DEBUG_ASSERT(m_decks.length() > 1) {
        return ADJ_NOT_TWO_DECKS;
    }

    // Load the next song from the queue.
    DeckAttributes& leftDeck = *m_decks[0];
    DeckAttributes& rightDeck = *m_decks[1];
    if (!leftDeck.isPlaying()) {
        removeLoadedTrackFromTopOfQueue(leftDeck);
        loadNextTrackFromQueue(leftDeck);
    } else if (!rightDeck.isPlaying()) {
        removeLoadedTrackFromTopOfQueue(rightDeck);
        loadNextTrackFromQueue(rightDeck);
    } else {
        // If both decks are playing remove next track in playlist
        TrackId nextId = m_pAutoDJTableModel->getTrackId(m_pAutoDJTableModel->index(0, 0));
        TrackId leftId = leftDeck.getLoadedTrack()->getId();
        TrackId rightId = rightDeck.getLoadedTrack()->getId();
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
    DeckAttributes* deck1 = m_decks[0];
    DeckAttributes* deck2 = m_decks[1];
    VERIFY_OR_DEBUG_ASSERT(deck1 && deck2) {
        return ADJ_NOT_TWO_DECKS;
    }

    bool deck1Playing = deck1->isPlaying();
    bool deck2Playing = deck2->isPlaying();

    if (enable) {  // Enable Auto DJ
        if (deck1Playing && deck2Playing) {
            qDebug() << "One deck must be stopped before enabling Auto DJ mode";
            // Keep the current state.
            emitAutoDJStateChanged(m_eState);
            return ADJ_BOTH_DECKS_PLAYING;
        }

        // TODO: This is a total bandaid for making Auto DJ work with decks 3
        // and 4.  We should design a nicer way to handle this.
        for (int i = 2; i < m_decks.length(); ++i) {
            if (m_decks[i] != NULL && m_decks[i]->isPlaying()) {
                return ADJ_DECKS_3_4_PLAYING;
            }
        }

        // Never load the same track if it is already playing
        if (deck1Playing) {
            removeLoadedTrackFromTopOfQueue(*deck1);
        } else if (deck2Playing) {
            removeLoadedTrackFromTopOfQueue(*deck2);
        } else {
            // If the first track is already cued at a position in the first
            // 2/3 in on of the Auto DJ decks, start it.
            // If the track is paused at a later position, it is probably too
            // close to the end. In this case it is loaded again at the stored
            // cue point.
            if (deck1->playPosition() < 0.66 &&
                    removeLoadedTrackFromTopOfQueue(*deck1)) {
                deck1->play();
                deck1Playing = true;
            } else if (deck2->playPosition() < 0.66 &&
                    removeLoadedTrackFromTopOfQueue(*deck2)) {
                deck2->play();
                deck2Playing = true;
            }
        }


        TrackPointer nextTrack = getNextTrackFromQueue();
        if (!nextTrack) {
            qDebug() << "Queue is empty now";
            if (m_pEnabledAutoDJ->get() != 0.0) {
                m_pEnabledAutoDJ->set(0.0);
            }
            emitAutoDJStateChanged(m_eState);
            return ADJ_QUEUE_EMPTY;
        }

        // Track is available so GO
        if (m_pEnabledAutoDJ->get() != 1.0) {
            m_pEnabledAutoDJ->set(1.0);
        }
        qDebug() << "Auto DJ enabled";

        m_pCOCrossfader->connectValueChanged(this, &AutoDJProcessor::crossfaderChanged);

        connect(deck1,
                &DeckAttributes::playPositionChanged,
                this,
                &AutoDJProcessor::playerPositionChanged);
        connect(deck2,
                &DeckAttributes::playPositionChanged,
                this,
                &AutoDJProcessor::playerPositionChanged);

        connect(deck1,
                &DeckAttributes::playChanged,
                this,
                &AutoDJProcessor::playerPlayChanged);
        connect(deck2,
                &DeckAttributes::playChanged,
                this,
                &AutoDJProcessor::playerPlayChanged);

        connect(deck1,
                &DeckAttributes::introStartPositionChanged,
                this,
                &AutoDJProcessor::playerIntroStartChanged);
        connect(deck2,
                &DeckAttributes::introStartPositionChanged,
                this,
                &AutoDJProcessor::playerIntroStartChanged);

        connect(deck1,
                &DeckAttributes::introEndPositionChanged,
                this,
                &AutoDJProcessor::playerIntroEndChanged);
        connect(deck2,
                &DeckAttributes::introEndPositionChanged,
                this,
                &AutoDJProcessor::playerIntroEndChanged);

        connect(deck1,
                &DeckAttributes::outroStartPositionChanged,
                this,
                &AutoDJProcessor::playerOutroStartChanged);
        connect(deck2,
                &DeckAttributes::outroStartPositionChanged,
                this,
                &AutoDJProcessor::playerOutroStartChanged);

        connect(deck1,
                &DeckAttributes::outroEndPositionChanged,
                this,
                &AutoDJProcessor::playerOutroEndChanged);
        connect(deck2,
                &DeckAttributes::outroEndPositionChanged,
                this,
                &AutoDJProcessor::playerOutroEndChanged);

        connect(deck1,
                &DeckAttributes::trackLoaded,
                this,
                &AutoDJProcessor::playerTrackLoaded);
        connect(deck2,
                &DeckAttributes::trackLoaded,
                this,
                &AutoDJProcessor::playerTrackLoaded);

        connect(deck1,
                &DeckAttributes::loadingTrack,
                this,
                &AutoDJProcessor::playerLoadingTrack);
        connect(deck2,
                &DeckAttributes::loadingTrack,
                this,
                &AutoDJProcessor::playerLoadingTrack);

        connect(deck1,
                &DeckAttributes::playerEmpty,
                this,
                &AutoDJProcessor::playerEmpty);
        connect(deck2,
                &DeckAttributes::playerEmpty,
                this,
                &AutoDJProcessor::playerEmpty);

        connect(deck1,
                &DeckAttributes::rateChanged,
                this,
                &AutoDJProcessor::playerRateChanged);
        connect(deck2,
                &DeckAttributes::rateChanged,
                this,
                &AutoDJProcessor::playerRateChanged);

        if (!deck1Playing && !deck2Playing) {
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
            emitLoadTrackToPlayer(nextTrack, deck1->group, true);
        } else {
            // One of the two decks is playing. Switch into IDLE mode and wait
            // until the playing deck crosses posThreshold to start fading.
            m_eState = ADJ_IDLE;
            if (deck1Playing) {
                // Load track into the right deck.
                emitLoadTrackToPlayer(nextTrack, deck2->group, false);
                // Move crossfader to the left.
                setCrossfader(-1.0);
            } else {
                // Load track into the left deck.
                emitLoadTrackToPlayer(nextTrack, deck1->group, false);
                // Move crossfader to the right.
                setCrossfader(1.0);
            }
        }
        emitAutoDJStateChanged(m_eState);
    } else {  // Disable Auto DJ
        if (m_pEnabledAutoDJ->get() != 0.0) {
            m_pEnabledAutoDJ->set(0.0);
        }
        qDebug() << "Auto DJ disabled";
        m_eState = ADJ_DISABLED;
        disconnect(m_pCOCrossfader,
                &ControlProxy::valueChanged,
                this,
                &AutoDJProcessor::crossfaderChanged);
        deck1->disconnect(this);
        deck2->disconnect(this);
        m_pCOCrossfader->set(0);
        emitAutoDJStateChanged(m_eState);
    }
    return ADJ_OK;
}

void AutoDJProcessor::controlEnable(double value) {
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

void AutoDJProcessor::crossfaderChanged(double value) {
    if (m_eState == ADJ_IDLE) {
        // The user is changing the crossfader manually. If the user has
        // moved it all the way to the other side, make the deck faded away
        // from the new "to deck" by loading the next track into it.
        DeckAttributes* fromDeck = getFromDeck();
        VERIFY_OR_DEBUG_ASSERT(fromDeck) {
            // we have always a from deck in case of state IDLE
            return;
        }

        DeckAttributes* toDeck = getOtherDeck(fromDeck);
        VERIFY_OR_DEBUG_ASSERT(toDeck) {
            // we have always a from deck in case of state IDLE
            return;
        }

        double crossfaderPosition = value * (m_pCOCrossfaderReverse->toBool() ? -1 : 1);

        if ((crossfaderPosition == 1.0 && fromDeck->isLeft()) ||       // crossfader right
                (crossfaderPosition == -1.0 && fromDeck->isRight())) { // crossfader left
            if (!toDeck->isPlaying()) {
                // Re-cue the track if the user has seeked it to the very end
                if (toDeck->playPosition() >= toDeck->fadeBeginPos) {
                    toDeck->setPlayPosition(toDeck->startPos);
                }
                toDeck->play();
            }
            fromDeck->stop();

            // Now that we have started the other deck playing, remove the track
            // that was "on deck" from the top of the queue.
            removeLoadedTrackFromTopOfQueue(*toDeck);
            loadNextTrackFromQueue(*fromDeck);
        }
    }
}

void AutoDJProcessor::playerPositionChanged(DeckAttributes* pAttributes,
                                            double thisPlayPosition) {
    if (sDebug) {
        //qDebug() << this << "playerPositionChanged" << pAttributes->group
        //         << thisPlayPosition;
    }

    // Auto-DJ needs at least two decks
    VERIFY_OR_DEBUG_ASSERT(m_decks.length() > 1) {
        return;
    }

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
            // sure our thresholds are configured (by calling calculateFadeThresholds
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
    if (thisPlayPosition >= thisDeck->fadeBeginPos && thisDeck->isFromDeck) {
        if (m_eState == ADJ_IDLE) {
            if (thisDeckPlaying || thisPlayPosition >= 1.0) {
                // Set the state as FADING.
                m_eState = thisDeck->isLeft() ? ADJ_LEFT_FADING : ADJ_RIGHT_FADING;
                m_transitionProgress = 0.0;
                emitAutoDJStateChanged(m_eState);

                if (!otherDeckPlaying) {
                    // Re-cue the track if the user has seeked it to the very end
                    if (otherDeck->playPosition() >= otherDeck->fadeBeginPos) {
                        otherDeck->setPlayPosition(otherDeck->startPos);
                    }
                    otherDeck->play();
                }

                if (thisDeck->fadeBeginPos >= thisDeck->fadeEndPos) {
                    setCrossfader(thisDeck->isLeft() ? 1.0 : -1.0);
                }

                // Now that we have started the other deck playing, remove the track
                // that was "on deck" from the top of the queue.
                // Note: This is a DB call and takes long.
                removeLoadedTrackFromTopOfQueue(*otherDeck);
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
        TrackPointer nextTrack = m_pAutoDJTableModel->getTrack(
            m_pAutoDJTableModel->index(0, 0));

        if (nextTrack) {
            if (nextTrack->checkFileExists()) {
                return nextTrack;
            } else {
                // Remove missing song from auto DJ playlist.
                m_pAutoDJTableModel->removeTrack(
                        m_pAutoDJTableModel->index(0, 0));
            }
        } else {
            // We're out of tracks. Return the null TrackPointer.
            return nextTrack;
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
    if (sDebug) {
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

    if (playing && !otherDeck->isPlaying()) {
        // In case both decks were stopped and now this one just started, make
        // this deck the "from deck".
        calculateTransition(thisDeck, getOtherDeck(thisDeck), false);
    }

    // If the user manually pressed play on the "to deck" before fading, for
    // example to adjust the intro/outro cues, and lets the deck play until the
    // end, seek back to the start point instead of keeping the deck stopped at
    // the end.
    if (!playing && thisDeck->playPosition() >= 1.0 && !thisDeck->isFromDeck) {
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

void AutoDJProcessor::playerIntroStartChanged(DeckAttributes* pAttributes, double position) {
    if (sDebug) {
        qDebug() << this << "playerIntroStartChanged" << pAttributes->group << position;
    }
    // nothing to do, because we want not to re-cue the toDeck and the from
    // Deck has already passed the intro
}

void AutoDJProcessor::playerIntroEndChanged(DeckAttributes* pAttributes, double position) {
    if (sDebug) {
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
    if (sDebug) {
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
    if (sDebug) {
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

double AutoDJProcessor::getIntroStartSecond(DeckAttributes* pDeck) {
    double introStartSample = pDeck->introStartPosition();
    if (introStartSample == Cue::kNoPosition) {
        return getFirstSoundSecond(pDeck);
    }
    return samplePositionToSeconds(introStartSample, pDeck);
}

double AutoDJProcessor::getIntroEndSecond(DeckAttributes* pDeck) {
    double introEndSample = pDeck->introEndPosition();
    if (introEndSample == Cue::kNoPosition) {
        // Assume a zero length intro if introEnd is not set.
        // The introStart is automatically placed by AnalyzerSilence, so use
        // that as a fallback if the user has not placed outroStart. If it has
        // not been placed, getIntroStartPosition will return 0:00.
        return getIntroStartSecond(pDeck);
    }
    return samplePositionToSeconds(introEndSample, pDeck);
}

double AutoDJProcessor::getOutroStartSecond(DeckAttributes* pDeck) {
    double outroStartSample = pDeck->outroStartPosition();
    if (outroStartSample == Cue::kNoPosition) {
        // Assume a zero length outro if outroStart is not set.
        // The outroEnd is automatically placed by AnalyzerSilence, so use
        // that as a fallback if the user has not placed outroStart. If it has
        // not been placed, getOutroEndPosition will return the end of the track.
        return getOutroEndSecond(pDeck);
    }
    return samplePositionToSeconds(outroStartSample, pDeck);
}

double AutoDJProcessor::getOutroEndSecond(DeckAttributes* pDeck) {
    double outroEndSample = pDeck->outroEndPosition();
    if (outroEndSample == Cue::kNoPosition) {
        return getLastSoundSecond(pDeck);
    }
    return samplePositionToSeconds(outroEndSample, pDeck);;
}

double AutoDJProcessor::getFirstSoundSecond(DeckAttributes* pDeck) {
    TrackPointer pTrack = pDeck->getLoadedTrack();
    if (!pTrack) {
        return 0.0;
    }

    CuePointer pFromTrackAudibleSound = pTrack->findCueByType(mixxx::CueType::AudibleSound);
    if (pFromTrackAudibleSound) {
        double firstSound = pFromTrackAudibleSound->getPosition();
        if (firstSound > 0.0) {
            return samplePositionToSeconds(firstSound, pDeck);
        }
    }
    return 0.0;
}

double AutoDJProcessor::getLastSoundSecond(DeckAttributes* pDeck) {
    TrackPointer pTrack = pDeck->getLoadedTrack();
    if (!pTrack) {
        return 0.0;
    }

    CuePointer pFromTrackAudibleSound = pTrack->findCueByType(mixxx::CueType::AudibleSound);
    if (pFromTrackAudibleSound && pFromTrackAudibleSound->getLength() > 0) {
        double lastSound = pFromTrackAudibleSound->getEndPosition();
        if (lastSound > 0) {
            return samplePositionToSeconds(lastSound, pDeck);
        }
    }
    return getEndSecond(pDeck);
}

double AutoDJProcessor::getEndSecond(DeckAttributes* pDeck) {
    TrackPointer pTrack = pDeck->getLoadedTrack();
    if (!pTrack) {
        return 0.0;
    }

    double endSamplePosition = pDeck->trackSamples();
    return samplePositionToSeconds(endSamplePosition, pDeck);
}

double AutoDJProcessor::samplePositionToSeconds(double samplePosition, DeckAttributes* pDeck) {
    samplePosition /= kChannelCount;
    double sampleRate = pDeck->sampleRate();
    if (sampleRate <= 0.0) {
        return 0.0;
    }
    return samplePosition / sampleRate / pDeck->rateRatio();
}

void AutoDJProcessor::calculateTransition(DeckAttributes* pFromDeck,
        DeckAttributes* pToDeck,
        bool seekToStartPoint) {
    if (pFromDeck == nullptr || pToDeck == nullptr) {
        return;
    }
    if (pFromDeck->loading || pToDeck->loading) {
        // don't use halve new halve old data during
        // changing of tracks
        return;
    }

    qDebug() << "player" << pFromDeck->group << "calculateTransition()";

    // We require ADJ_IDLE to prevent changing the thresholds in the middle of a
    // fade.
    VERIFY_OR_DEBUG_ASSERT(m_eState == ADJ_IDLE) {
        return;
    }

    const double fromDeckEndPosition = getEndSecond(pFromDeck);
    const double toDeckEndPosition = getEndSecond(pToDeck);
    // Since the end position is measured in seconds from 0:00 it is also
    // the track duration. Use this alias for better readability.
    const double fromDeckDuration = fromDeckEndPosition;
    const double toDeckDuration = toDeckEndPosition;

    VERIFY_OR_DEBUG_ASSERT(fromDeckDuration > 0) {
        // Playing Track has no duration. This should not happen, because short
        // tracks are skipped after load. Play ToDeck immediately.
        pFromDeck->fadeBeginPos = 0;
        pFromDeck->fadeEndPos = 0;
        pToDeck->startPos = kKeepPosition;
        return;
    }
    if (toDeckDuration <= 0) {
        // Playing Track has no duration. This should not happen, because short
        // tracks are skipped after load.
        loadNextTrackFromQueue(*pToDeck, false);
        return;
    }

    // Within this function, the outro refers to the outro of the currently
    // playing track and the intro refers to the intro of the next track.

    double outroEnd = getOutroEndSecond(pFromDeck);
    double outroStart = getOutroStartSecond(pFromDeck);
    const double fromDeckPosition = fromDeckDuration * pFromDeck->playPosition();

    VERIFY_OR_DEBUG_ASSERT(outroEnd <= fromDeckEndPosition) {
        outroEnd = fromDeckEndPosition;
    }

    if (fromDeckPosition > outroStart) {
        // We have already passed outroStart
        // This can happen if we have just enabled auto DJ
        outroStart = fromDeckPosition;
        if (fromDeckPosition > outroEnd) {
            outroEnd = math_min(outroStart + fabs(m_transitionTime), fromDeckEndPosition);
        }
    }
    double outroLength = outroEnd - outroStart;

    double toDeckPositionSeconds = toDeckDuration * pToDeck->playPosition();
    // Store here a possible fadeBeginPos for the transition after next
    // This is used to check if it will be possible or a re-cue is required.
    // here it is done for FullIntroOutro and FadeAtOutroStart.
    // It is adjusted below for the other modes.
    pToDeck->fadeBeginPos = getOutroStartSecond(pToDeck);
    pToDeck->fadeEndPos = getOutroEndSecond(pToDeck);

    double introStart;
    if (seekToStartPoint || toDeckPositionSeconds >= pToDeck->fadeBeginPos) {
        // toDeckPosition >= pToDeck->fadeBeginPos happens when the
        // user has seeked or played the to track behind fadeBeginPos of
        // the fade after the next.
        // In this case we recue the track just before the transition.
        introStart = getIntroStartSecond(pToDeck);
    } else {
        introStart = toDeckPositionSeconds;
    }

    double introEnd = getIntroEndSecond(pToDeck);
    if (introEnd < introStart) {
        // introEnd is invalid. Assume a zero length intro.
        // The introStart is automatically placed by AnalyzerSilence, so use
        // that as a fallback if the user has not placed introEnd.
        introEnd = introStart;
    }

    double introLength = introEnd - introStart;

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
            const double transitionEnd = introStart + transitionLength;
            if (transitionEnd > pToDeck->fadeBeginPos) {
                // End intro before next outro starts
                transitionLength = pToDeck->fadeBeginPos - introStart;
                VERIFY_OR_DEBUG_ASSERT(transitionLength > 0) {
                    // We seek to intro start above in this case so this never happens
                    transitionLength = 1;
                }
            }
            pFromDeck->fadeBeginPos = outroEnd - transitionLength;
            pFromDeck->fadeEndPos = outroEnd;
            pToDeck->startPos = introStart;
        } else {
            useFixedFadeTime(pFromDeck, pToDeck, fromDeckPosition, outroEnd, introStart);
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
            const double transitionEnd = introStart + transitionLength;
            if (transitionEnd > pToDeck->fadeBeginPos) {
                // End intro before next outro starts
                transitionLength = pToDeck->fadeBeginPos - introStart;
                VERIFY_OR_DEBUG_ASSERT(transitionLength > 0) {
                    // We seek to intro start above in this case so this never happens
                    transitionLength = 1;
                }
            }
            pFromDeck->fadeBeginPos = outroStart;
            pFromDeck->fadeEndPos = outroStart + transitionLength;
            pToDeck->startPos = introStart;
        } else if (introLength > 0) {
            transitionLength = introLength;
            pFromDeck->fadeBeginPos = outroEnd - transitionLength;
            pFromDeck->fadeEndPos = outroEnd;
            pToDeck->startPos = introStart;
        } else {
            useFixedFadeTime(pFromDeck, pToDeck, fromDeckPosition, outroEnd, introStart);
        }
    } break;
    case TransitionMode::FixedSkipSilence: {
        double startPoint;
        pToDeck->fadeBeginPos = getLastSoundSecond(pToDeck);
        if (seekToStartPoint || toDeckPositionSeconds >= pToDeck->fadeBeginPos) {
            // toDeckPosition >= pToDeck->fadeBeginPos happens when the
            // user has seeked or played the to track behind fadeBeginPos of
            // the fade after the next.
            // In this case we recue the track just before the transition.
            startPoint = getFirstSoundSecond(pToDeck);
        } else {
            startPoint = toDeckPositionSeconds;
        }
        useFixedFadeTime(pFromDeck, pToDeck, fromDeckPosition, getLastSoundSecond(pFromDeck), startPoint);
    } break;
    case TransitionMode::FixedFullTrack:
    default: {
        double startPoint;
        pToDeck->fadeBeginPos = toDeckEndPosition;
        if (seekToStartPoint || toDeckPositionSeconds >= pToDeck->fadeBeginPos) {
            // toDeckPosition >= pToDeck->fadeBeginPos happens when the
            // user has seeked or played the to track behind fadeBeginPos of
            // the fade after the next.
            // In this case we recue the track just before the transition.
            startPoint = 0.0;
        } else {
            startPoint = toDeckPositionSeconds;
        }
        useFixedFadeTime(pFromDeck, pToDeck, fromDeckPosition, fromDeckEndPosition, startPoint);
        }
    }

    // These are expected to be a fraction of the track length.
    pFromDeck->fadeBeginPos /= fromDeckDuration;
    pFromDeck->fadeEndPos /= fromDeckDuration;
    pToDeck->startPos /= toDeckDuration;
    pToDeck->fadeBeginPos /= toDeckDuration;
    pToDeck->fadeEndPos /= toDeckDuration;

    pFromDeck->isFromDeck = true;
    pToDeck->isFromDeck = false;

    VERIFY_OR_DEBUG_ASSERT(pFromDeck->fadeBeginPos <= 1) {
        pFromDeck->fadeBeginPos = 1;
    }

    if (sDebug) {
        qDebug() << this << "calculateTransition" << pFromDeck->group
                 << pFromDeck->fadeBeginPos << pFromDeck->fadeEndPos
                 << pToDeck->startPos;
    }
}

void AutoDJProcessor::useFixedFadeTime(
        DeckAttributes* pFromDeck,
        DeckAttributes* pToDeck,
        double fromDeckSecond,
        double fadeEndSecond,
        double toDeckStartSecond) {
    if (m_transitionTime > 0.0) {
        // Guard against the next track being too short. This transition must finish
        // before the next transition starts.
        double toDeckOutroStart = pToDeck->fadeBeginPos;
        if (pToDeck->fadeBeginPos >= pToDeck->fadeEndPos) {
            // no outro defined, the toDeck will also use the transition time
            toDeckOutroStart -= m_transitionTime;
        }
        if (toDeckOutroStart <= toDeckStartSecond) {
            // we are already too late
            // Check OutroEnd as alternative, which is for all transition mode
            // better than directly default to duration()
            double end = getOutroEndSecond(pToDeck);
            if (end <= toDeckStartSecond) {
                end = getEndSecond(pToDeck);
                VERIFY_OR_DEBUG_ASSERT(end > toDeckStartSecond) {
                    // as last resort move start point
                    // The caller makes sure that this never happens
                    toDeckStartSecond = end - 1;
                }
            }
            // use the remaining time for fading
            toDeckOutroStart = (end - toDeckStartSecond) / 2 + toDeckStartSecond;
        }
        double transitionTime = math_min(toDeckOutroStart - toDeckStartSecond,
                m_transitionTime);

        pFromDeck->fadeBeginPos = math_max(fadeEndSecond - transitionTime, fromDeckSecond);
        pFromDeck->fadeEndPos = fadeEndSecond;
        pToDeck->startPos = toDeckStartSecond;
    } else {
        pFromDeck->fadeBeginPos = fadeEndSecond;
        pFromDeck->fadeEndPos = fadeEndSecond;
        pToDeck->startPos = toDeckStartSecond + m_transitionTime;
    }
}

void AutoDJProcessor::playerTrackLoaded(DeckAttributes* pDeck, TrackPointer pTrack) {
    if (sDebug) {
        qDebug() << this << "playerTrackLoaded" << pDeck->group
                 << (pTrack ? pTrack->getLocation() : "(null)");
    }

    pDeck->loading = false;

    // Since the end position is measured in seconds from 0:00 it is also
    // the track duration.
    double duration = getEndSecond(pDeck);
    if (duration < 0.2) {
        qWarning() << "Skip track with" << duration << "Duration"
                   << pTrack->getLocation();
        // Remove Tack with duration smaller than two callbacks
        removeTrackFromTopOfQueue(pTrack);

        // Load the next track. If we are the first AutoDJ track
        // (ADJ_ENABLE_P1LOADED state) then play the track.
        loadNextTrackFromQueue(*pDeck, m_eState == ADJ_ENABLE_P1LOADED);
    } else if (m_eState == ADJ_IDLE) {
        // this deck has just changed the track so it becomes the toDeck
        DeckAttributes* fromDeck = getOtherDeck(pDeck);
        if (fromDeck) {
            // check if this deck has suitable alignment
            DeckAttributes* toDeck = getOtherDeck(fromDeck);
            if (toDeck == pDeck) {
                pDeck->startPos = kKeepPosition;
                calculateTransition(fromDeck, pDeck, true);
                if (pDeck->startPos != kKeepPosition) {
                    // Note: this seek will trigger the playerPositionChanged slot
                    // which may calls the calculateTransition() again without seek = true;
                    pDeck->setPlayPosition(pDeck->startPos);
                }
                // we are her in the relative domain 0..1
                if (!fromDeck->isPlaying() && fromDeck->playPosition() >= 1.0) {
                    // repeat a probably missed update
                    playerPositionChanged(fromDeck, 1.0);
                }
            }
        }
    }
}

void AutoDJProcessor::playerLoadingTrack(DeckAttributes* pDeck,
        TrackPointer pNewTrack, TrackPointer pOldTrack) {
    if (sDebug) {
        qDebug() << this << "playerLoadingTrack" << pDeck->group
                 << "new:"<< (pNewTrack ? pNewTrack->getLocation() : "(null)")
                 << "old:"<< (pOldTrack ? pOldTrack->getLocation() : "(null)");
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
        // this track seams to be undesired. Remove the bad track from the queue.
        removeTrackFromTopOfQueue(pOldTrack);

        // wait until the track is fully unloaded and the playerEmpty()
        // slot is called before load an alternative track.
    }
}

void AutoDJProcessor::playerEmpty(DeckAttributes* pDeck) {
    if (sDebug) {
        qDebug() << this << "playerEmpty()" << pDeck->group;
    }

    // The Deck has ejected a track and no new one is loaded
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
    if (sDebug) {
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

void AutoDJProcessor::setTransitionTime(int time) {
    if (sDebug) {
        qDebug() << this << "setTransitionTime" << time;
    }

    // Auto-DJ needs at least two decks
    VERIFY_OR_DEBUG_ASSERT(m_decks.length() > 1) {
        return;
    }

    // Update the transition time first.
    m_pConfig->set(ConfigKey(kConfigKey, kTransitionPreferenceName),
                   ConfigValue(time));
    m_transitionTime = time;

    // Then re-calculate fade thresholds for the decks.
    if (m_eState == ADJ_IDLE) {
        DeckAttributes& leftDeck = *m_decks[0];
        DeckAttributes& rightDeck = *m_decks[1];
        if (leftDeck.isPlaying()) {
            calculateTransition(&leftDeck, &rightDeck, false);
        }
        if (rightDeck.isPlaying()) {
            calculateTransition(&rightDeck, &leftDeck, false);
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
    DeckAttributes& leftDeck = *m_decks[0];
    DeckAttributes& rightDeck = *m_decks[1];

    if (leftDeck.isPlaying() && !rightDeck.isPlaying()) {
        calculateTransition(&leftDeck, &rightDeck, true);
        if (rightDeck.startPos != kKeepPosition) {
            // Note: this seek will trigger the playerPositionChanged slot
            // which may calls the calculateTransition() again without seek = true;
            rightDeck.setPlayPosition(rightDeck.startPos);
        }
    } else if (rightDeck.isPlaying() && !leftDeck.isPlaying()) {
        calculateTransition(&rightDeck, &leftDeck, true);
        if (leftDeck.startPos != kKeepPosition) {
            // Note: this seek will trigger the playerPositionChanged slot
            // which may calls the calculateTransition() again without seek = true;
            leftDeck.setPlayPosition(leftDeck.startPos);
        }
    } else {
        // user has manually started the other deck or stopped both.
        // don't know what to do.
    }
}

DeckAttributes* AutoDJProcessor::getOtherDeck(
        const DeckAttributes* pThisDeck) {
    if (pThisDeck->isLeft()) {
        // find first right deck
        foreach(DeckAttributes* pDeck, m_decks) {
            if (pDeck->isRight()) {
                return pDeck;
            }
        }
        return nullptr;
    }

    if (pThisDeck->isRight()) {
        // find first left deck
        foreach(DeckAttributes* pDeck, m_decks) {
            if (pDeck->isLeft()) {
                return pDeck;
            }
        }
        return nullptr;
    }
    return nullptr;
}

DeckAttributes* AutoDJProcessor::getFromDeck() {
    for (const auto& pDeck : m_decks) {
        if (pDeck->isFromDeck) {
            return pDeck;
        }
    }
    return nullptr;
}

bool AutoDJProcessor::nextTrackLoaded() {
    if (m_eState == ADJ_DISABLED) {
        // AutoDJ always loads the top track (again) if enabled
        return false;
    }

    DeckAttributes& leftDeck = *m_decks[0];
    DeckAttributes& rightDeck = *m_decks[1];
    bool leftDeckPlaying = leftDeck.isPlaying();
    bool rightDeckPlaying = rightDeck.isPlaying();


    // Calculate idle deck
    TrackPointer loadedTrack;
    if (leftDeckPlaying && !rightDeckPlaying) {
        loadedTrack = rightDeck.getLoadedTrack();
    } else if (!leftDeckPlaying && rightDeckPlaying) {
        loadedTrack = leftDeck.getLoadedTrack();
    } else if (getCrossfader() < 0.0) {
        loadedTrack = rightDeck.getLoadedTrack();
    } else {
        loadedTrack = leftDeck.getLoadedTrack();
    }

    return loadedTrack == getNextTrackFromQueue();
}
