// cuecontrol.cpp
// Created 11/5/2009 by RJ Ryan (rryan@mit.edu)

#include <QMutexLocker>
#include <QStringBuilder>

#include "engine/enginebuffer.h"
#include "engine/controls/cuecontrol.h"

#include "control/controlobject.h"
#include "control/controlpushbutton.h"
#include "control/controlindicator.h"
#include "vinylcontrol/defs_vinylcontrol.h"
#include "util/sample.h"

// TODO: Convert these doubles to a standard enum
// and convert elseif logic to switch statements
static const double CUE_MODE_MIXXX = 0.0;
static const double CUE_MODE_PIONEER = 1.0;
static const double CUE_MODE_DENON = 2.0;
static const double CUE_MODE_NUMARK = 3.0;
static const double CUE_MODE_MIXXX_NO_BLINK = 4.0;
static const double CUE_MODE_CUP = 5.0;

CueControl::CueControl(QString group,
                       UserSettingsPointer pConfig) :
        EngineControl(group, pConfig),
        m_bPreviewing(false),
        // m_pPlay->toBoo() -> engine play state
        // m_pPlay->set(1.0) -> emulate play button press
        m_pPlay(ControlObject::getControl(ConfigKey(group, "play"))),
        m_pStopButton(ControlObject::getControl(ConfigKey(group, "stop"))),
        m_iCurrentlyPreviewingHotcues(0),
        m_bypassCueSetByPlay(false),
        m_iNumHotCues(NUM_HOT_CUES),
        m_pLoadedTrack(),
        m_mutex(QMutex::Recursive) {
    // To silence a compiler warning about CUE_MODE_PIONEER.
    Q_UNUSED(CUE_MODE_PIONEER);
    createControls();

    m_pTrackSamples = ControlObject::getControl(ConfigKey(group, "track_samples"));

    m_pQuantizeEnabled = ControlObject::getControl(ConfigKey(group, "quantize"));

    m_pNextBeat = ControlObject::getControl(ConfigKey(group, "beat_next"));
    m_pClosestBeat = ControlObject::getControl(ConfigKey(group, "beat_closest"));

    m_pCuePoint = new ControlObject(ConfigKey(group, "cue_point"));
    m_pCuePoint->set(-1.0);

    m_pCueMode = new ControlObject(ConfigKey(group, "cue_mode"));

    m_pCueSet = new ControlPushButton(ConfigKey(group, "cue_set"));
    m_pCueSet->setButtonMode(ControlPushButton::TRIGGER);
    connect(m_pCueSet, &ControlObject::valueChanged,
            this, &CueControl::cueSet,
            Qt::DirectConnection);

    m_pCueGoto = new ControlPushButton(ConfigKey(group, "cue_goto"));
    connect(m_pCueGoto, &ControlObject::valueChanged,
            this, &CueControl::cueGoto,
            Qt::DirectConnection);

    m_pCueGotoAndPlay =
            new ControlPushButton(ConfigKey(group, "cue_gotoandplay"));
    connect(m_pCueGotoAndPlay, &ControlObject::valueChanged,
            this, &CueControl::cueGotoAndPlay,
            Qt::DirectConnection);

    m_pCuePlay =
            new ControlPushButton(ConfigKey(group, "cue_play"));
    connect(m_pCuePlay, &ControlObject::valueChanged,
            this, &CueControl::cuePlay,
            Qt::DirectConnection);

    m_pCueGotoAndStop =
            new ControlPushButton(ConfigKey(group, "cue_gotoandstop"));
    connect(m_pCueGotoAndStop, &ControlObject::valueChanged,
            this, &CueControl::cueGotoAndStop,
            Qt::DirectConnection);

    m_pCuePreview = new ControlPushButton(ConfigKey(group, "cue_preview"));
    connect(m_pCuePreview, &ControlObject::valueChanged,
            this, &CueControl::cuePreview,
            Qt::DirectConnection);

    m_pCueCDJ = new ControlPushButton(ConfigKey(group, "cue_cdj"));
    connect(m_pCueCDJ, &ControlObject::valueChanged,
            this, &CueControl::cueCDJ,
            Qt::DirectConnection);

    m_pCueDefault = new ControlPushButton(ConfigKey(group, "cue_default"));
    connect(m_pCueDefault, &ControlObject::valueChanged,
            this, &CueControl::cueDefault,
            Qt::DirectConnection);

    m_pPlayStutter = new ControlPushButton(ConfigKey(group, "play_stutter"));
    connect(m_pPlayStutter, &ControlObject::valueChanged,
            this, &CueControl::playStutter,
            Qt::DirectConnection);

    m_pCueIndicator = new ControlIndicator(ConfigKey(group, "cue_indicator"));
    m_pPlayIndicator = new ControlIndicator(ConfigKey(group, "play_indicator"));

    m_pVinylControlEnabled = new ControlProxy(group, "vinylcontrol_enabled");
    m_pVinylControlMode = new ControlProxy(group, "vinylcontrol_mode");
}

CueControl::~CueControl() {
    delete m_pCuePoint;
    delete m_pCueMode;
    delete m_pCueSet;
    delete m_pCueGoto;
    delete m_pCueGotoAndPlay;
    delete m_pCuePlay;
    delete m_pCueGotoAndStop;
    delete m_pCuePreview;
    delete m_pCueCDJ;
    delete m_pCueDefault;
    delete m_pPlayStutter;
    delete m_pCueIndicator;
    delete m_pPlayIndicator;
    delete m_pVinylControlEnabled;
    delete m_pVinylControlMode;
    qDeleteAll(m_hotcueControls);
}

void CueControl::createControls() {
    for (int i = 0; i < m_iNumHotCues; ++i) {
        HotcueControl* pControl = new HotcueControl(getGroup(), i);

        connect(pControl, &HotcueControl::hotcuePositionChanged,
                this, &CueControl::hotcuePositionChanged,
                Qt::DirectConnection);
        connect(pControl, &HotcueControl::hotcueSet,
                this, &CueControl::hotcueSet,
                Qt::DirectConnection);
        connect(pControl, &HotcueControl::hotcueGoto,
                this, &CueControl::hotcueGoto,
                Qt::DirectConnection);
        connect(pControl, &HotcueControl::hotcueGotoAndPlay,
                this, &CueControl::hotcueGotoAndPlay,
                Qt::DirectConnection);
        connect(pControl, &HotcueControl::hotcueGotoAndStop,
                this, &CueControl::hotcueGotoAndStop,
                Qt::DirectConnection);
        connect(pControl, &HotcueControl::hotcueActivate,
                this, &CueControl::hotcueActivate,
                Qt::DirectConnection);
        connect(pControl, &HotcueControl::hotcueActivatePreview,
                this, &CueControl::hotcueActivatePreview,
                Qt::DirectConnection);
        connect(pControl, &HotcueControl::hotcueClear,
                this, &CueControl::hotcueClear,
                Qt::DirectConnection);

        m_hotcueControls.append(pControl);
    }
}

void CueControl::attachCue(CuePointer pCue, int hotCue) {
    HotcueControl* pControl = m_hotcueControls.value(hotCue, NULL);
    if (pControl == NULL) {
        return;
    }
    if (pControl->getCue() != NULL) {
        detachCue(pControl->getHotcueNumber());
    }
    connect(pCue.get(), &Cue::updated,
            this, &CueControl::cueUpdated,
            Qt::DirectConnection);

    pControl->setCue(pCue);

}

void CueControl::detachCue(int hotCue) {
    HotcueControl* pControl = m_hotcueControls.value(hotCue, NULL);
    if (pControl == NULL) {
        return;
    }
    CuePointer pCue(pControl->getCue());
    if (!pCue)
        return;
    disconnect(pCue.get(), 0, this, 0);
    pControl->resetCue();
}

void CueControl::trackLoaded(TrackPointer pNewTrack) {
    QMutexLocker lock(&m_mutex);
    if (m_pLoadedTrack) {
        disconnect(m_pLoadedTrack.get(), 0, this, 0);
        for (int i = 0; i < m_iNumHotCues; ++i) {
            detachCue(i);
        }

        m_pCueIndicator->setBlinkValue(ControlIndicator::OFF);
        m_pCuePoint->set(-1.0);
        m_pLoadedTrack.reset();
    }

    if (!pNewTrack) {
        return;
    }
    m_pLoadedTrack = pNewTrack;

    connect(m_pLoadedTrack.get(), &Track::cuesUpdated,
            this, &CueControl::trackCuesUpdated,
            Qt::DirectConnection);

    CuePointer pLoadCue;
    for (const CuePointer& pCue: m_pLoadedTrack->getCuePoints()) {
        if (pCue->getType() == Cue::CUE) {
            continue; // skip
        }
        if (pCue->getType() == Cue::LOAD) {
            DEBUG_ASSERT(!pLoadCue);
            pLoadCue = pCue;
        }
        int hotcue = pCue->getHotCue();
        if (hotcue != -1) {
            attachCue(pCue, hotcue);
        }
    }
    double cuePoint;
    if (pLoadCue) {
        cuePoint = pLoadCue->getPosition();
    } else {
        // If no load cue point is stored, read from track
        cuePoint = m_pLoadedTrack->getCuePoint();
    }
    m_pCuePoint->set(cuePoint);

    // Need to unlock before emitting any signals to prevent deadlock.
    lock.unlock();

    // Use pNewTrack here, because m_pLoadedTrack might have been reset
    // immediately after leaving the locking scope!
    pNewTrack->setCuePoint(cuePoint);

    // If cue recall is ON in the prefs, then we're supposed to seek to the cue
    // point on song load. Note that [Controls],cueRecall == 0 corresponds to "ON", not OFF.
    bool cueRecall = (getConfig()->getValue(
           ConfigKey("[Controls]","CueRecall"), 0) == 0);
    if (cueRecall && (cuePoint >= 0.0)) {
        seekExact(cuePoint);
    } else if (!(m_pVinylControlEnabled->get() &&
            m_pVinylControlMode->get() == MIXXX_VCMODE_ABSOLUTE)) {
        // If cuerecall is off, seek to zero unless
        // vinylcontrol is on and set to absolute.  This allows users to
        // load tracks and have the needle-drop be maintained.
        seekExact(0.0);
    }

    trackCuesUpdated();
}

void CueControl::cueUpdated() {
    //QMutexLocker lock(&m_mutex);
    // We should get a trackCuesUpdated call anyway, so do nothing.
}

void CueControl::trackCuesUpdated() {
    QMutexLocker lock(&m_mutex);
    QSet<int> active_hotcues;

    if (!m_pLoadedTrack)
        return;

    const QList<CuePointer> cuePoints(m_pLoadedTrack->getCuePoints());
    QListIterator<CuePointer> it(cuePoints);
    while (it.hasNext()) {
        CuePointer pCue(it.next());

        if (pCue->getType() != Cue::CUE && pCue->getType() != Cue::LOAD)
            continue;

        int hotcue = pCue->getHotCue();
        if (hotcue != -1) {
            HotcueControl* pControl = m_hotcueControls.value(hotcue, NULL);

            // Cue's hotcue doesn't have a hotcue control.
            if (pControl == NULL) {
                continue;
            }

            CuePointer pOldCue(pControl->getCue());

            // If the old hotcue is different than this one.
            if (pOldCue != pCue) {
                // If the old hotcue exists, detach it
                if (pOldCue) {
                    detachCue(hotcue);
                }
                attachCue(pCue, hotcue);
            } else {
                // If the old hotcue is the same, then we only need to update
                pControl->setPosition(pCue->getPosition());
            }
            // Add the hotcue to the list of active hotcues
            active_hotcues.insert(hotcue);
        }
    }

    // Detach all hotcues that are no longer present
    for (int i = 0; i < m_iNumHotCues; ++i) {
        if (!active_hotcues.contains(i)) {
            detachCue(i);
        }
    }
}

void CueControl::hotcueSet(HotcueControl* pControl, double v) {
    //qDebug() << "CueControl::hotcueSet" << v;

    if (!v)
        return;

    QMutexLocker lock(&m_mutex);
    if (!m_pLoadedTrack)
        return;

    int hotcue = pControl->getHotcueNumber();
    // Note: the cue is just detached from the hotcue control
    // It remains in the database for later use
    // TODO: find a rule, that allows us to delete the cue as well
    // https://bugs.launchpad.net/mixxx/+bug/1653276
    hotcueClear(pControl, v);

    CuePointer pCue(m_pLoadedTrack->createAndAddCue());
    double closestBeat = m_pClosestBeat->get();
    double cuePosition =
            (m_pQuantizeEnabled->toBool() && closestBeat != -1) ?
                    closestBeat : getSampleOfTrack().current;
    pCue->setPosition(cuePosition);
    pCue->setHotCue(hotcue);
    pCue->setLabel("");
    pCue->setType(Cue::CUE);
    // TODO(XXX) deal with spurious signals
    attachCue(pCue, hotcue);

    // If quantize is enabled and we are not playing, jump to the cue point
    // since it's not necessarily where we currently are. TODO(XXX) is this
    // potentially invalid for vinyl control?
    bool playing = m_pPlay->toBool();
    if (!playing && m_pQuantizeEnabled->toBool()) {
        lock.unlock();  // prevent deadlock.
        // Enginebuffer will quantize more exactly than we can.
        seekAbs(cuePosition);
    }
}

void CueControl::hotcueGoto(HotcueControl* pControl, double v) {
    if (!v)
        return;

    QMutexLocker lock(&m_mutex);
    if (!m_pLoadedTrack) {
        return;
    }

    CuePointer pCue(pControl->getCue());

    // Need to unlock before emitting any signals to prevent deadlock.
    lock.unlock();

    if (pCue) {
        int position = pCue->getPosition();
        if (position != -1) {
            seekAbs(position);
        }
    }
}

void CueControl::hotcueGotoAndStop(HotcueControl* pControl, double v) {
    if (!v)
        return;

    QMutexLocker lock(&m_mutex);
    if (!m_pLoadedTrack)
        return;

    CuePointer pCue(pControl->getCue());

    // Need to unlock before emitting any signals to prevent deadlock.
    lock.unlock();

    if (pCue) {
        int position = pCue->getPosition();
        if (position != -1) {
            m_pPlay->set(0.0);
            seekExact(position);
        }
    }
}

void CueControl::hotcueGotoAndPlay(HotcueControl* pControl, double v) {
    if (!v)
        return;

    QMutexLocker lock(&m_mutex);
    if (!m_pLoadedTrack) {
        return;
    }

    CuePointer pCue(pControl->getCue());

    // Need to unlock before emitting any signals to prevent deadlock.
    lock.unlock();

    if (pCue) {
        int position = pCue->getPosition();
        if (position != -1) {
            seekAbs(position);
            if (!isPlayingByPlayButton()) {
                // cueGoto is processed asynchrony.
                // avoid a wrong cue set if seek by cueGoto is still pending
                m_bPreviewing = false;
                m_iCurrentlyPreviewingHotcues = 0;
                // don't move the cue point to the hot cue point in DENON mode
                m_bypassCueSetByPlay = true;
                m_pPlay->set(1.0);
            }
        }
    }
}

void CueControl::hotcueActivate(HotcueControl* pControl, double v) {
    //qDebug() << "CueControl::hotcueActivate" << v;

    QMutexLocker lock(&m_mutex);

    if (!m_pLoadedTrack) {
        return;
    }

    CuePointer pCue(pControl->getCue());

    lock.unlock();

    if (pCue) {
        if (v) {
            if (pCue->getPosition() == -1) {
                hotcueSet(pControl, v);
            } else {
                if (isPlayingByPlayButton()) {
                    hotcueGoto(pControl, v);
                } else {
                    hotcueActivatePreview(pControl, v);
                }
            }
        } else {
            if (pCue->getPosition() != -1) {
                hotcueActivatePreview(pControl, v);
            }
        }
    } else {
        if (v) {
            // just in case
            hotcueSet(pControl, v);
        } else if (m_iCurrentlyPreviewingHotcues) {
            // The cue is non-existent, yet we got a release for it and are
            // currently previewing a hotcue. This is indicative of a corner
            // case where the cue was detached while we were pressing it. Let
            // hotcueActivatePreview handle it.
            hotcueActivatePreview(pControl, v);
        }
    }
}

void CueControl::hotcueActivatePreview(HotcueControl* pControl, double v) {
    QMutexLocker lock(&m_mutex);
    if (!m_pLoadedTrack) {
        return;
    }
    CuePointer pCue(pControl->getCue());

    if (v) {
        if (pCue && pCue->getPosition() != -1) {
            m_iCurrentlyPreviewingHotcues++;
            double position = pCue->getPosition();
            m_bypassCueSetByPlay = true;
            m_pPlay->set(1.0);
            pControl->setPreviewing(true);
            pControl->setPreviewingPosition(position);

            // Need to unlock before emitting any signals to prevent deadlock.
            lock.unlock();

            seekAbs(position);
        }
    } else if (m_iCurrentlyPreviewingHotcues) {
        // This is a activate release and we are previewing at least one
        // hotcue. If this hotcue is previewing:
        if (pControl->isPreviewing()) {
            // Mark this hotcue as not previewing.
            double position = pControl->getPreviewingPosition();
            pControl->setPreviewing(false);
            pControl->setPreviewingPosition(-1);

            // If this is the last hotcue to leave preview.
            if (--m_iCurrentlyPreviewingHotcues == 0 && !m_bPreviewing) {
                m_pPlay->set(0.0);
                // Need to unlock before emitting any signals to prevent deadlock.
                lock.unlock();
                seekExact(position);
            }
        }
    }
}

void CueControl::hotcueClear(HotcueControl* pControl, double v) {
    if (!v)
        return;

    QMutexLocker lock(&m_mutex);
    if (!m_pLoadedTrack) {
        return;
    }

    CuePointer pCue(pControl->getCue());
    detachCue(pControl->getHotcueNumber());
    m_pLoadedTrack->removeCue(pCue);
}

void CueControl::hotcuePositionChanged(HotcueControl* pControl, double newPosition) {
    QMutexLocker lock(&m_mutex);
    if (!m_pLoadedTrack)
        return;

    CuePointer pCue(pControl->getCue());
    if (pCue) {
        // Setting the position to -1 is the same as calling hotcue_x_clear
        if (newPosition == -1) {
            pCue->setHotCue(-1);
            detachCue(pControl->getHotcueNumber());
        } else if (newPosition > 0 && newPosition < m_pTrackSamples->get()) {
            pCue->setPosition(newPosition);
        }
    }
}

void CueControl::hintReader(HintVector* pHintList) {
    Hint cue_hint;
    double cuePoint = m_pCuePoint->get();
    if (cuePoint >= 0) {
        cue_hint.frame = SampleUtil::floorPlayPosToFrame(m_pCuePoint->get());
        cue_hint.frameCount = Hint::kFrameCountForward;
        cue_hint.priority = 10;
        pHintList->append(cue_hint);
    }

    // this is called from the engine thread
    // it is no locking required, because m_hotcueControl is filled during the
    // constructor and getPosition()->get() is a ControlObject
    for (const auto& pControl: m_hotcueControls) {
        double position = pControl->getPosition();
        if (position != -1) {
            cue_hint.frame = SampleUtil::floorPlayPosToFrame(position);
            cue_hint.frameCount = Hint::kFrameCountForward;
            cue_hint.priority = 10;
            pHintList->append(cue_hint);
        }
    }
}

// Moves the cue point to current position or to closest beat in case
// quantize is enabled
void CueControl::cueSet(double v) {
    if (!v)
        return;

    QMutexLocker lock(&m_mutex);
    double closestBeat = m_pClosestBeat->get();
    double cue = (m_pQuantizeEnabled->toBool() && closestBeat != -1) ?
            closestBeat : getSampleOfTrack().current;
    m_pCuePoint->set(cue);
    TrackPointer pLoadedTrack = m_pLoadedTrack;
    lock.unlock();

    // Store cue point in loaded track
    if (pLoadedTrack) {
        pLoadedTrack->setCuePoint(cue);
    }
}

void CueControl::cueGoto(double v)
{
    if (!v)
        return;

    QMutexLocker lock(&m_mutex);
    // Seek to cue point
    double cuePoint = m_pCuePoint->get();

    // Need to unlock before emitting any signals to prevent deadlock.
    lock.unlock();

    seekAbs(cuePoint);
}

void CueControl::cueGotoAndPlay(double v)
{
    if (!v) return;
    cueGoto(v);
    QMutexLocker lock(&m_mutex);
    // Start playing if not already
    if (!isPlayingByPlayButton()) {
        // cueGoto is processed asynchrony.
        // avoid a wrong cue set if seek by cueGoto is still pending
        m_bPreviewing = false;
        m_iCurrentlyPreviewingHotcues = 0;
        m_bypassCueSetByPlay = true;
        m_pPlay->set(1.0);
    }
}

void CueControl::cueGotoAndStop(double v)
{
    if (!v)
        return;

    QMutexLocker lock(&m_mutex);
    m_pPlay->set(0.0);
    double cuePoint = m_pCuePoint->get();

    // Need to unlock before emitting any signals to prevent deadlock.
    lock.unlock();

    seekExact(cuePoint);
}

void CueControl::cuePreview(double v)
{
    QMutexLocker lock(&m_mutex);

    if (v) {
        m_bPreviewing = true;
        m_bypassCueSetByPlay = true;
        m_pPlay->set(1.0);
    } else if (!v && m_bPreviewing) {
        m_bPreviewing = false;
        if (!m_iCurrentlyPreviewingHotcues) {
            m_pPlay->set(0.0);
        } else {
            return;
        }
    } else {
        return;
    }

    // Need to unlock before emitting any signals to prevent deadlock.
    lock.unlock();

    seekAbs(m_pCuePoint->get());
}

void CueControl::cueCDJ(double v) {
    // This is how Pioneer cue buttons work:
    // If pressed while freely playing (i.e. playing and platter NOT being touched), stop playback and go to cue.
    // If pressed while NOT freely playing (i.e. stopped or playing but platter IS being touched), set new cue point.
    // If pressed while stopped and at cue, play while pressed.
    // If play is pressed while holding cue, the deck is now playing. (Handled in playFromCuePreview().)

    QMutexLocker lock(&m_mutex);
    const auto freely_playing = m_pPlay->toBool() && !getEngineBuffer()->getScratching();
    TrackAt trackAt = getTrackAt();

    if (v) {
        if (m_iCurrentlyPreviewingHotcues) {
            // we are already previewing by hotcues
            // just jump to cue point and continue previewing
            m_bPreviewing = true;
            lock.unlock();
            seekAbs(m_pCuePoint->get());
        } else if (freely_playing || trackAt == TrackAt::End) {
            // Jump to cue when playing or when at end position

            // Just in case.
            m_bPreviewing = false;
            m_pPlay->set(0.0);

            // Need to unlock before emitting any signals to prevent deadlock.
            lock.unlock();

            seekAbs(m_pCuePoint->get());
        } else if (trackAt == TrackAt::Cue) {
            // pause at cue point
            m_bPreviewing = true;
            m_pPlay->set(1.0);
        } else {
            // Pause not at cue point and not at end position
            cueSet(v);
            // Just in case.
            m_bPreviewing = false;
            m_pPlay->set(0.0);

            // If quantize is enabled, jump to the cue point since it's not
            // necessarily where we currently are
            if (m_pQuantizeEnabled->toBool()) {
                lock.unlock();  // prevent deadlock.
                // Enginebuffer will quantize more exactly than we can.
                seekAbs(m_pCuePoint->get());
            }
        }
    } else if (m_bPreviewing) {
        m_bPreviewing = false;
        if (!m_iCurrentlyPreviewingHotcues) {
            m_pPlay->set(0.0);

            // Need to unlock before emitting any signals to prevent deadlock.
            lock.unlock();

            seekAbs(m_pCuePoint->get());
        }
    }
    // indicator may flash because the delayed adoption of seekAbs
    // Correct the Indicator set via play
    if (m_pLoadedTrack && !freely_playing) {
        m_pCueIndicator->setBlinkValue(ControlIndicator::ON);
    } else {
        m_pCueIndicator->setBlinkValue(ControlIndicator::OFF);
    }
}

void CueControl::cueDenon(double v) {
    // This is how Denon DN-S 3700 cue buttons work:
    // If pressed go to cue and stop.
    // If pressed while stopped and at cue, play while pressed.
    // Cue Point is moved by play from pause

    QMutexLocker lock(&m_mutex);
    bool playing = (m_pPlay->toBool());
    TrackAt trackAt = getTrackAt();

    if (v) {
        if (m_iCurrentlyPreviewingHotcues) {
            // we are already previewing by hotcues
            // just jump to cue point and continue previewing
            m_bPreviewing = true;
            lock.unlock();
            seekAbs(m_pCuePoint->get());
        } else if (!playing && trackAt == TrackAt::Cue) {
            // pause at cue point
            m_bPreviewing = true;
            m_pPlay->set(1.0);
        } else {
            // Just in case.
            m_bPreviewing = false;
            m_pPlay->set(0.0);

            // Need to unlock before emitting any signals to prevent deadlock.
            lock.unlock();

            seekAbs(m_pCuePoint->get());
        }
    } else if (m_bPreviewing) {
        m_bPreviewing = false;
        if (!m_iCurrentlyPreviewingHotcues) {
            m_pPlay->set(0.0);

            // Need to unlock before emitting any signals to prevent deadlock.
            lock.unlock();

            seekAbs(m_pCuePoint->get());
        }
    }
}

void CueControl::cuePlay(double v) {
    // This is how CUP button works:
    // If freely playing (i.e. playing and platter NOT being touched), press to go to cue and stop.
    // If not freely playing (i.e. stopped or platter IS being touched), press to go to cue and stop.
    // On release, start playing from cue point.


    QMutexLocker lock(&m_mutex);
    const auto freely_playing = m_pPlay->toBool() && !getEngineBuffer()->getScratching();
    TrackAt trackAt = getTrackAt();

    // pressed
    if (v) {
        if (freely_playing) {
            m_bPreviewing = false;
            m_pPlay->set(0.0);

            // Need to unlock before emitting any signals to prevent deadlock.
            lock.unlock();

            seekAbs(m_pCuePoint->get());
        } else if (trackAt == TrackAt::ElseWhere) {
            // Pause not at cue point and not at end position
            cueSet(v);
            // Just in case.
            m_bPreviewing = false;
            m_pPlay->set(0.0);
            // If quantize is enabled, jump to the cue point since it's not
            // necessarily where we currently are
            if (m_pQuantizeEnabled->toBool()) {
                lock.unlock();  // prevent deadlock.
                // Enginebuffer will quantize more exactly than we can.
                seekAbs(m_pCuePoint->get());
            }
        }
    } else if (trackAt == TrackAt::Cue){
        m_bPreviewing = false;
        m_pPlay->set(1.0);
        lock.unlock();
    }
}

void CueControl::cueDefault(double v) {
    double cueMode = m_pCueMode->get();
    // Decide which cue implementation to call based on the user preference
    if (cueMode == CUE_MODE_DENON || cueMode == CUE_MODE_NUMARK) {
        cueDenon(v);
    } else if (cueMode == CUE_MODE_CUP) {
        cuePlay(v);
    } else {
        // The modes CUE_MODE_PIONEER and CUE_MODE_MIXXX are similar
        // are handled inside cueCDJ(v)
        // default to Pioneer mode
        cueCDJ(v);
    }
}

void CueControl::pause(double v) {
    QMutexLocker lock(&m_mutex);
    //qDebug() << "CueControl::pause()" << v;
    if (v != 0.0) {
        m_pPlay->set(0.0);
    }
}

void CueControl::playStutter(double v) {
    QMutexLocker lock(&m_mutex);
    //qDebug() << "playStutter" << v;
    if (v != 0.0) {
        if (isPlayingByPlayButton()) {
            cueGoto(1.0);
        } else {
            m_pPlay->set(1.0);
        }
    }
}

bool CueControl::updateIndicatorsAndModifyPlay(bool newPlay, bool playPossible) {
    //qDebug() << "updateIndicatorsAndModifyPlay" << newPlay << playPossible
    //        << m_iCurrentlyPreviewingHotcues << m_bPreviewing;
    QMutexLocker lock(&m_mutex);
    double cueMode = m_pCueMode->get();
    if ((cueMode == CUE_MODE_DENON || cueMode == CUE_MODE_NUMARK) &&
        newPlay && playPossible &&
        !m_pPlay->toBool() &&
        !m_bypassCueSetByPlay) {
        // in Denon mode each play from pause moves the cue point
        // if not previewing
        cueSet(1.0);
    }
    m_bypassCueSetByPlay = false;

    // when previewing, "play" was set by cue button, a following toggle request
    // (play = 0.0) is used for latching play.
    bool previewing = false;
    if (m_bPreviewing || m_iCurrentlyPreviewingHotcues) {
        if (!newPlay) {
            // play latch request: stop previewing and go into normal play mode.
            m_bPreviewing = false;
            m_iCurrentlyPreviewingHotcues = 0;
            newPlay = true;
        } else {
            previewing = true;
        }
    }

    TrackAt trackAt = getTrackAt();

    if (!playPossible) {
        // play not possible
        newPlay = false;
        m_pPlayIndicator->setBlinkValue(ControlIndicator::OFF);
        m_pStopButton->set(0.0);
    } else if (newPlay && !previewing) {
        // Play: Indicates a latched Play
        m_pPlayIndicator->setBlinkValue(ControlIndicator::ON);
        m_pStopButton->set(0.0);
    } else {
        // Pause:
        m_pStopButton->set(1.0);
        if (cueMode == CUE_MODE_DENON) {
            if (trackAt == TrackAt::Cue || previewing) {
                m_pPlayIndicator->setBlinkValue(ControlIndicator::OFF);
            } else {
                // Flashing indicates that a following play would move cue point
                m_pPlayIndicator->setBlinkValue(ControlIndicator::RATIO1TO1_500MS);
            }
        } else if (cueMode == CUE_MODE_MIXXX || cueMode == CUE_MODE_MIXXX_NO_BLINK ||
                   cueMode == CUE_MODE_NUMARK) {
            m_pPlayIndicator->setBlinkValue(ControlIndicator::OFF);
        } else {
            // Flashing indicates that play is possible in Pioneer mode
            m_pPlayIndicator->setBlinkValue(ControlIndicator::RATIO1TO1_500MS);
        }
    }

    if (cueMode != CUE_MODE_DENON && cueMode != CUE_MODE_NUMARK) {
        if (m_pCuePoint->get() != -1) {
            if (newPlay == 0.0 && trackAt == TrackAt::ElseWhere) {
                if (cueMode == CUE_MODE_MIXXX) {
                    // in Mixxx mode Cue Button is flashing slow if CUE will move Cue point
                    m_pCueIndicator->setBlinkValue(ControlIndicator::RATIO1TO1_500MS);
                } else if (cueMode == CUE_MODE_MIXXX_NO_BLINK) {
                    m_pCueIndicator->setBlinkValue(ControlIndicator::OFF);
                } else {
                    // in Pioneer mode Cue Button is flashing fast if CUE will move Cue point
                    m_pCueIndicator->setBlinkValue(ControlIndicator::RATIO1TO1_250MS);
                }
            } else {
                m_pCueIndicator->setBlinkValue(ControlIndicator::OFF);
            }
        } else {
            m_pCueIndicator->setBlinkValue(ControlIndicator::OFF);
        }
    }
    m_pPlayStutter->set(newPlay ? 1.0 : 0.0);

    return newPlay;
}

// called from the engine thread
void CueControl::updateIndicators() {
    // No need for mutex lock because we are only touching COs.
    double cueMode = m_pCueMode->get();
    TrackAt trackAt = getTrackAt();

    if (cueMode == CUE_MODE_DENON || cueMode == CUE_MODE_NUMARK) {
        // Cue button is only lit at cue point
        bool playing = m_pPlay->toBool();
        if (trackAt == TrackAt::Cue) {
            // at cue point
            if (!playing) {
                m_pCueIndicator->setBlinkValue(ControlIndicator::ON);
                m_pPlayIndicator->setBlinkValue(ControlIndicator::OFF);
            }
        } else {
            m_pCueIndicator->setBlinkValue(ControlIndicator::OFF);
            if (!playing) {
                if (trackAt != TrackAt::End && cueMode != CUE_MODE_NUMARK) {
                    // Play will move cue point
                    m_pPlayIndicator->setBlinkValue(ControlIndicator::RATIO1TO1_500MS);
                } else {
                    // At track end
                    m_pPlayIndicator->setBlinkValue(ControlIndicator::OFF);
                }
            }
        }
    } else {
        // Here we have CUE_MODE_PIONEER or CUE_MODE_MIXXX
        // default to Pioneer mode
        if (!m_bPreviewing) {
            const auto freely_playing = m_pPlay->toBool() && !getEngineBuffer()->getScratching();
            if (!freely_playing) {
                switch (trackAt) {
                case TrackAt::ElseWhere:
                    if (cueMode == CUE_MODE_MIXXX) {
                        // in Mixxx mode Cue Button is flashing slow if CUE will move Cue point
                        m_pCueIndicator->setBlinkValue(ControlIndicator::RATIO1TO1_500MS);
                    } else if (cueMode == CUE_MODE_MIXXX_NO_BLINK) {
                        m_pCueIndicator->setBlinkValue(ControlIndicator::OFF);
                    } else {
                        // in Pioneer mode Cue Button is flashing fast if CUE will move Cue point
                        m_pCueIndicator->setBlinkValue(ControlIndicator::RATIO1TO1_250MS);
                    }
                    break;
                case TrackAt::End:
                    // At track end
                    m_pCueIndicator->setBlinkValue(ControlIndicator::OFF);
                    break;
                case TrackAt::Cue:
                    // Next Press is preview
                    m_pCueIndicator->setBlinkValue(ControlIndicator::ON);
                    break;
                }
            } else {
                // Cue indicator should be off when freely playing
                m_pCueIndicator->setBlinkValue(ControlIndicator::OFF);
            }
        }
    }
}

void CueControl::resetIndicators() {
    m_pCueIndicator->setBlinkValue(ControlIndicator::OFF);
    m_pPlayIndicator->setBlinkValue(ControlIndicator::OFF);
}

CueControl::TrackAt CueControl::getTrackAt() const {
    SampleOfTrack sot = getSampleOfTrack();
    if (sot.current >= sot.total) {
        return TrackAt::End;
    }
    double cue = m_pCuePoint->get();
    if (cue != -1 && fabs(sot.current - cue) < 1.0f) {
        return TrackAt::Cue;
    }
    return TrackAt::ElseWhere;
}

bool CueControl::isPlayingByPlayButton() {
    return m_pPlay->toBool() &&
            !m_iCurrentlyPreviewingHotcues && !m_bPreviewing;
}


ConfigKey HotcueControl::keyForControl(int hotcue, const char* name) {
    ConfigKey key;
    key.group = m_group;
    // Add one to hotcue so that we don't have a hotcue_0
    key.item = QLatin1String("hotcue_") % QString::number(hotcue+1) % "_" % name;
    return key;
}

HotcueControl::HotcueControl(QString group, int i)
        : m_group(group),
          m_iHotcueNumber(i),
          m_pCue(NULL),
          m_bPreviewing(false),
          m_previewingPosition(-1) {
    m_hotcuePosition = new ControlObject(keyForControl(i, "position"));
    connect(m_hotcuePosition, &ControlObject::valueChanged,
            this, &HotcueControl::slotHotcuePositionChanged,
            Qt::DirectConnection);
    m_hotcuePosition->set(-1);

    m_hotcueEnabled = new ControlObject(keyForControl(i, "enabled"));
    m_hotcueEnabled->setReadOnly();

    m_hotcueSet = new ControlPushButton(keyForControl(i, "set"));
    connect(m_hotcueSet, &ControlObject::valueChanged,
            this, &HotcueControl::slotHotcueSet,
            Qt::DirectConnection);

    m_hotcueGoto = new ControlPushButton(keyForControl(i, "goto"));
    connect(m_hotcueGoto, &ControlObject::valueChanged,
            this, &HotcueControl::slotHotcueGoto,
            Qt::DirectConnection);

    m_hotcueGotoAndPlay = new ControlPushButton(keyForControl(i, "gotoandplay"));
    connect(m_hotcueGotoAndPlay, &ControlObject::valueChanged,
            this, &HotcueControl::slotHotcueGotoAndPlay,
            Qt::DirectConnection);

    m_hotcueGotoAndStop = new ControlPushButton(keyForControl(i, "gotoandstop"));
    connect(m_hotcueGotoAndStop, &ControlObject::valueChanged,
            this, &HotcueControl::slotHotcueGotoAndStop,
            Qt::DirectConnection);

    m_hotcueActivate = new ControlPushButton(keyForControl(i, "activate"));
    connect(m_hotcueActivate, &ControlObject::valueChanged,
            this, &HotcueControl::slotHotcueActivate,
            Qt::DirectConnection);

    m_hotcueActivatePreview = new ControlPushButton(keyForControl(i, "activate_preview"));
    connect(m_hotcueActivatePreview, &ControlObject::valueChanged,
            this, &HotcueControl::slotHotcueActivatePreview,
            Qt::DirectConnection);

    m_hotcueClear = new ControlPushButton(keyForControl(i, "clear"));
    connect(m_hotcueClear, &ControlObject::valueChanged,
            this, &HotcueControl::slotHotcueClear,
            Qt::DirectConnection);
}

HotcueControl::~HotcueControl() {
    delete m_hotcuePosition;
    delete m_hotcueEnabled;
    delete m_hotcueSet;
    delete m_hotcueGoto;
    delete m_hotcueGotoAndPlay;
    delete m_hotcueGotoAndStop;
    delete m_hotcueActivate;
    delete m_hotcueActivatePreview;
    delete m_hotcueClear;
}

void HotcueControl::slotHotcueSet(double v) {
    emit(hotcueSet(this, v));
}

void HotcueControl::slotHotcueGoto(double v) {
    emit(hotcueGoto(this, v));
}

void HotcueControl::slotHotcueGotoAndPlay(double v) {
    emit(hotcueGotoAndPlay(this, v));
}

void HotcueControl::slotHotcueGotoAndStop(double v) {
    emit(hotcueGotoAndStop(this, v));
}

void HotcueControl::slotHotcueActivate(double v) {
    emit(hotcueActivate(this, v));
}

void HotcueControl::slotHotcueActivatePreview(double v) {
    emit(hotcueActivatePreview(this, v));
}

void HotcueControl::slotHotcueClear(double v) {
    emit(hotcueClear(this, v));
}

void HotcueControl::slotHotcuePositionChanged(double newPosition) {
    m_hotcueEnabled->forceSet(newPosition == -1 ? 0.0 : 1.0);
    emit(hotcuePositionChanged(this, newPosition));
}

double HotcueControl::getPosition() const {
    return m_hotcuePosition->get();
}

void HotcueControl::setCue(CuePointer pCue) {
    setPosition(pCue->getPosition());
    // set pCue only if all other data is in place
    // because we have a null check for valid data else where in the code
    m_pCue = pCue;
}

void HotcueControl::resetCue() {
    // clear pCue first because we have a null check for valid data else where
    // in the code
    m_pCue.reset();
    setPosition(-1.0);
}

void HotcueControl::setPosition(double position) {
    m_hotcuePosition->set(position);
    m_hotcueEnabled->forceSet(position == -1.0 ? 0.0 : 1.0);
}
