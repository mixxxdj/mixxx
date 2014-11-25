#include "library/autodj/autodjprocessor.h"

#include "library/trackcollection.h"
#include "controlpushbutton.h"
#include "controlobjectthread.h"
#include "controlobjectslave.h"
#include "playerinfo.h"
#include "util/math.h"

#define kConfigKey "[Auto DJ]"
const char* kTransitionPreferenceName = "Transition";
const int kTransitionPreferenceDefault = 10;

AutoDJProcessor::AutoDJProcessor(QObject* pParent,
                                 ConfigObject<ConfigValue>* pConfig,
                                 int iAutoDJPlaylistId,
                                 TrackCollection* pTrackCollection)
        : QObject(pParent),
          m_pConfig(pConfig),
          m_pAutoDJTableModel(NULL),
          m_eState(ADJ_DISABLED),
          m_posThreshold1(1.0),
          m_posThreshold2(1.0),
          m_fadeDuration1(0.0),
          m_fadeDuration2(0.0),
          m_iTransitionTime(kTransitionPreferenceDefault),
          m_iBackupTransitionTime(kTransitionPreferenceDefault) {
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

    // playposition is from 0 to 1 (though out of range sets are allowed)
    m_pCOPlayPos1 = new ControlObjectThread("[Channel1]", "playposition");
    m_pCOPlayPos2 = new ControlObjectThread("[Channel2]", "playposition");
    m_pCOPlay1 = new ControlObjectThread("[Channel1]", "play");
    m_pCOPlay2 = new ControlObjectThread("[Channel2]", "play");
    m_pCORepeat1 = new ControlObjectSlave("[Channel1]", "repeat");
    m_pCORepeat2 = new ControlObjectSlave("[Channel2]", "repeat");
    m_pCOCrossfader = new ControlObjectSlave("[Master]", "crossfader");
    m_pCOCrossfaderReverse = new ControlObjectSlave("[Mixer Profile]", "xFaderReverse");

    QString str_autoDjTransition = m_pConfig->getValueString(
            ConfigKey(kConfigKey, kTransitionPreferenceName));
    if (!str_autoDjTransition.isEmpty()) {
        m_iTransitionTime = str_autoDjTransition.toInt();
        m_iBackupTransitionTime = m_iTransitionTime;
    }
}

AutoDJProcessor::~AutoDJProcessor() {
    delete m_pCOPlayPos1;
    delete m_pCOPlayPos2;
    delete m_pCOPlay1;
    delete m_pCOPlay2;
    delete m_pCORepeat1;
    delete m_pCORepeat2;
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
    if (m_eState == ADJ_IDLE) {
        double crossfader = getCrossfader();
        if (crossfader <= 0.3 && m_pCOPlay1->get() == 1.0) {
            m_posThreshold1 = m_pCOPlayPos1->get() -
                    ((crossfader + 1.0) / 2 * (m_fadeDuration1));
            // Repeat is disabled by FadeNow but disables auto Fade
            m_pCORepeat1->set(0.0);
        } else if (crossfader >= -0.3 && m_pCOPlay2->get() == 1.0) {
            m_posThreshold2 = m_pCOPlayPos2->get() -
                    ((1.0 - crossfader) / 2 * (m_fadeDuration2));
            // Repeat is disabled by FadeNow but disables auto Fade
            m_pCORepeat2->set(0.0);
        }
    }
    return ADJ_OK;
}

AutoDJProcessor::AutoDJError AutoDJProcessor::skipNext() {
    if (m_eState == ADJ_DISABLED) {
        return ADJ_IS_INACTIVE;
    }

    // Load the next song from the queue.
    if (m_pCOPlay1->get() == 0.0) {
        removeLoadedTrackFromTopOfQueue("[Channel1]");
        loadNextTrackFromQueue();
    } else if (m_pCOPlay2->get() == 0.0) {
        removeLoadedTrackFromTopOfQueue("[Channel2]");
        loadNextTrackFromQueue();
    }
    return ADJ_OK;
}

AutoDJProcessor::AutoDJError AutoDJProcessor::toggleAutoDJ(bool enable) {
    bool deck1Playing = m_pCOPlay1->get() == 1.0;
    bool deck2Playing = m_pCOPlay2->get() == 1.0;

    if (enable) {  // Enable Auto DJ
        if (deck1Playing && deck2Playing) {
            qDebug() << "One deck must be stopped before enabling Auto DJ mode";
            // Keep the current state.
            emit(autoDJStateChanged(m_eState));
            return ADJ_BOTH_DECKS_PLAYING;
        }

        // Never load the same track if it is already playing
        if (deck1Playing) {
            removeLoadedTrackFromTopOfQueue("[Channel1]");
        }
        if (deck2Playing) {
            removeLoadedTrackFromTopOfQueue("[Channel2]");
        }

        TrackPointer nextTrack = getNextTrackFromQueue();
        if (!nextTrack) {
            qDebug() << "Queue is empty now";
            if (m_pEnabledAutoDJ->get() != 0.0) {
                m_pEnabledAutoDJ->set(0.0);
            }
            emit(autoDJStateChanged(m_eState));
            return ADJ_QUEUE_EMPTY;
        }

        // Track is available so GO
        if (m_pEnabledAutoDJ->get() != 1.0) {
            m_pEnabledAutoDJ->set(1.0);
        }
        qDebug() << "Auto DJ enabled";

        connect(m_pCOPlayPos1, SIGNAL(valueChanged(double)),
                this, SLOT(player1PositionChanged(double)));
        connect(m_pCOPlayPos2, SIGNAL(valueChanged(double)),
                this, SLOT(player2PositionChanged(double)));

        connect(m_pCOPlay1, SIGNAL(valueChanged(double)),
                this, SLOT(player1PlayChanged(double)));
        connect(m_pCOPlay2, SIGNAL(valueChanged(double)),
                this, SLOT(player2PlayChanged(double)));

        if (!deck1Playing && !deck2Playing) {
            // both decks are stopped
            m_eState = ADJ_ENABLE_P1LOADED;
            // Force Update on load Track
            player1PositionChanged(m_pCOPlayPos1->get());
        } else {
            m_eState = ADJ_IDLE;
            if (deck1Playing) {
                // deck 1 is already playing
                player1PlayChanged(1.0);
            } else {
                // deck 2 is already playing
                player2PlayChanged(1.0);
            }
        }
        emit(autoDJStateChanged(m_eState));
        // Loads into first deck If stopped else into second else not
        emit(loadTrack(nextTrack));
    } else {  // Disable Auto DJ
        if (m_pEnabledAutoDJ->get() != 0.0) {
            m_pEnabledAutoDJ->set(0.0);
        }
        qDebug() << "Auto DJ disabled";
        m_eState = ADJ_DISABLED;
        m_pCOPlayPos1->disconnect(this);
        m_pCOPlayPos2->disconnect(this);
        m_pCOPlay1->disconnect(this);
        m_pCOPlay2->disconnect(this);
        m_pCOCrossfader->set(0);
        emit(autoDJStateChanged(m_eState));
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

void AutoDJProcessor::player1PositionChanged(double value) {
    // 95% playback is when we crossfade and do stuff
    // const double posThreshold = 0.95;

    // 0.05; // 5% playback is crossfade duration
    const double fadeDuration = m_fadeDuration1;

    // qDebug() << "player1PositionChanged(" << value << ")";
    if (m_eState == ADJ_DISABLED) {
        //nothing to do
        return;
    }

    bool deck1Playing = m_pCOPlay1->get() > 0.0;
    bool deck2Playing = m_pCOPlay2->get() > 0.0;

    if (m_eState == ADJ_ENABLE_P1LOADED) {
        // Auto DJ Start
        if (!deck1Playing && !deck2Playing) {
            setCrossfader(-1.0, false);  // Move crossfader to the left!
            m_pCOPlay1->set(1.0);  // Play the track in player 1
            removeLoadedTrackFromTopOfQueue("[Channel1]");
        } else {
            m_eState = ADJ_IDLE;
            if (deck1Playing && !deck2Playing) {
                // Here we are, if first deck was playing before starting Auto DJ
                // or if it was started just before
                loadNextTrackFromQueue();
                // if we start the deck from code we don`t get a signal
                player1PlayChanged(1.0);
                // call function manually
            } else {
                player2PlayChanged(1.0);
            }
            emit(autoDJStateChanged(m_eState));
        }
        return;
    }

    if (m_eState == ADJ_P2FADING) {
        if (deck1Playing && !deck2Playing) {
            // End State
            setCrossfader(-1.0, false);  // Move crossfader to the left!
            m_eState = ADJ_IDLE;
            loadNextTrackFromQueue();
            emit(autoDJStateChanged(m_eState));
        }
        return;
    }

    if (m_eState == ADJ_IDLE) {
        if (m_pCORepeat1->get() == 1.0) {
            // repeat disables auto DJ
            // TODO(rryan): So why doesn't this disable Auto DJ?
            return;
        }
    }

    if (value >= m_posThreshold1) {
        if (m_eState == ADJ_IDLE &&
            (deck1Playing || m_posThreshold1 >= 1.0)) {
            if (!deck2Playing) {
                // Start Deck 2
                player2PlayChanged(1.0);
                m_pCOPlay2->set(1.0);
                if (fadeDuration < 0.0) {
                    // Scroll back for pause between tracks
                    m_pCOPlayPos2->set(m_fadeDuration2);
                }
            }
            removeLoadedTrackFromTopOfQueue("[Channel2]");
            m_eState = ADJ_P1FADING;
            emit(autoDJStateChanged(m_eState));
        }

        double posFadeEnd = math_min(1.0, m_posThreshold1 + fadeDuration);

        if (value >= posFadeEnd) {
            // Pre-EndState

            m_pCOPlay1->set(0.0);  // Stop the player
            //m_posThreshold = 1.0 - fadeDuration; // back to default

            // does not work always immediately after stop
            // loadNextTrackFromQueue();
            // m_eState = ADJ_IDLE; // Fading ready
        } else {
            // Crossfade!
            double crossfadeValue = -1.0 +
                    2*(value-m_posThreshold1)/(posFadeEnd-m_posThreshold1);
            // crossfadeValue = -1.0 -> + 1.0
            // Move crossfader to the right!
            setCrossfader(crossfadeValue, true);
        }
    }
}

void AutoDJProcessor::player2PositionChanged(double value) {
    // 95% playback is when we crossfade and do stuff
    // const double posThreshold = 0.95;

    // 0.05; // 5% playback is crossfade duration
    double fadeDuration = m_fadeDuration2;

    //qDebug() << "player2PositionChanged(" << value << ")";
    if (m_eState == ADJ_DISABLED) {
        //nothing to do
        return;
    }

    bool deck1Playing = m_pCOPlay1->get() == 1.0;
    bool deck2Playing = m_pCOPlay2->get() == 1.0;

    if (m_eState == ADJ_P1FADING) {
        if (!deck1Playing && deck2Playing) {
            // End State
            // Move crossfader to the right!
            setCrossfader(1.0, true);
            m_eState = ADJ_IDLE;
            loadNextTrackFromQueue();
            emit(autoDJStateChanged(m_eState));
        }
        return;
    }

    if (m_eState == ADJ_IDLE) {
        if (m_pCORepeat2->get() == 1.0) {
            //repeat disables auto DJ
            // TODO(rryan): So why doesn't this disable Auto DJ?
            return;
        }
    }

    if (value >= m_posThreshold2) {
        if (m_eState == ADJ_IDLE &&
            (deck2Playing || m_posThreshold2 >= 1.0)) {
            if (!deck1Playing) {
                player1PlayChanged(1.0);
                m_pCOPlay1->set(1.0);
                if (fadeDuration < 0) {
                    // Scroll back for pause between tracks
                    m_pCOPlayPos1->set(m_fadeDuration1);
                }
            }
            removeLoadedTrackFromTopOfQueue("[Channel1]");
            m_eState = ADJ_P2FADING;
            emit(autoDJStateChanged(m_eState));
        }

        double posFadeEnd = math_min(1.0, m_posThreshold2 + fadeDuration);

        if (value >= posFadeEnd) {
            // Pre-End State

            m_pCOPlay2->set(0.0);  // Stop the player

            //m_posThreshold = 1.0 - fadeDuration; // back to default

            // does not work always immediately after stop
            // loadNextTrackFromQueue();
            // m_eState = ADJ_IDLE; // Fading ready
        } else {
            //Crossfade!
            double crossfadeValue = 1.0 -
                    2*(value-m_posThreshold2)/(posFadeEnd-m_posThreshold2);
            // crossfadeValue = 1.0 -> + -1.0
            setCrossfader(crossfadeValue, false); // Move crossfader to the left!
        }
    }
}

TrackPointer AutoDJProcessor::getNextTrackFromQueue() {
    int tmp = m_iBackupTransitionTime;
    // This will also signal valueChanged and by that change
    // m_iBackupTransitionTime so we need to copy to orignal value back
    m_iTransitionTime = m_iBackupTransitionTime;
    emit(transitionTimeChanged(m_iTransitionTime));
    m_iBackupTransitionTime = tmp;

    // Get the track at the top of the playlist...
    while (true) {
        TrackPointer nextTrack = m_pAutoDJTableModel->getTrack(
            m_pAutoDJTableModel->index(0, 0));

        if (nextTrack) {
            if (nextTrack->exists()) {
                // found a valid Track
                if (nextTrack->getDuration() < m_iBackupTransitionTime) {
                    m_iTransitionTime = nextTrack->getDuration()/2;
                    m_iBackupTransitionTime = tmp;
                    emit(transitionTimeChanged(m_iTransitionTime));
                }
                return nextTrack;
            } else {
                // Remove missing song from auto DJ playlist
                m_pAutoDJTableModel->removeTrack(
                        m_pAutoDJTableModel->index(0, 0));
            }
        } else {
            // we are running out of tracks
            return nextTrack;
        }
    }
}

bool AutoDJProcessor::loadNextTrackFromQueue() {
    TrackPointer nextTrack = getNextTrackFromQueue();

    // We ran out of tracks in the queue...
    if (!nextTrack) {
        // Disable auto DJ and return...
        m_eState = ADJ_DISABLED;
        emit(autoDJStateChanged(m_eState));
        // And eject track as "End of auto DJ warning"
        emit(loadTrack(nextTrack));
        return false;
    }

    emit(loadTrack(nextTrack));
    return true;
}

bool AutoDJProcessor::removeLoadedTrackFromTopOfQueue(const QString& group) {
    // Get the track id at the top of the playlist.
    int nextId = m_pAutoDJTableModel->getTrackId(
            m_pAutoDJTableModel->index(0, 0));

    // No track at the top of the queue. Bail.
    if (nextId == -1) {
        return false;
    }

    // Get loaded track for this group.
    TrackPointer loadedTrack = PlayerInfo::instance().getTrackInfo(group);

    // No loaded track in this group.
    if (loadedTrack.isNull()) {
        return false;
    }

    int loadedId = loadedTrack->getId();

    // Loaded track is not a library track.
    if (loadedId == -1) {
        return false;
    }

    // If the loaded track is not the next track in the queue then do nothing.
    if (loadedId != nextId) {
        return false;
    }

    // Remove the top track.
    m_pAutoDJTableModel->removeTrack(m_pAutoDJTableModel->index(0, 0));

    // Re-queue if configured.
    if (m_pConfig->getValueString(ConfigKey(kConfigKey, "Requeue")).toInt()) {
        m_pAutoDJTableModel->appendTrack(loadedId);
    }
    return true;
}

void AutoDJProcessor::player1PlayChanged(double value) {
    //qDebug() << "player1PlayChanged(" << value << ")";
    if (value > 0.0 && m_eState == ADJ_IDLE) {
        TrackPointer loadedTrack =
                PlayerInfo::instance().getTrackInfo("[Channel1]");
        if (loadedTrack) {
            int TrackDuration = loadedTrack->getDuration();
            qDebug() << "TrackDuration = " << TrackDuration;

            // The track might be shorter than the transition period. Use a
            // sensible cap.
            int autoDjTransition = math_min(m_iTransitionTime,
                                            TrackDuration/2);

            if (TrackDuration > autoDjTransition) {
                m_fadeDuration1 = static_cast<double>(autoDjTransition) /
                        static_cast<double>(TrackDuration);
            } else {
                m_fadeDuration1 = 0;
            }

            if (autoDjTransition > 0) {
                m_posThreshold1 = 1.0 - m_fadeDuration1;
            } else {
                // in case of pause
                m_posThreshold1 = 1.0;
            }
            qDebug() << "m_fadeDuration1 = " << m_fadeDuration1;
        }
    }
}

void AutoDJProcessor::player2PlayChanged(double value) {
    //qDebug() << "player2PlayChanged(" << value << ")";
    if (value > 0 && m_eState == ADJ_IDLE) {
        TrackPointer loadedTrack =
                PlayerInfo::instance().getTrackInfo("[Channel2]");
        if (loadedTrack) {
            int TrackDuration = loadedTrack->getDuration();
            qDebug() << "TrackDuration = " << TrackDuration;

            // The track might be shorter than the transition period. Use a
            // sensibile cap.
            int autoDjTransition = math_min(m_iTransitionTime,
                                            TrackDuration/2);

            if (TrackDuration > autoDjTransition) {
                m_fadeDuration2 = static_cast<double>(autoDjTransition) /
                        static_cast<double>(TrackDuration);
            } else {
                m_fadeDuration2 = 0;
            }

            if (autoDjTransition > 0) {
                m_posThreshold2 = 1.0 - m_fadeDuration2;
            } else {
                // in case of pause
                m_posThreshold2 = 1.0;
            }
            qDebug() << "m_fadeDuration2 = " << m_fadeDuration2;
        }
    }
}

void AutoDJProcessor::setTransitionTime(int time) {
    if (m_eState == ADJ_IDLE) {
        if (m_pCOPlay1->get() > 0.0) {
            player1PlayChanged(1.0);
        }
        if (m_pCOPlay2->get() > 0.0) {
            player2PlayChanged(1.0);
        }
    }
    m_pConfig->set(ConfigKey(kConfigKey, kTransitionPreferenceName),
                   ConfigValue(time));
    m_iTransitionTime = time;
    m_iBackupTransitionTime = time;
}
