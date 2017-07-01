// cuecontrol.cpp
// Created 11/5/2009 by RJ Ryan (rryan@mit.edu)

#include <QMutexLocker>
#include <QStringBuilder>

#include "engine/enginebuffer.h"
#include "engine/cuecontrol.h"

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

    m_pCueSource = new ControlObject(ConfigKey(group, "cue_source"));

    m_pCueMode = new ControlObject(ConfigKey(group, "cue_mode"));

    m_pSeekOnLoadMode = new ControlObject(ConfigKey(group, "seekonload_mode"));

    m_pCueSet = new ControlPushButton(ConfigKey(group, "cue_set"));
    m_pCueSet->setButtonMode(ControlPushButton::TRIGGER);
    connect(m_pCueSet, SIGNAL(valueChanged(double)),
            this, SLOT(cueSet(double)),
            Qt::DirectConnection);

    m_pCueClear = new ControlPushButton(ConfigKey(group, "cue_clear"));
    m_pCueClear->setButtonMode(ControlPushButton::TRIGGER);
    connect(m_pCueClear, SIGNAL(valueChanged(double)),
            this, SLOT(cueClear(double)),
            Qt::DirectConnection);

    m_pCueGoto = new ControlPushButton(ConfigKey(group, "cue_goto"));
    connect(m_pCueGoto, SIGNAL(valueChanged(double)),
            this, SLOT(cueGoto(double)),
            Qt::DirectConnection);

    m_pCueGotoAndPlay =
            new ControlPushButton(ConfigKey(group, "cue_gotoandplay"));
    connect(m_pCueGotoAndPlay, SIGNAL(valueChanged(double)),
            this, SLOT(cueGotoAndPlay(double)),
            Qt::DirectConnection);

    m_pCuePlay =
            new ControlPushButton(ConfigKey(group, "cue_play"));
    connect(m_pCuePlay, SIGNAL(valueChanged(double)),
            this, SLOT(cuePlay(double)),
            Qt::DirectConnection);

    m_pCueGotoAndStop =
            new ControlPushButton(ConfigKey(group, "cue_gotoandstop"));
    connect(m_pCueGotoAndStop, SIGNAL(valueChanged(double)),
            this, SLOT(cueGotoAndStop(double)),
            Qt::DirectConnection);

    m_pCuePreview = new ControlPushButton(ConfigKey(group, "cue_preview"));
    connect(m_pCuePreview, SIGNAL(valueChanged(double)),
            this, SLOT(cuePreview(double)),
            Qt::DirectConnection);

    m_pCueCDJ = new ControlPushButton(ConfigKey(group, "cue_cdj"));
    connect(m_pCueCDJ, SIGNAL(valueChanged(double)),
            this, SLOT(cueCDJ(double)),
            Qt::DirectConnection);

    m_pCueDefault = new ControlPushButton(ConfigKey(group, "cue_default"));
    connect(m_pCueDefault, SIGNAL(valueChanged(double)),
            this, SLOT(cueDefault(double)),
            Qt::DirectConnection);

    m_pPlayStutter = new ControlPushButton(ConfigKey(group, "play_stutter"));
    connect(m_pPlayStutter, SIGNAL(valueChanged(double)),
            this, SLOT(playStutter(double)),
            Qt::DirectConnection);

    m_pCueIndicator = new ControlIndicator(ConfigKey(group, "cue_indicator"));
    m_pPlayIndicator = new ControlIndicator(ConfigKey(group, "play_indicator"));

    m_pAutoDJStartPosition = new ControlObject(ConfigKey(group, "autodj_start_position"));
    m_pAutoDJStartPosition->set(-1.0);

    m_pAutoDJStartSource = new ControlObject(ConfigKey(group, "autodj_start_source"));

    m_pAutoDJStartSet = new ControlPushButton(ConfigKey(group, "autodj_start_set"));
    connect(m_pAutoDJStartSet, SIGNAL(valueChanged(double)),
            this, SLOT(autoDJStartSet(double)),
            Qt::DirectConnection);

    m_pAutoDJStartClear = new ControlPushButton(ConfigKey(group, "autodj_start_clear"));
    connect(m_pAutoDJStartClear, SIGNAL(valueChanged(double)),
            this, SLOT(autoDJStartClear(double)),
            Qt::DirectConnection);

    m_pAutoDJEndPosition = new ControlObject(ConfigKey(group, "autodj_end_position"));
    m_pAutoDJEndPosition->set(-1.0);

    m_pAutoDJEndSource = new ControlObject(ConfigKey(group, "autodj_end_source"));

    m_pAutoDJEndSet = new ControlPushButton(ConfigKey(group, "autodj_end_set"));
    connect(m_pAutoDJEndSet, SIGNAL(valueChanged(double)),
            this, SLOT(autoDJEndSet(double)),
            Qt::DirectConnection);

    m_pAutoDJEndClear = new ControlPushButton(ConfigKey(group, "autodj_end_clear"));
    connect(m_pAutoDJEndClear, SIGNAL(valueChanged(double)),
            this, SLOT(autoDJEndClear(double)),
            Qt::DirectConnection);

    m_pVinylControlEnabled = new ControlProxy(group, "vinylcontrol_enabled");
    m_pVinylControlMode = new ControlProxy(group, "vinylcontrol_mode");
}

CueControl::~CueControl() {
    delete m_pCuePoint;
    delete m_pCueSource;
    delete m_pCueMode;
    delete m_pSeekOnLoadMode;
    delete m_pCueSet;
    delete m_pCueClear;
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
    delete m_pAutoDJStartPosition;
    delete m_pAutoDJStartSource;
    delete m_pAutoDJStartSet;
    delete m_pAutoDJStartClear;
    delete m_pAutoDJEndPosition;
    delete m_pAutoDJEndSource;
    delete m_pAutoDJEndSet;
    delete m_pAutoDJEndClear;
    delete m_pVinylControlEnabled;
    delete m_pVinylControlMode;
    qDeleteAll(m_hotcueControls);
}

void CueControl::createControls() {
    for (int i = 0; i < m_iNumHotCues; ++i) {
        HotcueControl* pControl = new HotcueControl(getGroup(), i);

        connect(pControl, SIGNAL(hotcuePositionChanged(HotcueControl*, double)),
                this, SLOT(hotcuePositionChanged(HotcueControl*, double)),
                Qt::DirectConnection);
        connect(pControl, SIGNAL(hotcueSet(HotcueControl*, double)),
                this, SLOT(hotcueSet(HotcueControl*, double)),
                Qt::DirectConnection);
        connect(pControl, SIGNAL(hotcueGoto(HotcueControl*, double)),
                this, SLOT(hotcueGoto(HotcueControl*, double)),
                Qt::DirectConnection);
        connect(pControl, SIGNAL(hotcueGotoAndPlay(HotcueControl*, double)),
                this, SLOT(hotcueGotoAndPlay(HotcueControl*, double)),
                Qt::DirectConnection);
        connect(pControl, SIGNAL(hotcueGotoAndStop(HotcueControl*, double)),
                this, SLOT(hotcueGotoAndStop(HotcueControl*, double)),
                Qt::DirectConnection);
        connect(pControl, SIGNAL(hotcueActivate(HotcueControl*, double)),
                this, SLOT(hotcueActivate(HotcueControl*, double)),
                Qt::DirectConnection);
        connect(pControl, SIGNAL(hotcueActivatePreview(HotcueControl*, double)),
                this, SLOT(hotcueActivatePreview(HotcueControl*, double)),
                Qt::DirectConnection);
        connect(pControl, SIGNAL(hotcueClear(HotcueControl*, double)),
                this, SLOT(hotcueClear(HotcueControl*, double)),
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
    connect(pCue.get(), SIGNAL(updated()),
            this, SLOT(cueUpdated()),
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

void CueControl::trackLoaded(TrackPointer pNewTrack, TrackPointer pOldTrack) {
    QMutexLocker lock(&m_mutex);

    DEBUG_ASSERT(m_pLoadedTrack == pOldTrack);
    if (m_pLoadedTrack) {
        disconnect(m_pLoadedTrack.get(), 0, this, 0);
        for (int i = 0; i < m_iNumHotCues; ++i) {
            detachCue(i);
        }

        m_pCueIndicator->setBlinkValue(ControlIndicator::OFF);
        m_pCuePoint->set(-1.0);
        m_pCueSource->set(Cue::UNKNOWN);
        m_pAutoDJStartPosition->set(-1.0);
        m_pAutoDJStartSource->set(Cue::UNKNOWN);
        m_pAutoDJEndPosition->set(-1.0);
        m_pAutoDJEndSource->set(Cue::UNKNOWN);
        m_pLoadedTrack.reset();
    }

    if (!pNewTrack) {
        return;
    }
    m_pLoadedTrack = pNewTrack;

    connect(m_pLoadedTrack.get(), SIGNAL(cuesUpdated()),
            this, SLOT(trackCuesUpdated()),
            Qt::DirectConnection);

    connect(m_pLoadedTrack.get(), SIGNAL(beatsUpdated()),
            this, SLOT(trackBeatsUpdated()),
            Qt::DirectConnection);

    // Need to unlock before emitting any signals to prevent deadlock.
    lock.unlock();

    // Update COs with cues from track.
    trackCuesUpdated();

    // Seek track according to SeekOnLoadMode.
    SeekOnLoadMode seekOnLoadMode = getSeekOnLoadMode();
    switch (seekOnLoadMode) {
    case SEEK_ON_LOAD_ZERO_POS:
        seekExact(0.0);
        break;
    case SEEK_ON_LOAD_MAIN_CUE:
        seekExact(m_pCuePoint->get());
        break;
    case SEEK_ON_LOAD_ADJ_START:
        seekExact(m_pAutoDJStartPosition->get());
        break;
    default:
        // Respect cue recall preference option.
        if (isCueRecallEnabled() && m_pCuePoint->get() != -1.0) {
            // If cue recall is ON and main cue point is set, seek to it.
            seekExact(m_pCuePoint->get());
        } else if (!(m_pVinylControlEnabled->get() &&
                     m_pVinylControlMode->get() == MIXXX_VCMODE_ABSOLUTE)) {
            // Otherwise, seek to zero unless vinylcontrol is on and
            // set to absolute. This allows users to load tracks and
            // have the needle-drop be maintained.
            seekExact(0.0);
        }
        break;
    }
}

void CueControl::cueUpdated() {
    //QMutexLocker lock(&m_mutex);
    // We should get a trackCuesUpdated call anyway, so do nothing.
}

void CueControl::trackCuesUpdated() {
    QMutexLocker lock(&m_mutex);
    QSet<int> active_hotcues;
    CuePointer pLoadCue, pStartCue, pEndCue;

    if (!m_pLoadedTrack)
        return;

    for (const CuePointer& pCue: m_pLoadedTrack->getCuePoints()) {
        if (pCue->getType() == Cue::LOAD) {
            DEBUG_ASSERT(!pLoadCue);  // There should be only one LOAD cue
            pLoadCue = pCue;
        } else if (pCue->getType() == Cue::BEGIN) {
            DEBUG_ASSERT(!pStartCue);  // There should be only one BEGIN cue
            pStartCue = pCue;
        } else if (pCue->getType() == Cue::END) {
            DEBUG_ASSERT(!pEndCue);  // There should be only one END cue
            pEndCue = pCue;
        } else if (pCue->getType() == Cue::CUE && pCue->getHotCue() != -1) {
            int hotcue = pCue->getHotCue();
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

    SeekOnLoadMode seekOnLoadMode = getSeekOnLoadMode();

    if (pLoadCue) {
        setCue(pLoadCue->getPosition(), pLoadCue->getSource());
    } else {
        m_pCuePoint->set(-1.0);
        m_pCueSource->set(Cue::UNKNOWN);
    }

    if (pStartCue) {
        bool wasTrackAtStart = isTrackAtADJStart();

        // Update COs.
        m_pAutoDJStartPosition->set(pStartCue->getPosition());
        m_pAutoDJStartSource->set(pStartCue->getSource());

        // If track was at AutoDJ start, move track along with it.
        if (wasTrackAtStart && seekOnLoadMode == SEEK_ON_LOAD_ADJ_START) {
            seekExact(pStartCue->getPosition());
        }
    } else {
        m_pAutoDJStartPosition->set(-1.0);
        m_pAutoDJStartSource->set(Cue::UNKNOWN);
    }

    if (pEndCue) {
        m_pAutoDJEndPosition->set(pEndCue->getPosition());
        m_pAutoDJEndSource->set(pEndCue->getSource());
    } else {
        m_pAutoDJEndPosition->set(-1.0);
        m_pAutoDJEndSource->set(Cue::UNKNOWN);
    }

    // Detach all hotcues that are no longer present
    for (int i = 0; i < m_iNumHotCues; ++i) {
        if (!active_hotcues.contains(i)) {
            detachCue(i);
        }
    }
}

void CueControl::trackBeatsUpdated() {
    if (!m_pLoadedTrack)
        return;

    setCue(m_pCuePoint->get(), getCueSource());
}

void CueControl::setCue(double position, Cue::CueSource source) {
    bool wasTrackAtCue = isTrackAtCue();

    // Snap automatically-placed cue point to nearest beat
    if (position != -1.0 && source != Cue::MANUAL) {
        BeatsPointer pBeats = m_pLoadedTrack->getBeats();
        if (pBeats) {
            double closestBeat = pBeats->findClosestBeat(position);
            if (closestBeat != -1.0) {
                position = closestBeat;
            }
        }
    }

    // Update COs
    m_pCuePoint->set(position);
    m_pCueSource->set(source);

    // Make track follow the cue point
    if ((wasTrackAtCue || getCurrentSample() == 0.0) && position != -1.0) {
        SeekOnLoadMode seekOnLoadMode = getSeekOnLoadMode();
        if ((seekOnLoadMode == SEEK_ON_LOAD_DEFAULT && isCueRecallEnabled()) ||
                seekOnLoadMode == SEEK_ON_LOAD_MAIN_CUE) {
            seekExact(position);
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
                    closestBeat : getCurrentSample();
    pCue->setPosition(cuePosition);
    pCue->setHotCue(hotcue);
    pCue->setLabel("");
    pCue->setType(Cue::CUE);
    pCue->setSource(Cue::MANUAL);
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
            closestBeat : getCurrentSample();
    m_pCuePoint->set(cue);
    m_pCueSource->set(Cue::MANUAL);
    TrackPointer pLoadedTrack = m_pLoadedTrack;
    lock.unlock();

    // Store cue point in loaded track
    if (pLoadedTrack) {
        pLoadedTrack->setCuePoint(CuePosition(cue, Cue::MANUAL));
    }
}

void CueControl::cueClear(double v) {
    if (!v) {
        return;
    }

    QMutexLocker lock(&m_mutex);
    m_pCuePoint->set(-1.0);
    m_pCueSource->set(Cue::UNKNOWN);
    TrackPointer pLoadedTrack = m_pLoadedTrack;
    lock.unlock();

    if (pLoadedTrack) {
        pLoadedTrack->setCuePoint(CuePosition());
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

    if (v) {
        if (m_iCurrentlyPreviewingHotcues) {
            // we are already previewing by hotcues
            // just jump to cue point and continue previewing
            m_bPreviewing = true;
            lock.unlock();
            seekAbs(m_pCuePoint->get());
        } else if (freely_playing || atEndPosition()) {
            // Jump to cue when playing or when at end position

            // Just in case.
            m_bPreviewing = false;
            m_pPlay->set(0.0);

            // Need to unlock before emitting any signals to prevent deadlock.
            lock.unlock();

            seekAbs(m_pCuePoint->get());
        } else if (isTrackAtCue()) {
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

    if (v) {
        if (m_iCurrentlyPreviewingHotcues) {
            // we are already previewing by hotcues
            // just jump to cue point and continue previewing
            m_bPreviewing = true;
            lock.unlock();
            seekAbs(m_pCuePoint->get());
        } else if (!playing && isTrackAtCue()) {
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

    // pressed
    if (v) {
        if (freely_playing) {
            m_bPreviewing = false;
            m_pPlay->set(0.0);

            // Need to unlock before emitting any signals to prevent deadlock.
            lock.unlock();

            seekAbs(m_pCuePoint->get());
        } else if (!isTrackAtCue() && getCurrentSample() <= getTotalSamples()) {
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
    } else if (isTrackAtCue()){
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

void CueControl::autoDJStartSet(double v) {
    if (!v) {
        return;
    }

    QMutexLocker lock(&m_mutex);
    double position = getCurrentSample();

    // Make sure user is not trying to place start cue on or after end cue.
    if (position >= m_pAutoDJEndPosition->get()) {
        qWarning() << "Trying to place start cue on or after end cue.";
        return;
    }

    m_pAutoDJStartPosition->set(position);
    m_pAutoDJStartSource->set(Cue::MANUAL);

    TrackPointer pLoadedTrack = m_pLoadedTrack;
    lock.unlock();

    if (pLoadedTrack) {
        CuePointer pCue = pLoadedTrack->findCueByType(Cue::BEGIN);
        if (!pCue) {
            pCue = pLoadedTrack->createAndAddCue();
            pCue->setType(Cue::BEGIN);
        }
        pCue->setSource(Cue::MANUAL);
        pCue->setPosition(position);
    }
}

void CueControl::autoDJStartClear(double v) {
    if (!v) {
        return;
    }

    QMutexLocker lock(&m_mutex);
    m_pAutoDJStartPosition->set(-1.0);
    m_pAutoDJStartSource->set(Cue::UNKNOWN);
    TrackPointer pLoadedTrack = m_pLoadedTrack;
    lock.unlock();

    if (pLoadedTrack) {
        CuePointer pCue = pLoadedTrack->findCueByType(Cue::BEGIN);
        if (pCue) {
            pLoadedTrack->removeCue(pCue);
        }
    }
}

void CueControl::autoDJEndSet(double v) {
    if (!v) {
        return;
    }

    QMutexLocker lock(&m_mutex);
    double position = getCurrentSample();

    // Make sure user is not trying to place end cue on or before start cue.
    if (position <= m_pAutoDJStartPosition->get()) {
        qWarning() << "Trying to place end cue on or before start cue.";
        return;
    }

    m_pAutoDJEndPosition->set(position);
    m_pAutoDJEndSource->set(Cue::MANUAL);

    TrackPointer pLoadedTrack = m_pLoadedTrack;
    lock.unlock();

    if (pLoadedTrack) {
        CuePointer pCue = pLoadedTrack->findCueByType(Cue::END);
        if (!pCue) {
            pCue = pLoadedTrack->createAndAddCue();
            pCue->setType(Cue::END);
        }
        pCue->setSource(Cue::MANUAL);
        pCue->setPosition(position);
    }
}

void CueControl::autoDJEndClear(double v) {
    if (!v) {
        return;
    }

    QMutexLocker lock(&m_mutex);
    m_pAutoDJEndPosition->set(-1.0);
    m_pAutoDJEndSource->set(Cue::UNKNOWN);
    TrackPointer pLoadedTrack = m_pLoadedTrack;
    lock.unlock();

    if (pLoadedTrack) {
        CuePointer pCue = pLoadedTrack->findCueByType(Cue::END);
        if (pCue) {
            pLoadedTrack->removeCue(pCue);
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
            if (isTrackAtCue() || previewing) {
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
            if (newPlay == 0.0 && !isTrackAtCue() &&
                    !atEndPosition()) {
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

void CueControl::updateIndicators() {
    // No need for mutex lock because we are only touching COs.
    double cueMode = m_pCueMode->get();

    if (cueMode == CUE_MODE_DENON || cueMode == CUE_MODE_NUMARK) {
        // Cue button is only lit at cue point
        bool playing = m_pPlay->toBool();
        if (isTrackAtCue()) {
            // at cue point
            if (!playing) {
                m_pCueIndicator->setBlinkValue(ControlIndicator::ON);
                m_pPlayIndicator->setBlinkValue(ControlIndicator::OFF);
            }
        } else {
            m_pCueIndicator->setBlinkValue(ControlIndicator::OFF);
            if (!playing) {
                if (!atEndPosition() && cueMode != CUE_MODE_NUMARK) {
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
                if (!isTrackAtCue()) {
                    if (!atEndPosition()) {
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
                        // At track end
                        m_pCueIndicator->setBlinkValue(ControlIndicator::OFF);
                    }
                } else if (m_pCuePoint->get() != -1) {
                    // Next Press is preview
                    m_pCueIndicator->setBlinkValue(ControlIndicator::ON);
                }
            } else {
                // Cue indicator should be off when freely playing
                m_pCueIndicator->setBlinkValue(ControlIndicator::OFF);
            }
        }
    }
}

bool CueControl::isTrackAtCue() {
    return (fabs(getCurrentSample() - m_pCuePoint->get()) < 1.0f);
}

bool CueControl::isTrackAtADJStart() {
    return (fabs(getCurrentSample() - m_pAutoDJStartPosition->get()) < 1.0f);
}

bool CueControl::isPlayingByPlayButton() {
    return m_pPlay->toBool() &&
            !m_iCurrentlyPreviewingHotcues && !m_bPreviewing;
}

bool CueControl::isCueRecallEnabled() {
    // Note that [Controls],CueRecall == 0 corresponds to "ON", not "OFF".
    return getConfig()->getValue(ConfigKey("[Controls]", "CueRecall"), 0) == 0;
}

SeekOnLoadMode CueControl::getSeekOnLoadMode() {
    return seekOnLoadModeFromDouble(m_pSeekOnLoadMode->get());
}

Cue::CueSource CueControl::getCueSource() {
    // msvs does not allow to cast from double to an enum
    Cue::CueSource source = static_cast<Cue::CueSource>(int(m_pCueSource->get()));
    if (source < 0 || source > 2) {
        return Cue::UNKNOWN;
    }
    return source;
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
    connect(m_hotcuePosition, SIGNAL(valueChanged(double)),
            this, SLOT(slotHotcuePositionChanged(double)),
            Qt::DirectConnection);
    m_hotcuePosition->set(-1);

    m_hotcueEnabled = new ControlObject(keyForControl(i, "enabled"));
    m_hotcueEnabled->setReadOnly();

    m_hotcueSet = new ControlPushButton(keyForControl(i, "set"));
    connect(m_hotcueSet, SIGNAL(valueChanged(double)),
            this, SLOT(slotHotcueSet(double)),
            Qt::DirectConnection);

    m_hotcueGoto = new ControlPushButton(keyForControl(i, "goto"));
    connect(m_hotcueGoto, SIGNAL(valueChanged(double)),
            this, SLOT(slotHotcueGoto(double)),
            Qt::DirectConnection);

    m_hotcueGotoAndPlay = new ControlPushButton(keyForControl(i, "gotoandplay"));
    connect(m_hotcueGotoAndPlay, SIGNAL(valueChanged(double)),
            this, SLOT(slotHotcueGotoAndPlay(double)),
            Qt::DirectConnection);

    m_hotcueGotoAndStop = new ControlPushButton(keyForControl(i, "gotoandstop"));
    connect(m_hotcueGotoAndStop, SIGNAL(valueChanged(double)),
            this, SLOT(slotHotcueGotoAndStop(double)),
            Qt::DirectConnection);

    m_hotcueActivate = new ControlPushButton(keyForControl(i, "activate"));
    connect(m_hotcueActivate, SIGNAL(valueChanged(double)),
            this, SLOT(slotHotcueActivate(double)),
            Qt::DirectConnection);

    m_hotcueActivatePreview = new ControlPushButton(keyForControl(i, "activate_preview"));
    connect(m_hotcueActivatePreview, SIGNAL(valueChanged(double)),
            this, SLOT(slotHotcueActivatePreview(double)),
            Qt::DirectConnection);

    m_hotcueClear = new ControlPushButton(keyForControl(i, "clear"));
    connect(m_hotcueClear, SIGNAL(valueChanged(double)),
            this, SLOT(slotHotcueClear(double)),
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
