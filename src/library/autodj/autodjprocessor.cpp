#include "library/autodj/autodjprocessor.h"

#include "library/trackcollection.h"
#include "control/controlpushbutton.h"
#include "control/controlproxy.h"
#include "engine/engine.h"
#include "util/math.h"
#include "mixer/playermanager.h"
#include "mixer/basetrackplayer.h"

#define kConfigKey "[Auto DJ]"
const char* kTransitionPreferenceName = "Transition";
const char* kTransitionModePreferenceName = "TransitionMode";
const double kTransitionPreferenceDefault = 10.0;
const double kKeepPosition = -1.0;

const mixxx::AudioSignal::ChannelCount kChannelCount = mixxx::kEngineChannelCount;

static const bool sDebug = true; // false;

DeckAttributes::DeckAttributes(int index,
                               BaseTrackPlayer* pPlayer,
                               EngineChannel::ChannelOrientation orientation)
        : index(index),
          group(pPlayer->getGroup()),
          startPos(kKeepPosition),
          fadeBeginPos(1.0),
          fadeEndPos(1.0),
          loading(false),
          m_orientation(orientation),
          m_playPos(group, "playposition"),
          m_play(group, "play"),
          m_repeat(group, "repeat"),
          m_introStartPos(group, "intro_start_position"),
          m_introEndPos(group, "intro_end_position"),
          m_outroStartPos(group, "outro_start_position"),
          m_outroEndPos(group, "outro_end_position"),
          m_sampleRate(group, "track_samplerate"),
          m_duration(group, "duration"),
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
}

DeckAttributes::~DeckAttributes() {
}

void DeckAttributes::slotPlayChanged(double v) {
    emit(playChanged(this, v > 0.0));
}

void DeckAttributes::slotPlayPosChanged(double v) {
    emit(playPositionChanged(this, v));
}

void DeckAttributes::slotIntroStartPositionChanged(double v) {
    emit(introStartPositionChanged(this, v));
}

void DeckAttributes::slotIntroEndPositionChanged(double v) {
    emit(introEndPositionChanged(this, v));
}

void DeckAttributes::slotOutroStartPositionChanged(double v) {
    emit(outroStartPositionChanged(this, v));
}

void DeckAttributes::slotOutroEndPositionChanged(double v) {
    emit(outroEndPositionChanged(this, v));
}

void DeckAttributes::slotTrackLoaded(TrackPointer pTrack) {
    emit(trackLoaded(this, pTrack));
}

void DeckAttributes::slotLoadingTrack(TrackPointer pNewTrack, TrackPointer pOldTrack) {
    //qDebug() << "DeckAttributes::slotLoadingTrack";
    emit(loadingTrack(this, pNewTrack, pOldTrack));
}

void DeckAttributes::slotPlayerEmpty() {
    emit(playerEmpty(this));
}

TrackPointer DeckAttributes::getLoadedTrack() const {
    return m_pPlayer != NULL ? m_pPlayer->getLoadedTrack() : TrackPointer();
}

AutoDJProcessor::AutoDJProcessor(QObject* pParent,
        UserSettingsPointer pConfig,
        PlayerManagerInterface* pPlayerManager,
        int iAutoDJPlaylistId,
        TrackCollection* pTrackCollection)
        : QObject(pParent),
          m_pConfig(pConfig),
          m_pPlayerManager(pPlayerManager),
          m_pAutoDJTableModel(NULL),
          m_eState(ADJ_DISABLED),
          m_transitionProgress(0.0),
          m_transitionTime(kTransitionPreferenceDefault) {
    m_pAutoDJTableModel = new PlaylistTableModel(this, pTrackCollection,
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
            static_cast<int>(TransitionMode::AlignIntroOutroStart));
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
    if (m_pCOCrossfaderReverse->get() > 0.0) {
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
    DeckAttributes& leftDeck = *m_decks[0];
    DeckAttributes& rightDeck = *m_decks[1];
    if (leftDeck.isPlaying() &&
            (!rightDeck.isPlaying() || crossfader < 0.0)) {
        // Make sure leftDeck.fadeDuration is up to date.
        calculateTransition(&leftDeck, &rightDeck, true, false);
        // Repeat is disabled by FadeNow but disables auto Fade
        leftDeck.setRepeat(false);
    } else if (rightDeck.isPlaying()) {
        // Make sure rightDeck.fadeDuration is up to date.
        calculateTransition(&rightDeck, &leftDeck, true, false);
        // Repeat is disabled by FadeNow but disables auto Fade
        rightDeck.setRepeat(false);
    }
    //else {
    //    No deck playing, do not know what to do
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
    }
    return ADJ_OK;
}

AutoDJProcessor::AutoDJError AutoDJProcessor::toggleAutoDJ(bool enable) {
    DeckAttributes& deck1 = *m_decks[0];
    DeckAttributes& deck2 = *m_decks[1];
    bool deck1Playing = deck1.isPlaying();
    bool deck2Playing = deck2.isPlaying();

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
            removeLoadedTrackFromTopOfQueue(deck1);
        } else if (deck2Playing) {
            removeLoadedTrackFromTopOfQueue(deck2);
        } else {
            // If the first track is already cued at a position in the first
            // 2/3 in on of the Auto DJ decks, start it.
            // If the track is paused at a later position, it is probably too
            // close to the end. In this case it is loaded again at the stored
            // cue point.
            if (deck1.playPosition() < 0.66 &&
                    removeLoadedTrackFromTopOfQueue(deck1)) {
                deck1.play();
                deck1Playing = true;
            } else if (deck2.playPosition() < 0.66 &&
                    removeLoadedTrackFromTopOfQueue(deck2)) {
                deck2.play();
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

        connect(&deck1, &DeckAttributes::playPositionChanged,
                this, &AutoDJProcessor::playerPositionChanged);
        connect(&deck2, &DeckAttributes::playPositionChanged,
                this, &AutoDJProcessor::playerPositionChanged);

        connect(&deck1, &DeckAttributes::playChanged,
                this, &AutoDJProcessor::playerPlayChanged);
        connect(&deck2, &DeckAttributes::playChanged,
                this, &AutoDJProcessor::playerPlayChanged);

        connect(&deck1, &DeckAttributes::introStartPositionChanged,
                this, &AutoDJProcessor::playerIntroStartChanged);
        connect(&deck2, &DeckAttributes::introStartPositionChanged,
                this, &AutoDJProcessor::playerIntroStartChanged);

        connect(&deck1, &DeckAttributes::introEndPositionChanged,
                this, &AutoDJProcessor::playerIntroEndChanged);
        connect(&deck2, &DeckAttributes::introEndPositionChanged,
                this, &AutoDJProcessor::playerIntroEndChanged);

        connect(&deck1, &DeckAttributes::outroStartPositionChanged,
                this, &AutoDJProcessor::playerOutroStartChanged);
        connect(&deck2, &DeckAttributes::outroStartPositionChanged,
                this, &AutoDJProcessor::playerOutroStartChanged);

        connect(&deck1, &DeckAttributes::outroEndPositionChanged,
                this, &AutoDJProcessor::playerOutroEndChanged);
        connect(&deck2, &DeckAttributes::outroEndPositionChanged,
                this, &AutoDJProcessor::playerOutroEndChanged);

        connect(&deck1, &DeckAttributes::trackLoaded,
                this, &AutoDJProcessor::playerTrackLoaded);
        connect(&deck2, &DeckAttributes::trackLoaded,
                this, &AutoDJProcessor::playerTrackLoaded);

        connect(&deck1, &DeckAttributes::loadingTrack,
                this, &AutoDJProcessor::playerLoadingTrack);
        connect(&deck2, &DeckAttributes::loadingTrack,
                this, &AutoDJProcessor::playerLoadingTrack);

        connect(&deck1, &DeckAttributes::playerEmpty,
                this, &AutoDJProcessor::playerEmpty);
        connect(&deck2, &DeckAttributes::playerEmpty,
                this, &AutoDJProcessor::playerEmpty);

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
            emitLoadTrackToPlayer(nextTrack, deck1.group, true);
        } else {
            // One of the two decks is playing. Switch into IDLE mode and wait
            // until the playing deck crosses posThreshold to start fading.
            m_eState = ADJ_IDLE;
            if (deck1Playing) {
                // Load track into the right deck.
                emitLoadTrackToPlayer(nextTrack, deck2.group, false);
                // Move crossfader to the left.
                setCrossfader(-1.0);
            } else {
                // Load track into the left deck.
                emitLoadTrackToPlayer(nextTrack, deck1.group, false);
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
        deck1.disconnect(this);
        deck2.disconnect(this);
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

    DeckAttributes& thisDeck = *pAttributes;
    // prefer to fade to deck 0
    int otherDeckIndex = 0;
    if (thisDeck.index == 0) {
        otherDeckIndex = 1;
    }
    DeckAttributes& otherDeck = *m_decks[otherDeckIndex];

    DeckAttributes& leftDeck = *m_decks[0];
    DeckAttributes& rightDeck = *m_decks[1];

    bool leftDeckPlaying = leftDeck.isPlaying();
    bool rightDeckPlaying = rightDeck.isPlaying();

    bool thisDeckPlaying = thisDeck.isPlaying();
    bool otherDeckPlaying = otherDeck.isPlaying();

    // To switch out of ADJ_ENABLE_P1LOADED we wait for a playposition update
    // for either deck.
    if (m_eState == ADJ_ENABLE_P1LOADED) {
        if (leftDeckPlaying || rightDeckPlaying) {
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
                removeLoadedTrackFromTopOfQueue(leftDeck);

                // Load the next track into the right player since it is not
                // playing.
                loadNextTrackFromQueue(rightDeck);

                // Note: calculateTransition() is called in playerTrackLoaded()
            } else {
                // At least right deck is playing
                // Set crossfade thresholds for right deck.
                calculateTransition(&rightDeck, &leftDeck, false, false);
            }
            emitAutoDJStateChanged(m_eState);
        }
        return;
    }

    // In FADING states, we expect that both tracks are playing.
    // Normally the the fading fromDeck stops after the transition is over and
    // we need to replace it with a new track from the queue. In the rare case the
    // toDeck stops first, we replace this one and stop the transition.
    // Then we switch the crossfader fully to the new track side,
    // switch to IDLE mode and load the next track into the other deck.
    if (m_eState == ADJ_LEFT_FADING || m_eState == ADJ_RIGHT_FADING) {
        // Once P1 or P2 has stopped switch out of fading mode to idle.
        if (!otherDeckPlaying) {
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
            thisDeck.fadeBeginPos = 1.0;
            thisDeck.fadeEndPos = 1.0;
            // Load the next track to otherDeck.
            loadNextTrackFromQueue(otherDeck);
            emitAutoDJStateChanged(m_eState);
            return;
        }
    }

    if (m_eState == ADJ_IDLE) {
        if (!thisDeckPlaying) {
            // this is a cueing seek, recalculate the transition, from the
            // new position.
            // This can be our own seek to startPos or a random seek by a user.
            // we need to call calculateTransition() because we are not sure.
            calculateTransition(&otherDeck, &thisDeck, false, false);
        } else if (thisDeck.isRepeat()) {
            // repeat pauses auto DJ
            return;
        }
    }

    // If we are past this deck's posThreshold then:
    // - transition into fading mode, play the other deck and fade to it.
    // - check if fading is done and stop the deck
    // - update the crossfader
    if (thisPlayPosition >= thisDeck.fadeBeginPos) {
        if (m_eState == ADJ_IDLE) {
            if (thisDeckPlaying || thisPlayPosition >= 1.0) {
                if (!otherDeckPlaying) {
                    otherDeck.play();
                }

                // Now that we have started the other deck playing, remove the track
                // that was "on deck" from the top of the queue.
                removeLoadedTrackFromTopOfQueue(otherDeck);

                m_transitionProgress = 0.0;
                // Set the state as FADING.
                m_eState = thisDeck.isLeft() ? ADJ_LEFT_FADING : ADJ_RIGHT_FADING;
                emitAutoDJStateChanged(m_eState);
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
            thisDeck.stop();
            m_transitionProgress = 1.0;
        } else {
            // We are in Fading state.
            // Calculate the current transitionProgress, the place between begin
            // and end position and the step we have taken since the last call
            double transitionProgress = (thisPlayPosition - thisDeck.fadeBeginPos) /
                    (thisDeck.fadeEndPos - thisDeck.fadeBeginPos);
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
        emit(randomTrackRequested(tracksToAdd));
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

    // Fill random tracks if configured
    int minAutoDJCrateTracks = m_pConfig->getValueString(
            ConfigKey(kConfigKey, "RandomQueueMinimumAllowed")).toInt();
    bool randomQueueEnabled = (((m_pConfig->getValueString(
            ConfigKey("[Auto DJ]", "EnableRandomQueue")).toInt())) == 1);

    int tracksToAdd = minAutoDJCrateTracks - m_pAutoDJTableModel->rowCount();
    if (randomQueueEnabled && (tracksToAdd > 0)) {
        qDebug() << "Randomly adding tracks";
        emit(randomTrackRequested(tracksToAdd));
    }

    return true;
}

void AutoDJProcessor::playerPlayChanged(DeckAttributes* pAttributes, bool playing) {
    if (sDebug) {
        qDebug() << this << "playerPlayChanged" << pAttributes->group << playing;
    }
    // We may want to do more than just calculate fade thresholds when playing
    // state changes so keep these two as separate methods for now.

    // This will calculate the Transition to the already loaded, in most cases
    // already played other track.
    // This is required because the user may have loaded a track or changed play
    // manually
    if (playing) {
        calculateTransition(pAttributes, getOtherDeck(pAttributes), false, false);
    }
}

void AutoDJProcessor::playerIntroStartChanged(DeckAttributes* pAttributes, double position) {
    if (sDebug) {
        qDebug() << this << "playerIntroStartChanged" << pAttributes->group << position;
    }

    if (!pAttributes->loading && !pAttributes->isPlaying()) {
        calculateTransition(getOtherDeck(pAttributes, true), pAttributes, false, false);
    }
}

void AutoDJProcessor::playerIntroEndChanged(DeckAttributes* pAttributes, double position) {
    if (sDebug) {
        qDebug() << this << "playerIntroEndChanged" << pAttributes->group << position;
    }

    if (!pAttributes->loading && !pAttributes->isPlaying()) {
        calculateTransition(getOtherDeck(pAttributes, true), pAttributes, false, false);
    }
}

void AutoDJProcessor::playerOutroStartChanged(DeckAttributes* pAttributes, double position) {
    if (sDebug) {
        qDebug() << this << "playerOutroStartChanged" << pAttributes->group << position;
    }

    if (!pAttributes->loading && pAttributes->isPlaying()) {
        calculateTransition(pAttributes, getOtherDeck(pAttributes, false), false, false);
    }
}

void AutoDJProcessor::playerOutroEndChanged(DeckAttributes* pAttributes, double position) {
    if (sDebug) {
        qDebug() << this << "playerOutroEndChanged" << pAttributes->group << position;
    }

    if (!pAttributes->loading && pAttributes->isPlaying()) {
        calculateTransition(pAttributes, getOtherDeck(pAttributes, false), false, false);
    }
}

double AutoDJProcessor::getIntroStartPosition(DeckAttributes* pDeck) {
    double introStart = samplePositionToSeconds(pDeck->introStartPosition(), pDeck);
    if (introStart <= 0.0) {
        introStart = getFirstSoundPosition(pDeck);
    }
    return introStart;
}

double AutoDJProcessor::getIntroEndPosition(DeckAttributes* pDeck) {
    return samplePositionToSeconds(pDeck->introEndPosition(), pDeck);
}

double AutoDJProcessor::getOutroStartPosition(DeckAttributes* pDeck) {
    return samplePositionToSeconds(pDeck->outroStartPosition(), pDeck);
}

double AutoDJProcessor::getOutroEndPosition(DeckAttributes* pDeck) {
    double outroEnd = samplePositionToSeconds(pDeck->outroEndPosition(), pDeck);
    if (outroEnd <= 0.0) {
        outroEnd = getLastSoundPosition(pDeck);
    }
    return outroEnd;
}

double AutoDJProcessor::getFirstSoundPosition(DeckAttributes* pDeck) {
    TrackPointer pTrack = pDeck->getLoadedTrack();
    if (!pTrack) {
        return 0;
    }

    CuePointer pFromTrackFirstSound = pTrack->findCueByType(Cue::Type::FirstSound);
    if (pFromTrackFirstSound) {
        return samplePositionToSeconds(pFromTrackFirstSound->getPosition(), pDeck);
    } else {
        return 0;
    }
}

double AutoDJProcessor::getLastSoundPosition(DeckAttributes* pDeck) {
    TrackPointer pTrack = pDeck->getLoadedTrack();
    if (!pTrack) {
        return 0;
    }

    CuePointer pFromTrackLastSound = pTrack->findCueByType(Cue::Type::LastSound);
    if (pFromTrackLastSound) {
        return samplePositionToSeconds(pFromTrackLastSound->getPosition(), pDeck);
    } else {
        return pDeck->duration();
    }
}

double AutoDJProcessor::getMainCuePosition(DeckAttributes* pDeck) {
    TrackPointer pTrack = pDeck->getLoadedTrack();
    if (!pTrack) {
        return 0;
    }

    CuePointer pMainCue = pTrack->findCueByType(Cue::Type::MainCue);
    if (pMainCue) {
        return samplePositionToSeconds(pMainCue->getPosition(), pDeck);
    } else {
        return 0;
    }
}


double AutoDJProcessor::samplePositionToSeconds(double samplePosition, DeckAttributes* pDeck) {
    samplePosition /= kChannelCount;
    double sampleRate = pDeck->sampleRate();
    if (samplePosition <= 0.0 || sampleRate <= 0.0) {
        return -1.0;
    }
    return samplePosition / sampleRate;
}

void AutoDJProcessor::calculateTransition(DeckAttributes* pFromDeck,
        DeckAttributes* pToDeck,
        bool fadeNow,
        bool seekToStartPoint) {
    if (pFromDeck == nullptr) {
        return;
    }

    if (sDebug) {
        qDebug() << this << "calculateFadeThresholds" << pFromDeck->group;
    }

    //qDebug() << "player" << pAttributes->group << "PlayChanged(" << playing << ")";

    // We require ADJ_IDLE to prevent changing the thresholds in the middle of a
    // fade.
    if (m_eState != ADJ_IDLE) {
        return;
    }

    double fromTrackDuration = pFromDeck->duration();
    double toTrackDuration = pToDeck->duration();

    VERIFY_OR_DEBUG_ASSERT(fromTrackDuration > 0) {
        // Playing Track has no duration. This should not happen, because short
        // tracks are skipped after load. Play ToDeck immediately.
        pFromDeck->fadeBeginPos = 0;
        pFromDeck->fadeEndPos = 0;
        pToDeck->startPos = kKeepPosition;
        return;
    }
    VERIFY_OR_DEBUG_ASSERT(toTrackDuration > 0) {
        // Playing Track has no duration. This should not happen, because short
        // tracks are skipped after load.
        pToDeck->startPos = kKeepPosition;
        // Disable AutoDJ.
        toggleAutoDJ(false);
    }

    // Within this function, the outro refers to the outro of the currently
    // playing track and the intro refers to the intro of the next track.

    double outroStart;

    double outroEnd = getOutroEndPosition(pFromDeck);

    if (fadeNow) {
        // Assume that the outro starts now and equals the given transition time
        // but do not pass the original outroEnd
        outroStart = pFromDeck->playPosition() * fromTrackDuration;
        // if one uses fadeNow with a gap transition we take this time for fading here
        double transitionTime = fabs(m_transitionTime);
        if (outroEnd > outroStart) {
            outroEnd = math_min(outroEnd, outroStart + transitionTime);
        } else {
            outroEnd = math_min(fromTrackDuration, outroStart + transitionTime);
        }
    } else {
        outroStart = getOutroStartPosition(pFromDeck);
        if (outroStart <= 0.0) {
            // Assume a zero length outro.
            // The outroEnd is automatically placed by AnalyzerSilence, so use
            // that as a fallback if the user has not placed outroStart.
            outroStart = outroEnd;
        }
    }

    double outroLength = outroEnd - outroStart;

    double introStart;
    if (seekToStartPoint) {
        introStart = getIntroStartPosition(pToDeck);
    } else {
        introStart = pToDeck->playPosition() * toTrackDuration;
    }

    double introEnd = getIntroEndPosition(pToDeck);
    if (introEnd < introStart) {
        // introEnd is invalid. Assume a zero length intro.
        // The introStart is automatically placed by AnalyzerSilence, so use
        // that as a fallback if the user has not placed introEnd.
        introEnd = introStart;
    }

    double introLength = introEnd - introStart;

    // For both the AlignIntroOutroStart & AlignIntroOutroEnd TransitionModes,
    // fadeDuration is the intro or outro length, whichever is shorter. This
    // is the best way to avoid clashing sounds overlapping. If only one track
    // has an intro or outro range marked (not just one point of the range, but
    // the full range with the end and beginning markers), use the length of
    // that range as the transition time as a best guess. Only fall back to the
    // fixed number of seconds from the spinbox as a last resort.
    switch (m_transitionMode) {
    case TransitionMode::AlignIntroOutroStart:
        pToDeck->startPos = introStart;
        if (outroLength > 0) {
            pFromDeck->fadeBeginPos = outroStart;
            if (introLength > 0 && introLength < outroLength) {
                pFromDeck->fadeEndPos = pFromDeck->fadeBeginPos + introLength;
            } else if (pToDeck->startPos + outroLength >= toTrackDuration) {
                pFromDeck->fadeEndPos = pFromDeck->fadeBeginPos +
                        toTrackDuration - pToDeck->startPos;
            } else {
                pFromDeck->fadeEndPos = outroEnd;
            }
        } else if (introLength > 0) {
            pFromDeck->fadeBeginPos = outroEnd - introLength;
            pFromDeck->fadeEndPos = outroEnd;
        } else {
            useFixedFadeTime(pFromDeck, pToDeck, outroEnd, introStart);
        }
        break;
    case TransitionMode::AlignIntroOutroEnd:
        if (introLength > 0) {
            pFromDeck->fadeEndPos = outroEnd;
            if (outroLength > 0) {
                pFromDeck->fadeBeginPos = pFromDeck->fadeEndPos - math_min(outroLength, introLength);
            } else {
                pFromDeck->fadeBeginPos = math_max(pFromDeck->fadeEndPos - introLength, 0.0);
            }
            pToDeck->startPos = introEnd - (pFromDeck->fadeEndPos - pFromDeck->fadeBeginPos);
        } else if (outroLength > 0) {
            pToDeck->startPos = introStart;
            pFromDeck->fadeEndPos = outroEnd;
            if (pToDeck->startPos + outroLength >= toTrackDuration) {
                pFromDeck->fadeBeginPos = pFromDeck->fadeEndPos -
                        toTrackDuration + pToDeck->startPos;
            } else {
                pFromDeck->fadeBeginPos = outroStart;
            }
        } else {
            useFixedFadeTime(pFromDeck, pToDeck, outroEnd, introStart);
        }
        break;
    case TransitionMode::FixedSkipSilence:
        if (fadeNow) {
            useFixedFadeTime(pFromDeck,
                    pToDeck,
                    outroEnd,
                    getFirstSoundPosition(pToDeck));
        } else {
            useFixedFadeTime(pFromDeck,
                    pToDeck,
                    getLastSoundPosition(pFromDeck),
                    getFirstSoundPosition(pToDeck));
        }
        break;
    case TransitionMode::FixedLoadAtCue:
        if (fadeNow) {
            useFixedFadeTime(pFromDeck,
                    pToDeck,
                    outroEnd,
                    getMainCuePosition(pToDeck));
        } else {
            useFixedFadeTime(pFromDeck,
                    pToDeck,
                    getLastSoundPosition(pFromDeck),
                    getMainCuePosition(pToDeck));
        }
        break;
    case TransitionMode::LimitedIntroOutroStart:
        // Uses a simple set of rules:
        // * outro start starts fade if set
        // * intro length is limited by transition time
        // * fade time is the minimum of all
        // no re-cuing of "to track" if no intro end is set. (legacy mode)

        // Use cases:
        //
        // By default only intro start and outro end are set. In this case fade
        // starts at outro end - transitionTime. The new track is not recued,
        // the deck preferences are used to set the start point.
        //
        // If the track has for instance a boring long outro the user can
        // manually set the outro start. This set the fade start in any case.
        // The fade time is now set by the outo length or the tranitonTime.
        // The shorter is selected.
        //
        // If the track has a poisend end or something the user can adjust the
        // outro end. It is never played bejond that point.
        //
        // The user can override the deck track load preferences by setting a
        // intro end point and adjust the intro start. Fading starts at intro
        // start or intro end - transitionTime, depending on which is shorter.
        // The transition is always finished before passing intro out.
        // This can for instance be useful to ensure that fading is over before
        // vocals start.

        // outro            xxxxxxx
        // intro            xxxxx
        // transitionTime   xxxxxxx
        // fade             xxxxx

        // outro             xxx
        // intro           xxxxxxx
        // transitionTime    xxxxx
        // fade              xxx

        // outro            xxxxxxx
        // intro          xxxxx
        // transitionTime   xxx
        // fade             xxx

        // outro          xxx
        // intro                  xxx
        // negativeTime      -----
        // fade              xxxxx

        // Negative transitionTimes are working but not too reasonable.
        // Note: In the negative transitionTime case we play bejond outro end.
        // TODO: It would be nice to fade out the last beat outro, add the silence gap
        // and fade in the first beat into.

        if (m_transitionTime <= 0) {
            // play track including full intro and outro with an adjustable gap
            useFixedFadeTime(pFromDeck, pToDeck, outroEnd, introStart);
        } else {
            double fadeLength = m_transitionTime;
            if (outroLength > 0 && outroLength < fadeLength) {
                fadeLength = outroLength;
            }
            if (introLength == 0) {
                // Intro not set, don't re-cue the track
                pToDeck->startPos = kKeepPosition;
            } else if (introLength > m_transitionTime) {
                // Intro is too long, cut some time from the beginning
                pToDeck->startPos = introEnd - m_transitionTime;
            } else {
                // Intro is fine, play entirely
                pToDeck->startPos = introStart;
                if (introLength < fadeLength) {
                    // Use full intro for fading if it is the shortest time.
                    // This guarantees not to fade beyond intro end.
                    fadeLength = introLength;
                }
            }
            if (outroLength > 0) {
                // If an outroStart cue is set: Us it!
                pFromDeck->fadeBeginPos = outroStart;
                pFromDeck->fadeEndPos = outroStart + fadeLength;
            } else {
                pFromDeck->fadeBeginPos = outroEnd - fadeLength;
                pFromDeck->fadeEndPos = outroEnd;
            }
        }
        break;
    case TransitionMode::FixedFullTrack:
    default:
        if (fadeNow) {
            useFixedFadeTime(pFromDeck, pToDeck, outroEnd, 0);
        } else {
            useFixedFadeTime(pFromDeck, pToDeck, fromTrackDuration, 0);
        }
    }

    // These are expected to be a fraction of the track length.
    pFromDeck->fadeBeginPos /= fromTrackDuration;
    pFromDeck->fadeEndPos /= fromTrackDuration;
    pToDeck->startPos /= toTrackDuration;

    DEBUG_ASSERT(pFromDeck->fadeBeginPos <= 1);

    if (sDebug) {
        qDebug() << this << pFromDeck->fadeBeginPos << pFromDeck->fadeEndPos
                << pToDeck->startPos;
    }
}

void AutoDJProcessor::useFixedFadeTime(DeckAttributes* pFromDeck,
        DeckAttributes* pToDeck, double endPoint, double startPoint) {
    if (m_transitionTime > 0.0) {
        // Guard against the next track being too short. This transition must finish
        // before the next transition starts.
        double toDeckOutroStart = getOutroStartPosition(pToDeck);
        if (toDeckOutroStart <= startPoint) {
            // we are already to late
            double end = getOutroEndPosition(pToDeck);
            if (end <= startPoint) {
                end = pToDeck->duration();
                VERIFY_OR_DEBUG_ASSERT(end > startPoint) {
                    // as last resort move start point
                    startPoint = pToDeck->duration() - 1;
                }
            }
            // use the remaining time for fad to this dack ende fade to the next
            toDeckOutroStart = (end - startPoint) / 2 + startPoint;
        }
        double transitionTime = math_min(toDeckOutroStart - startPoint,
                m_transitionTime);

        pFromDeck->fadeBeginPos = endPoint - transitionTime;
        pFromDeck->fadeEndPos = endPoint;
        pToDeck->startPos = startPoint;
    } else {
        pFromDeck->fadeBeginPos = endPoint;
        pFromDeck->fadeEndPos = endPoint - m_transitionTime;
        pToDeck->startPos = startPoint + m_transitionTime;
    }
}

void AutoDJProcessor::playerTrackLoaded(DeckAttributes* pDeck, TrackPointer pTrack) {
    if (sDebug) {
        qDebug() << this << "playerTrackLoaded" << pDeck->group
                 << (pTrack ? pTrack->getLocation() : "(null)");
    }

    pDeck->loading = false;

    double duration = pTrack->getDuration();
    if (duration < 0.2) {
        qWarning() << "Skip track with" << duration << "Duration"
                   << pTrack->getLocation();
        // Remove Tack with duration smaller than two callbacks
        removeTrackFromTopOfQueue(pTrack);

        // Load the next track. If we are the first AutoDJ track
        // (ADJ_ENABLE_P1LOADED state) then play the track.
        loadNextTrackFromQueue(*pDeck, m_eState == ADJ_ENABLE_P1LOADED);
    } else {
        calculateTransition(getOtherDeck(pDeck, true), pDeck, false, true);
        if (pDeck->startPos != kKeepPosition) {
            // Note: this seek will trigger the playerPositionChanged slot
            // which may calls the calculateTransition() again without seek = true;
            pDeck->setPlayPosition(pDeck->startPos);
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
            calculateTransition(&leftDeck, &rightDeck, false, false);
        }
        if (rightDeck.isPlaying()) {
            calculateTransition(&rightDeck, &leftDeck, false, false);
        }
    }
}

void AutoDJProcessor::setTransitionMode(TransitionMode newMode) {
    m_pConfig->set(ConfigKey(kConfigKey, kTransitionModePreferenceName),
                   ConfigValue(static_cast<int>(newMode)));
    m_transitionMode = newMode;

    // Then re-calculate fade thresholds for the decks.
    if (m_eState == ADJ_IDLE) {
        DeckAttributes& leftDeck = *m_decks[0];
        DeckAttributes& rightDeck = *m_decks[1];

        if (leftDeck.isPlaying() && !rightDeck.isPlaying()) {
            calculateTransition(&leftDeck, &rightDeck, false, true);
            if (rightDeck.startPos != kKeepPosition) {
                // Note: this seek will trigger the playerPositionChanged slot
                // which may calls the calculateTransition() again without seek = true;
                rightDeck.setPlayPosition(rightDeck.startPos);
            }
        } else if (rightDeck.isPlaying() && !leftDeck.isPlaying()) {
            calculateTransition(&rightDeck, &leftDeck, false, true);
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
}

DeckAttributes* AutoDJProcessor::getOtherDeck(DeckAttributes* pThisDeck,
                                              bool playing) {
    DeckAttributes* pOtherDeck = NULL;
    if (pThisDeck->isLeft()) {
        // find first right deck
        foreach(DeckAttributes* pDeck, m_decks) {
            if (pDeck->isRight()) {
                if (!playing || pDeck->isPlaying()) {
                    pOtherDeck = pDeck;
                    break;
                }
            }
        }
    } else if (pThisDeck->isRight()) {
        // find first left deck
        foreach(DeckAttributes* pDeck, m_decks) {
            if (pDeck->isLeft()) {
                if (!playing || pDeck->isPlaying()) {
                    pOtherDeck = pDeck;
                    break;
                }
            }
        }
    }
    return pOtherDeck;
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
