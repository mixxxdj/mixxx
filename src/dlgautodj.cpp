#include <QSqlTableModel>

#include "dlgautodj.h"

#include "controlobject.h"
#include "controlobjectslave.h"
#include "library/playlisttablemodel.h"
#include "library/trackcollection.h"
#include "playerinfo.h"
#include "widget/wskincolor.h"
#include "widget/wtracktableview.h"
#include "widget/wwidget.h"
#include "util/math.h"

#define CONFIG_KEY "[Auto DJ]"
const char* kTransitionPreferenceName = "Transition";
const int kTransitionPreferenceDefault = 10;

DlgAutoDJ::DlgAutoDJ(QWidget* parent, ConfigObject<ConfigValue>* pConfig,
                     TrackCollection* pTrackCollection,
                     MixxxKeyboard* pKeyboard)
        : QWidget(parent),
          Ui::DlgAutoDJ(),
          m_pConfig(pConfig),
          m_pTrackCollection(pTrackCollection),
          m_pTrackTableView(
              new WTrackTableView(this, pConfig, m_pTrackCollection, false)), // no sorting
          m_bFadeNow(false),
          m_eState(ADJ_DISABLED),
          m_posThreshold1(1.0f),
          m_posThreshold2(1.0f),
          m_fadeDuration1(0.0f),
          m_fadeDuration2(0.0f) {
    setupUi(this);

    m_pTrackTableView->installEventFilter(pKeyboard);
    connect(m_pTrackTableView, SIGNAL(loadTrack(TrackPointer)),
            this, SIGNAL(loadTrack(TrackPointer)));
    connect(m_pTrackTableView, SIGNAL(loadTrackToPlayer(TrackPointer, QString, bool)),
            this, SIGNAL(loadTrackToPlayer(TrackPointer, QString, bool)));

    QBoxLayout* box = dynamic_cast<QBoxLayout*>(layout());
    Q_ASSERT(box); //Assumes the form layout is a QVBox/QHBoxLayout!
    box->removeWidget(m_pTrackTablePlaceholder);
    m_pTrackTablePlaceholder->hide();
    box->insertWidget(1, m_pTrackTableView);

    m_pAutoDJTableModel = new PlaylistTableModel(this, pTrackCollection,
                                                 "mixxx.db.model.autodj");
    PlaylistDAO& playlistDao = pTrackCollection->getPlaylistDAO();
    int playlistId = playlistDao.getPlaylistIdFromName(AUTODJ_TABLE);
    if (playlistId < 0) {
        playlistId = playlistDao.createPlaylist(AUTODJ_TABLE,
                                                PlaylistDAO::PLHT_AUTO_DJ);
    }
    m_pAutoDJTableModel->setTableModel(playlistId);
    m_pTrackTableView->loadTrackModel(m_pAutoDJTableModel);

    // Override some playlist-view properties:

    // Do not set this because it disables auto-scrolling
    //m_pTrackTableView->setDragDropMode(QAbstractItemView::InternalMove);

    pushButtonFadeNow->setEnabled(false);
    pushButtonSkipNext->setEnabled(false);

    m_pCOShufflePlaylist = new ControlPushButton(
            ConfigKey("[AutoDJ]", "shuffle_playlist"));
    m_pCOTShufflePlaylist = new ControlObjectThread(m_pCOShufflePlaylist->getKey());
    connect(m_pCOTShufflePlaylist, SIGNAL(valueChanged(double)),
            this, SLOT(shufflePlaylist(double)));
    connect(pushButtonShuffle, SIGNAL(clicked(bool)),
            this, SLOT(shufflePlaylistButton(bool)));

    m_pCOSkipNext = new ControlPushButton(
            ConfigKey("[AutoDJ]", "skip_next"));
    m_pCOTSkipNext = new ControlObjectThread(m_pCOSkipNext->getKey());
    connect(m_pCOTSkipNext, SIGNAL(valueChanged(double)),
            this, SLOT(skipNext(double)));
    connect(pushButtonSkipNext, SIGNAL(clicked(bool)),
            this, SLOT(skipNextButton(bool)));

#ifdef __AUTODJCRATES__
    connect(pushButtonAddRandom, SIGNAL(clicked(bool)),
            this, SIGNAL(addRandomButton(bool)));
#else // __AUTODJCRATES__
    pushButtonAddRandom->setVisible(false);
    horizontalLayout->removeWidget(pushButtonAddRandom);
#endif // __AUTODJCRATES__

    m_pCOFadeNow = new ControlPushButton(
            ConfigKey("[AutoDJ]", "fade_now"));
    m_pCOTFadeNow = new ControlObjectThread(m_pCOFadeNow->getKey());
    connect(m_pCOTFadeNow, SIGNAL(valueChanged(double)),
            this, SLOT(fadeNow(double)));
    connect(pushButtonFadeNow, SIGNAL(clicked(bool)),
            this, SLOT(fadeNowButton(bool)));

    connect(spinBoxTransition, SIGNAL(valueChanged(int)),
            this, SLOT(transitionValueChanged(int)));

    connect(pushButtonAutoDJ, SIGNAL(toggled(bool)),
            this, SLOT(toggleAutoDJButton(bool)));

    m_pCOEnabledAutoDJ = new ControlPushButton(
            ConfigKey("[AutoDJ]", "enabled"));
    m_pCOEnabledAutoDJ->setButtonMode(ControlPushButton::TOGGLE);
    m_pCOTEnabledAutoDJ = new ControlObjectThread(m_pCOEnabledAutoDJ->getKey());
    connect(m_pCOTEnabledAutoDJ, SIGNAL(valueChanged(double)),
            this, SLOT(enableAutoDJCo(double)));

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
        ConfigKey(CONFIG_KEY, kTransitionPreferenceName));
    if (str_autoDjTransition.isEmpty()) {
        spinBoxTransition->setValue(kTransitionPreferenceDefault);
    } else {
        spinBoxTransition->setValue(str_autoDjTransition.toInt());
    }
    m_backUpTransition = spinBoxTransition->value();
}

DlgAutoDJ::~DlgAutoDJ() {
    qDebug() << "~DlgAutoDJ()";
    delete m_pCOPlayPos1;
    delete m_pCOPlayPos2;
    delete m_pCOPlay1;
    delete m_pCOPlay2;
    delete m_pCORepeat1;
    delete m_pCORepeat2;
    delete m_pCOCrossfader;
    delete m_pCOCrossfaderReverse;
    delete m_pCOSkipNext;
    delete m_pCOShufflePlaylist;
    delete m_pCOEnabledAutoDJ;
    delete m_pCOFadeNow;
    delete m_pCOTSkipNext;
    delete m_pCOTShufflePlaylist;
    delete m_pCOTEnabledAutoDJ;
    delete m_pCOTFadeNow;
    // Delete m_pTrackTableView before the table model. This is because the
    // table view saves the header state using the model.
    delete m_pTrackTableView;
    delete m_pAutoDJTableModel;
}

void DlgAutoDJ::onShow() {
    m_pAutoDJTableModel->select();
}

void DlgAutoDJ::onSearch(const QString& text) {
    // Do not allow filtering the Auto DJ playlist, because
    // Auto DJ will work from the filtered table
    Q_UNUSED(text);
}

double DlgAutoDJ::getCrossfader() const {
    if (m_pCOCrossfaderReverse->toBool()) {
        return m_pCOCrossfader->get() * -1.0;
    }
    return m_pCOCrossfader->get();
}

void DlgAutoDJ::setCrossfader(double value, bool right) {
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

void DlgAutoDJ::loadSelectedTrack() {
    m_pTrackTableView->loadSelectedTrack();
}

void DlgAutoDJ::loadSelectedTrackToGroup(QString group, bool play) {
    m_pTrackTableView->loadSelectedTrackToGroup(group, play);
}

void DlgAutoDJ::moveSelection(int delta) {
    m_pTrackTableView->moveSelection(delta);
}

void DlgAutoDJ::shufflePlaylistButton(bool) {
    // Activate regardless of button being checked
    shufflePlaylist(1.0);
}

void DlgAutoDJ::shufflePlaylist(double value) {
    if (value <= 0.0) {
        return;
    }
    QModelIndexList IndexList = m_pTrackTableView->selectionModel()->selectedRows();
    QModelIndex exclude;
    if (m_eState != ADJ_DISABLED) {
        exclude = m_pAutoDJTableModel->index(0, 0);
    }
    m_pAutoDJTableModel->shuffleTracks(IndexList, exclude);
}

void DlgAutoDJ::skipNextButton(bool) {
    // Activate regardless of button being checked
    skipNext(1.0);
}

void DlgAutoDJ::skipNext(double value) {
    if (value <= 0.0 || m_eState == ADJ_DISABLED) {
        return;
    }
    // Load the next song from the queue.
    if (m_pCOPlay1->get() == 0.0) {
        removePlayingTrackFromQueue("[Channel1]");
        loadNextTrackFromQueue();
    } else if (m_pCOPlay2->get() == 0.0) {
        removePlayingTrackFromQueue("[Channel2]");
        loadNextTrackFromQueue();
    }
}

void DlgAutoDJ::fadeNowButton(bool) {
    // Activate regardless of button being checked
    fadeNow(1.0);
}

void DlgAutoDJ::fadeNow(double value) {
    if (value <= 0.0) {
        return;
    }
    if (m_eState == ADJ_IDLE) {
        m_bFadeNow = true;
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
}

void DlgAutoDJ::toggleAutoDJButton(bool enable) {
    bool deck1Playing = m_pCOPlay1->get() == 1.0;
    bool deck2Playing = m_pCOPlay2->get() == 1.0;

    if (enable) {  // Enable Auto DJ
        if (deck1Playing && deck2Playing) {
            QMessageBox::warning(
                NULL, tr("Auto-DJ"),
                tr("One deck must be stopped to enable Auto-DJ mode."),
                QMessageBox::Ok);
            qDebug() << "One deck must be stopped before enabling Auto DJ mode";
            pushButtonAutoDJ->setChecked(false);
            return;
        }

        // Never load the same track if it is already playing
        if (deck1Playing) {
            removePlayingTrackFromQueue("[Channel1]");
        }
        if (deck2Playing) {
            removePlayingTrackFromQueue("[Channel2]");
        }

        TrackPointer nextTrack = getNextTrackFromQueue();
        if (!nextTrack) {
            qDebug() << "Queue is empty now";
            pushButtonAutoDJ->setChecked(false);
            if (m_pCOTEnabledAutoDJ->get() != 0.0) {
                m_pCOTEnabledAutoDJ->set(0.0);
            }
            return;
        }

        // Track is available so GO
        pushButtonAutoDJ->setToolTip(tr("Disable Auto DJ"));
        pushButtonAutoDJ->setText(tr("Disable Auto DJ"));
        if (m_pCOTEnabledAutoDJ->get() != 1.0) {
            m_pCOTEnabledAutoDJ->set(1.0);
        }
        qDebug() << "Auto DJ enabled";

        pushButtonSkipNext->setEnabled(true);

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
            pushButtonFadeNow->setEnabled(false);
            // Force Update on load Track
            player1PositionChanged(m_pCOPlayPos1->get());
        } else {
            m_eState = ADJ_IDLE;
            pushButtonFadeNow->setEnabled(true);
            if (deck1Playing) {
                // deck 1 is already playing
                player1PlayChanged(1.0);
            } else {
                // deck 2 is already playing
                player2PlayChanged(1.0);
            }
        }
        // Loads into first deck If stopped else into second else not
        emit(loadTrack(nextTrack));
    } else {  // Disable Auto DJ
        pushButtonAutoDJ->setToolTip(tr("Enable Auto DJ"));
        pushButtonAutoDJ->setText(tr("Enable Auto DJ"));
        if (m_pCOTEnabledAutoDJ->get() != 0.0) {
            m_pCOTEnabledAutoDJ->set(0.0);
        }
        qDebug() << "Auto DJ disabled";
        m_eState = ADJ_DISABLED;
        pushButtonFadeNow->setEnabled(false);
        pushButtonSkipNext->setEnabled(false);
        m_bFadeNow = false;
        m_pCOPlayPos1->disconnect(this);
        m_pCOPlayPos2->disconnect(this);
        m_pCOPlay1->disconnect(this);
        m_pCOPlay2->disconnect(this);
        m_pCOCrossfader->set(0);
    }
}

void DlgAutoDJ::enableAutoDJCo(double value) {
    bool enable = (bool)value;
    pushButtonAutoDJ->setChecked(enable);
    toggleAutoDJButton(enable);
}

void DlgAutoDJ::player1PositionChanged(double value) {
    // 95% playback is when we crossfade and do stuff
    // const float posThreshold = 0.95;

    // 0.05; // 5% playback is crossfade duration
    const float fadeDuration = m_fadeDuration1;

    // qDebug() << "player1PositionChanged(" << value << ")";
    if (m_eState == ADJ_DISABLED) {
        //nothing to do
        return;
    }

    bool deck1Playing = m_pCOPlay1->get() == 1.0;
    bool deck2Playing = m_pCOPlay2->get() == 1.0;

    if (m_eState == ADJ_ENABLE_P1LOADED) {
        // Auto DJ Start
        if (!deck1Playing && !deck2Playing) {
            setCrossfader(-1.0, false);  // Move crossfader to the left!
            m_pCOPlay1->set(1.0);  // Play the track in player 1
            removePlayingTrackFromQueue("[Channel1]");
        } else {
            m_eState = ADJ_IDLE;
            pushButtonFadeNow->setEnabled(true);
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
        }
        return;
    }

    if (m_eState == ADJ_P2FADING) {
        if (deck1Playing && !deck2Playing) {
            // End State
            setCrossfader(-1.0, false);  // Move crossfader to the left!
            m_eState = ADJ_IDLE;
            pushButtonFadeNow->setEnabled(true);
            loadNextTrackFromQueue();
        }
        return;
    }

    if (m_eState == ADJ_IDLE) {
        if (m_pCORepeat1->get() == 1.0) {
            // repeat disables auto DJ
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
            removePlayingTrackFromQueue("[Channel2]");
            m_eState = ADJ_P1FADING;
            pushButtonFadeNow->setEnabled(false);
        }

        float posFadeEnd = math_min(1.0f, m_posThreshold1 + fadeDuration);

        if (value >= posFadeEnd) {
            // Pre-EndState

            m_pCOPlay1->set(0.0);  // Stop the player
            //m_posThreshold = 1.0f - fadeDuration; // back to default

            // does not work always immediately after stop
            // loadNextTrackFromQueue();
            // m_eState = ADJ_IDLE; // Fading ready
        } else {
            // Crossfade!
            float crossfadeValue = -1.0f +
                    2*(value-m_posThreshold1)/(posFadeEnd-m_posThreshold1);
            // crossfadeValue = -1.0f -> + 1.0f
            // Move crossfader to the right!
            setCrossfader(crossfadeValue, true);
        }
    }
}

void DlgAutoDJ::player2PositionChanged(double value) {
    // 95% playback is when we crossfade and do stuff
    // const float posThreshold = 0.95;

    // 0.05; // 5% playback is crossfade duration
    float fadeDuration = m_fadeDuration2;

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
            pushButtonFadeNow->setEnabled(true);
            loadNextTrackFromQueue();
        }
        return;
    }

    if (m_eState == ADJ_IDLE) {
        if (m_pCORepeat2->get() == 1.0) {
            //repeat disables auto DJ
            return;
        }
    }

    if (value >= m_posThreshold2) {
        if (m_eState == ADJ_IDLE &&
            (deck2Playing || m_posThreshold2 >= 1.0f)) {
            if (!deck1Playing) {
                player1PlayChanged(1.0);
                m_pCOPlay1->set(1.0);
                if (fadeDuration < 0) {
                    // Scroll back for pause between tracks
                    m_pCOPlayPos1->set(m_fadeDuration1);
                }
            }
            removePlayingTrackFromQueue("[Channel1]");
            m_eState = ADJ_P2FADING;
            pushButtonFadeNow->setEnabled(false);
        }

        float posFadeEnd = math_min(1.0f, m_posThreshold2 + fadeDuration);

        if (value >= posFadeEnd) {
            // Pre-End State

            m_pCOPlay2->set(0.0f);  // Stop the player

            //m_posThreshold = 1.0f - fadeDuration; // back to default

            // does not work always immediately after stop
            // loadNextTrackFromQueue();
            // m_eState = ADJ_IDLE; // Fading ready
        } else {
            //Crossfade!
            float crossfadeValue = 1.0f -
                    2*(value-m_posThreshold2)/(posFadeEnd-m_posThreshold2);
            // crossfadeValue = 1.0f -> + -1.0f
            setCrossfader(crossfadeValue, false); // Move crossfader to the left!
        }
    }
}

TrackPointer DlgAutoDJ::getNextTrackFromQueue() {
    // Get the track at the top of the playlist...
    TrackPointer nextTrack;
    int tmp = m_backUpTransition;
    // This will also signal valueChanged and by that change m_backUpTransition
    // so we need to copy to orignal value back
    spinBoxTransition->setValue(m_backUpTransition);
    m_backUpTransition = tmp;

    while (true) {
        nextTrack = m_pAutoDJTableModel->getTrack(
            m_pAutoDJTableModel->index(0, 0));

        if (nextTrack) {
            if (nextTrack->exists()) {
                // found a valid Track
                if (nextTrack->getDuration() < m_backUpTransition) {
                    spinBoxTransition->setValue(nextTrack->getDuration()/2);
                    m_backUpTransition = tmp;
                }
                return nextTrack;
            } else {
                // Remove missing song from auto DJ playlist
                m_pAutoDJTableModel->removeTrack(
                m_pAutoDJTableModel->index(0, 0));
            }
        } else {
            // we are running out of tracks
            break;
        }
    }
    return nextTrack;
}

bool DlgAutoDJ::loadNextTrackFromQueue() {
    TrackPointer nextTrack = getNextTrackFromQueue();

    // We ran out of tracks in the queue...
    if (!nextTrack) {
        // Disable auto DJ and return...
        pushButtonAutoDJ->setChecked(false);
        // And eject track as "End of auto DJ warning"
        emit(loadTrack(nextTrack));
        return false;
    }

    emit(loadTrack(nextTrack));
    return true;
}

bool DlgAutoDJ::removePlayingTrackFromQueue(QString group) {
    TrackPointer nextTrack, loadedTrack;
    int nextId = 0, loadedId = 0;

    // Get the track at the top of the playlist...
    nextTrack = m_pAutoDJTableModel->getTrack(m_pAutoDJTableModel->index(0, 0));
    if (nextTrack) {
        nextId = nextTrack->getId();
    }

    // Get loaded track
    loadedTrack = PlayerInfo::instance().getTrackInfo(group);
    if (loadedTrack) {
        loadedId = loadedTrack->getId();
    }

    // When enable auto DJ and Topmost Song is already on second deck, nothing to do
    //BaseTrackPlayer::getLoadedTrack()
    //pTrack = PlayerInfo::Instance().getCurrentPlayingTrack();

    if (loadedId != nextId) {
        // Do not remove when the user has loaded a track manually
        return false;
    }

    // remove the top track
    m_pAutoDJTableModel->removeTrack(m_pAutoDJTableModel->index(0, 0));

    // Re-queue if configured
    if (m_pConfig->getValueString(ConfigKey(CONFIG_KEY, "Requeue")).toInt()) {
        m_pAutoDJTableModel->appendTrack(loadedId);
    }
    return true;
}

void DlgAutoDJ::player1PlayChanged(double value) {
    //qDebug() << "player1PlayChanged(" << value << ")";
    if (value == 1.0 && m_eState == ADJ_IDLE) {
        TrackPointer loadedTrack =
                PlayerInfo::instance().getTrackInfo("[Channel1]");
        if (loadedTrack) {
            int TrackDuration = loadedTrack->getDuration();
            qDebug() << "TrackDuration = " << TrackDuration;

            // The track might be shorter than the transition period. Use a
            // sensible cap.
            int autoDjTransition = math_min(spinBoxTransition->value(),
                                            TrackDuration/2);

            if (TrackDuration > autoDjTransition) {
                m_fadeDuration1 = static_cast<float>(autoDjTransition) /
                        static_cast<float>(TrackDuration);
            } else {
                m_fadeDuration1 = 0;
            }

            if (autoDjTransition > 0) {
                m_posThreshold1 = 1.0f - m_fadeDuration1;
            } else {
                // in case of pause
                m_posThreshold1 = 1.0f;
            }
            qDebug() << "m_fadeDuration1 = " << m_fadeDuration1;
        }
    }
}

void DlgAutoDJ::player2PlayChanged(double value) {
    //qDebug() << "player2PlayChanged(" << value << ")";
    if (value == 1.0f && m_eState == ADJ_IDLE) {
        TrackPointer loadedTrack =
                PlayerInfo::instance().getTrackInfo("[Channel2]");
        if (loadedTrack) {
            int TrackDuration = loadedTrack->getDuration();
            qDebug() << "TrackDuration = " << TrackDuration;

            // The track might be shorter than the transition period. Use a
            // sensibile cap.
            int autoDjTransition = math_min(spinBoxTransition->value(),
                                            TrackDuration/2);

            if (TrackDuration > autoDjTransition) {
                m_fadeDuration2 = static_cast<float>(autoDjTransition) /
                        static_cast<float>(TrackDuration);
            } else {
                m_fadeDuration2 = 0;
            }

            if (autoDjTransition > 0) {
                m_posThreshold2 = 1.0f - m_fadeDuration2;
            } else {
                // in case of pause
                m_posThreshold2 = 1.0f;
            }
            qDebug() << "m_fadeDuration2 = " << m_fadeDuration2;
        }
    }
}

void DlgAutoDJ::transitionValueChanged(int value) {
    if (m_eState == ADJ_IDLE) {
        if (m_pCOPlay1->get() == 1.0) {
            player1PlayChanged(1.0);
        }
        if (m_pCOPlay2->get() == 1.0) {
            player2PlayChanged(1.0);
        }
    }
    m_pConfig->set(ConfigKey(CONFIG_KEY, kTransitionPreferenceName),
                   ConfigValue(value));
    m_backUpTransition = value;
}

void DlgAutoDJ::enableRandomButton(bool enabled) {
#ifdef __AUTODJCRATES__
    pushButtonAddRandom->setEnabled(enabled);
#endif // __AUTODJCRATES__
}
