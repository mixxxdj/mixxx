#include "library/autodj/autodjprocessor.h"

#include "library/trackcollection.h"
#include "controlpushbutton.h"
#include "controlobjectslave.h"
#include "util/math.h"
#include "playermanager.h"
#include "basetrackplayer.h"

#define kConfigKey "[Auto DJ]"
const char* kTransitionPreferenceName = "Transition";
const int kTransitionPreferenceDefault = 10;

static const bool sDebug = false;

DeckAttributes::DeckAttributes(int index,
                               BaseTrackPlayer* pPlayer,
                               EngineChannel::ChannelOrientation orientation)
        : index(index),
          group(pPlayer->getGroup()),
          posThreshold(1.0),
          fadeDuration(0.0),
          m_orientation(orientation),
          m_playPos(group, "playposition"),
          m_play(group, "play"),
          m_repeat(group, "repeat"),
          m_pPlayer(pPlayer) {
    connect(m_pPlayer, SIGNAL(newTrackLoaded(TrackPointer)),
            this, SLOT(slotTrackLoaded(TrackPointer)));
    connect(m_pPlayer, SIGNAL(loadTrackFailed(TrackPointer)),
            this, SLOT(slotTrackLoadFailed(TrackPointer)));
    connect(m_pPlayer, SIGNAL(unloadingTrack(TrackPointer)),
            this, SLOT(slotTrackUnloaded(TrackPointer)));
    m_playPos.connectValueChanged(this, SLOT(slotPlayPosChanged(double)));
    m_play.connectValueChanged(this, SLOT(slotPlayChanged(double)));
}

DeckAttributes::~DeckAttributes() {
}

void DeckAttributes::slotPlayChanged(double v) {
    emit(playChanged(this, v > 0.0));
}

void DeckAttributes::slotPlayPosChanged(double v) {
    emit(playPositionChanged(this, v));
}

void DeckAttributes::slotTrackLoaded(TrackPointer pTrack) {
    emit(trackLoaded(this, pTrack));
}

void DeckAttributes::slotTrackLoadFailed(TrackPointer pTrack) {
    emit(trackLoadFailed(this, pTrack));
}

void DeckAttributes::slotTrackUnloaded(TrackPointer pTrack) {
    emit(trackUnloaded(this, pTrack));
}

TrackPointer DeckAttributes::getLoadedTrack() const {
    return m_pPlayer != NULL ? m_pPlayer->getLoadedTrack() : TrackPointer();
}

AutoDJProcessor::AutoDJProcessor(QObject* pParent,
                                 ConfigObject<ConfigValue>* pConfig,
                                 PlayerManagerInterface* pPlayerManager,
                                 int iAutoDJPlaylistId,
                                 TrackCollection* pTrackCollection)
        : QObject(pParent),
          m_pConfig(pConfig),
          m_pPlayerManager(pPlayerManager),
          m_pAutoDJTableModel(NULL),
          m_eState(ADJ_DISABLED),
          m_iTransitionTime(kTransitionPreferenceDefault),
          m_nextTransitionTime(kTransitionPreferenceDefault) {
    m_pAutoDJTableModel = new PlaylistTableModel(this, pTrackCollection,
                                                 "mixxx.db.model.autodj");
    m_pAutoDJTableModel->setTableModel(iAutoDJPlaylistId);

    m_pShufflePlaylist = new ControlPushButton(
            ConfigKey("[AutoDJ]", "shuffle_playlist"));
    connect(m_pShufflePlaylist, SIGNAL(valueChanged(double)),
            this, SLOT(controlShuffle(double)));

    m_pSkipNext = new ControlPushButton(
            ConfigKey("[AutoDJ]", "skip_next"));
    connect(m_pSkipNext, SIGNAL(valueChanged(double)),
            this, SLOT(controlSkipNext(double)));

    m_pFadeNow = new ControlPushButton(
            ConfigKey("[AutoDJ]", "fade_now"));
    connect(m_pFadeNow, SIGNAL(valueChanged(double)),
            this, SLOT(controlFadeNow(double)));

    m_pEnabledAutoDJ = new ControlPushButton(
            ConfigKey("[AutoDJ]", "enabled"));
    m_pEnabledAutoDJ->setButtonMode(ControlPushButton::TOGGLE);
    connect(m_pEnabledAutoDJ, SIGNAL(valueChanged(double)),
            this, SLOT(controlEnable(double)));

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

    m_pCOCrossfader = new ControlObjectSlave("[Master]", "crossfader");
    m_pCOCrossfaderReverse = new ControlObjectSlave("[Mixer Profile]", "xFaderReverse");

    QString str_autoDjTransition = m_pConfig->getValueString(
            ConfigKey(kConfigKey, kTransitionPreferenceName));
    if (!str_autoDjTransition.isEmpty()) {
        m_iTransitionTime = str_autoDjTransition.toInt();
        m_nextTransitionTime =  m_iTransitionTime;
    }
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

void AutoDJProcessor::setCrossfader(double value, bool right) {
    if (m_pCOCrossfaderReverse->get() > 0.0) {
        value *= -1.0;
        right = !right;
    }
    double current_value = m_pCOCrossfader->get();
    if (right) {
        // ignore if we move slider left
        if (value > current_value) {
            m_pCOCrossfader->set(value);
        }
    } else {
        // ignore if we move slider right
        if (value < current_value) {
            m_pCOCrossfader->set(value);
        }
    }
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

AutoDJProcessor::AutoDJError AutoDJProcessor::fadeNow() {
    // Auto-DJ needs at least two decks
    DEBUG_ASSERT_AND_HANDLE(m_decks.length() > 1) {
        return ADJ_NOT_TWO_DECKS;
    }
    if (m_eState == ADJ_IDLE) {
        double crossfader = getCrossfader();
        DeckAttributes& leftDeck = *m_decks[0];
        DeckAttributes& rightDeck = *m_decks[1];
        if (crossfader <= 0.3 && leftDeck.isPlaying()) {
            // left deck is playing and the crossfader is on the left

            // Make sure leftDeck.fadeDuration is up to date.
            calculateTransition(&leftDeck, &rightDeck);

            // override posThreshold to start fade now
            leftDeck.posThreshold = leftDeck.playPosition() -
                    ((crossfader + 1.0) / 2 * (leftDeck.fadeDuration));
            // Repeat is disabled by FadeNow but disables auto Fade
            leftDeck.setRepeat(false);
        } else if (crossfader >= -0.3 && rightDeck.isPlaying()) {
            // right deck is playing and the crossfader is on the right

            // Make sure rightDeck.fadeDuration is up to date.
            calculateTransition(&rightDeck, &leftDeck);

            // override posThreshold to start fade now
            rightDeck.posThreshold = rightDeck.playPosition() -
                    ((1.0 - crossfader) / 2 * (rightDeck.fadeDuration));
            // Repeat is disabled by FadeNow but disables auto Fade
            rightDeck.setRepeat(false);
        }
        // else { // do not know what to do  }
    }
    return ADJ_OK;
}

AutoDJProcessor::AutoDJError AutoDJProcessor::skipNext() {
    if (m_eState == ADJ_DISABLED) {
        return ADJ_IS_INACTIVE;
    }

    // Auto-DJ needs at least two decks
    DEBUG_ASSERT_AND_HANDLE(m_decks.length() > 1) {
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
    DeckAttributes& leftDeck = *m_decks[0];
    DeckAttributes& rightDeck = *m_decks[1];
    bool deck1Playing = leftDeck.isPlaying();
    bool deck2Playing = rightDeck.isPlaying();

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
            removeLoadedTrackFromTopOfQueue(leftDeck);
        }
        if (deck2Playing) {
            removeLoadedTrackFromTopOfQueue(rightDeck);
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

        connect(&leftDeck, SIGNAL(playPositionChanged(DeckAttributes*, double)),
                this, SLOT(playerPositionChanged(DeckAttributes*, double)));
        connect(&rightDeck, SIGNAL(playPositionChanged(DeckAttributes*, double)),
                this, SLOT(playerPositionChanged(DeckAttributes*, double)));

        connect(&leftDeck, SIGNAL(playChanged(DeckAttributes*, bool)),
                this, SLOT(playerPlayChanged(DeckAttributes*, bool)));
        connect(&rightDeck, SIGNAL(playChanged(DeckAttributes*, bool)),
                this, SLOT(playerPlayChanged(DeckAttributes*, bool)));

        connect(&leftDeck, SIGNAL(trackLoaded(DeckAttributes*, TrackPointer)),
                this, SLOT(playerTrackLoaded(DeckAttributes*, TrackPointer)));
        connect(&rightDeck, SIGNAL(trackLoaded(DeckAttributes*, TrackPointer)),
                this, SLOT(playerTrackLoaded(DeckAttributes*, TrackPointer)));

        connect(&leftDeck, SIGNAL(trackUnloaded(DeckAttributes*, TrackPointer)),
                this, SLOT(playerTrackUnloaded(DeckAttributes*, TrackPointer)));
        connect(&rightDeck, SIGNAL(trackUnloaded(DeckAttributes*, TrackPointer)),
                this, SLOT(playerTrackUnloaded(DeckAttributes*, TrackPointer)));

        connect(&leftDeck, SIGNAL(trackLoadFailed(DeckAttributes*, TrackPointer)),
                this, SLOT(playerTrackLoadFailed(DeckAttributes*, TrackPointer)));
        connect(&rightDeck, SIGNAL(trackLoadFailed(DeckAttributes*, TrackPointer)),
                this, SLOT(playerTrackLoadFailed(DeckAttributes*, TrackPointer)));


        if (!deck1Playing && !deck2Playing) {
            // Both decks are stopped. Load a track into deck 1 and start it
            // playing. Instruct playerPositionChanged to wait for a
            // playposition update from deck 1. playerPositionChanged for
            // ADJ_ENABLE_P1LOADED will set the crossfader left and remove the
            // loaded track from the queue and wait for the next call to
            // playerPositionChanged for deck1 after the track is loaded.
            m_eState = ADJ_ENABLE_P1LOADED;

            // Move crossfader to the left.
            setCrossfader(-1.0, false);

            // Load track into the left deck and play. Once it starts playing,
            // we will receive a playerPositionChanged update for deck 1 which
            // will load a track into the right deck and switch to IDLE mode.
            emitLoadTrackToPlayer(nextTrack, leftDeck.group, true);
        } else {
            // One of the two decks is playing. Switch into IDLE mode and wait
            // until the playing deck crosses posThreshold to start fading.
            m_eState = ADJ_IDLE;
            if (deck1Playing) {
                // Update fade thresholds for the left deck.
                calculateTransition(&leftDeck, &rightDeck);
                // Load track into the right deck.
                emitLoadTrackToPlayer(nextTrack, rightDeck.group, false);
            } else {
                // Update fade thresholds for the right deck.
                calculateTransition(&rightDeck, &leftDeck);
                // Load track into the left deck.
                emitLoadTrackToPlayer(nextTrack, leftDeck.group, false);
            }
        }
        emitAutoDJStateChanged(m_eState);
    } else {  // Disable Auto DJ
        if (m_pEnabledAutoDJ->get() != 0.0) {
            m_pEnabledAutoDJ->set(0.0);
        }
        qDebug() << "Auto DJ disabled";
        m_eState = ADJ_DISABLED;
        leftDeck.disconnect(this);
        rightDeck.disconnect(this);
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
        qDebug() << this << "playerPositionChanged" << pAttributes->group
                 << thisPlayPosition;
    }

    // Auto-DJ needs at least two decks
    DEBUG_ASSERT_AND_HANDLE(m_decks.length() > 1) {
        return;
    }

    // 95% playback is when we crossfade and do stuff
    // const double posThreshold = 0.95;

    // 0.05; // 5% playback is crossfade duration
    const double thisFadeDuration = pAttributes->fadeDuration;

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

            if (leftDeckPlaying && !rightDeckPlaying) {
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

                // Set crossfade thresholds for left deck.
                calculateTransition(&leftDeck, &rightDeck);
            } else {
                // Set crossfade thresholds for right deck.
                calculateTransition(&rightDeck, &leftDeck);
            }
            emitAutoDJStateChanged(m_eState);
        }
        return;
    }

    // In P1FADING/P2FADING states, we are waiting for the deck we are fading
    // from (P1 or P2) to stop playing. Once we are playing and the other deck
    // is not playing -- we switch the crossfader fully to this deck's side,
    // switch to IDLE mode and load the next track into the other deck.
    if ((m_eState == ADJ_P1FADING && thisDeck.isRight()) ||
        (m_eState == ADJ_P2FADING && thisDeck.isLeft())) {
        // Once P1 or P2 has stopped switch out of fading mode to idle.
        if (thisDeckPlaying && !otherDeckPlaying) {
            // Force crossfader all the way to this side.
            if (thisDeck.isLeft()) {
                setCrossfader(-1.0, false);
            } else {
                setCrossfader(1.0, true);
            }
            m_eState = ADJ_IDLE;
            // Invalidate threshold calculated for the old otherDeck
            // This avoids starting a fade back before the new track is
            // loaded into the otherDeck
            thisDeck.posThreshold = 1.0;
            thisDeck.fadeDuration = 0.0;
            // Load the next track to otherDeck.
            loadNextTrackFromQueue(otherDeck);
            emitAutoDJStateChanged(m_eState);
        }
        return;
    }

    if (m_eState == ADJ_IDLE && thisDeck.isRepeat()) {
        // repeat pauses auto DJ
        return;
    }

    // If we are past this deck's posThreshold then:
    // - transition into fading mode, play the other deck and fade to it.
    // - check if we've passed the fade end point and stop the deck
    // - update the crossfader
    // TODO(rryan): We need to investigate the chain of events that occur when
    // the track we are fading to crosses its posThreshold before we are done
    // fading to it.
    if (thisPlayPosition >= thisDeck.posThreshold) {
        if (m_eState == ADJ_IDLE && (thisDeckPlaying ||
                                     thisDeck.posThreshold >= 1.0)) {
            if (!otherDeckPlaying) {
                // Setup the other deck's fade thresholds (since we use the
                // fadeDuration next).
                calculateTransition(&otherDeck, &thisDeck);

                // For negative fade durations, jump back to insert a pause
                // between the tracks.
                // Note: This overrides the cue position
                if (thisFadeDuration < 0.0) {
                    // Note: since the fade duration is relative to the track
                    // length, we need to use the other deck fade duration
                    // which is negative as well
                    otherDeck.setPlayPosition(otherDeck.fadeDuration);
                } else {
                    // Guard against very short other tracks, or CUE points and
                    // seeks near end of track.
                    // The other track must not be finished before this track,
                    // otherwise Auto-DJ stops
                    double otherDeckPlaypos = otherDeck.playPosition();
                    // We need 2 x fadeDuration, to fade in and out
                    double maxPlaypos = 1.0 - (otherDeck.fadeDuration * 2);
                    if (maxPlaypos < otherDeckPlaypos) {
                        otherDeck.setPlayPosition(maxPlaypos);
                    }
                }
                otherDeck.play();
            }

            // Now that we have started the other deck playing, remove the track
            // that was "on deck" from the top of the queue.
            removeLoadedTrackFromTopOfQueue(otherDeck);

            // Set the state as FADING.
            m_eState = thisDeck.isLeft() ? ADJ_P1FADING : ADJ_P2FADING;
            emitAutoDJStateChanged(m_eState);
        }

        double posFadeEnd = math_min(1.0, thisDeck.posThreshold + thisFadeDuration);
        if (thisPlayPosition >= posFadeEnd) {
            // If this track has passed the end of its target fade then we stop
            // it. We don't handle mode switches here since that's handled by
            // the next playerPositionChanged call otherDeck (see the
            // P1/P2FADING case above).
            thisDeck.stop();
        } else {
            // We are past this deck's posThreshold but before its
            // posThreshold+fadeDuration, we move the crossfader linearly with
            // movements in this track's play position.

            // If thisDeck is left, the new crossfade value is -1 plus the
            // adjustment.  If thisDeck is right, the new value is 1.0 minus the
            // adjustment.
            double crossfadeEdgeValue = -1.0;
            double adjustment = 2 * (thisPlayPosition - thisDeck.posThreshold) /
                    (posFadeEnd - thisDeck.posThreshold);
            bool isLeft = thisDeck.isLeft();
            if (!isLeft) {
                crossfadeEdgeValue = 1.0;
                adjustment *= -1.0;
            }
            setCrossfader(crossfadeEdgeValue + adjustment, isLeft);
        }
    }
}

TrackPointer AutoDJProcessor::getNextTrackFromQueue() {
    // Get the track at the top of the playlist.
    bool randomQueueEnabled = (((m_pConfig->getValueString(
                ConfigKey("[Auto DJ]", "EnableRandomQueue")).toInt())) == 1);
    int minAutoDJCrateTracks = m_pConfig->getValueString(
                ConfigKey(kConfigKey, "RandomQueueMinimumAllowed")).toInt();
    int tracksToAdd = minAutoDJCrateTracks - m_pAutoDJTableModel->rowCount();
    // Incase we start off with < minimum tracks
    if (randomQueueEnabled && (tracksToAdd > 0)) {
        emit(randomTrackRequested(tracksToAdd));
    }

    while (true) {
        TrackPointer nextTrack = m_pAutoDJTableModel->getTrack(
            m_pAutoDJTableModel->index(0, 0));

        if (nextTrack) {
            if (nextTrack->exists()) {
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
    // Get loaded track for this group.
    TrackPointer loadedTrack = deck.getLoadedTrack();

    // No loaded track in this group.
    if (loadedTrack.isNull()) {
        return false;
    }

    return removeTrackFromTopOfQueue(loadedTrack);
}

bool AutoDJProcessor::removeTrackFromTopOfQueue(TrackPointer pTrack) {
    // No track to test for.
    if (pTrack.isNull()) {
        return false;
    }

    int trackId = pTrack->getId();

    // Loaded track is not a library track.
    if (trackId == -1) {
        return false;
    }

    // Get the track id at the top of the playlist.
    int nextId = m_pAutoDJTableModel->getTrackId(
            m_pAutoDJTableModel->index(0, 0));

    // No track at the top of the queue.
    if (nextId == -1) {
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
        calculateTransition(pAttributes, getOtherDeck(pAttributes));
    }
}

void AutoDJProcessor::calculateTransition(DeckAttributes* pFromDeck,
                                          DeckAttributes* pToDeck) {
    if (pFromDeck == NULL) {
        return;
    }

    if (sDebug) {
        qDebug() << this << "calculateFadeThresholds" << pFromDeck->group;
    }

    //qDebug() << "player" << pAttributes->group << "PlayChanged(" << playing << ")";

    // We require ADJ_IDLE to prevent changing the thresholds in the middle of a
    // fade.
    if (m_eState == ADJ_IDLE) {
        TrackPointer fromTrack = pFromDeck->getLoadedTrack();
        if (fromTrack) {
            // TODO(rryan): Duration is super inaccurate! We should be using
            // track_samples / track_samplerate instead.
            int fromTrackDuration = fromTrack->getDuration();
            qDebug() << fromTrack->getLocation()
                    << "fromTrackDuration =" << fromTrackDuration;

            // The track might be shorter than the transition period. Use a
            // sensible cap.
            m_nextTransitionTime = math_min(m_iTransitionTime,
                                            fromTrackDuration / 2);

            if (pToDeck) {
                TrackPointer toTrack = pToDeck->getLoadedTrack();
                if (toTrack) {
                    // TODO(rryan): Duration is super inaccurate! We should be using
                    // track_samples / track_samplerate instead.
                    int toTrackDuration = toTrack->getDuration();
                    qDebug() << toTrack->getLocation()
                            << "toTrackDuration = " << toTrackDuration;
                    m_nextTransitionTime = math_min(m_nextTransitionTime,
                                                    toTrackDuration / 2);
                }
            }

            if (fromTrackDuration > 0) {
                pFromDeck->fadeDuration =
                        static_cast<double>(m_nextTransitionTime) /
                        static_cast<double>(fromTrackDuration);
            } else {
                pFromDeck->fadeDuration = 0;
            }

            if (m_nextTransitionTime > 0) {
                pFromDeck->posThreshold = 1.0 - pFromDeck->fadeDuration;
            } else {
                // in case of pause transition
                pFromDeck->posThreshold = 1.0;
            }
            qDebug() << "m_fadeDuration" << pFromDeck->group << "="
                     << pFromDeck->fadeDuration;
        }
    }
}

void AutoDJProcessor::playerTrackLoaded(DeckAttributes* pDeck, TrackPointer pTrack) {
    if (sDebug) {
        qDebug() << this << "playerTrackLoaded" << pDeck->group
                 << (pTrack.isNull() ? "(null)" : pTrack->getLocation());
    }

    if (pTrack->getDuration() == 0) {
        qWarning() << "Skip track with 0:00 Duration" << pTrack->getLocation();
        // Remove Tack with duration < 1 s
        removeTrackFromTopOfQueue(pTrack);

        // Load the next track. If we are the first AutoDJ track
        // (ADJ_ENABLE_P1LOADED state) then play the track.
        loadNextTrackFromQueue(*pDeck, m_eState == ADJ_ENABLE_P1LOADED);
    } else {
        calculateTransition(getOtherDeck(pDeck, true), pDeck);
    }
}

void AutoDJProcessor::playerTrackLoadFailed(DeckAttributes* pDeck, TrackPointer pTrack) {
    if (sDebug) {
        qDebug() << this << "playerTrackLoadFailed" << pDeck->group
                 << (pTrack.isNull() ? "(null)" : pTrack->getLocation());
    }

    // There are four conditions under which we load a track.
    // 1) We are enabling AutoDJ and no decks are playing. Mode is
    //    ADJ_ENABLE_P1LOADED.
    // 2) After #1, we load a track into the other deck. Mode is ADJ_IDLE.
    // 3) We are enabling AutoDJ and a single deck is playing. Mode is ADJ_IDLE.
    // 4) We have just completed fading from one deck to another. Mode is
    //    ADJ_IDLE.
    // In all of these cases, it should be safe to skip the bad track in the
    // queue and re-request a track load for the next track. The only case where
    // we request a load-and-play is case #1 currently so we can easily test for
    // this based on the mode.

    // Remove the bad track from the queue.
    removeTrackFromTopOfQueue(pTrack);

    // Load the next track. If we are the first AutoDJ track
    // (ADJ_ENABLE_P1LOADED state) then play the track.
    loadNextTrackFromQueue(*pDeck, m_eState == ADJ_ENABLE_P1LOADED);
}

void AutoDJProcessor::playerTrackUnloaded(DeckAttributes* pDeck, TrackPointer pTrack) {
    if (sDebug) {
        qDebug() << this << "playerTrackUnloaded" << pDeck->group
                 << (pTrack.isNull() ? "(null)" : pTrack->getLocation());
    }
}

void AutoDJProcessor::setTransitionTime(int time) {
    if (sDebug) {
        qDebug() << this << "setTransitionTime" << time;
    }

    // Auto-DJ needs at least two decks
    DEBUG_ASSERT_AND_HANDLE(m_decks.length() > 1) {
        return;
    }

    // Update the transition time first.
    m_pConfig->set(ConfigKey(kConfigKey, kTransitionPreferenceName),
                   ConfigValue(time));
    m_iTransitionTime = time;

    // Then re-calculate fade thresholds for the decks.
    if (m_eState == ADJ_IDLE) {
        DeckAttributes& leftDeck = *m_decks[0];
        DeckAttributes& rightDeck = *m_decks[1];
        if (leftDeck.isPlaying()) {
            calculateTransition(&leftDeck, &rightDeck);
        }
        if (rightDeck.isPlaying()) {
            calculateTransition(&rightDeck, &leftDeck);
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
