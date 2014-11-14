// cuecontrol.cpp
// Created 11/5/2009 by RJ Ryan (rryan@mit.edu)

#include <QMutexLocker>

#include "engine/cuecontrol.h"

#include "controlobject.h"
#include "controlpushbutton.h"
#include "controlindicator.h"
#include "trackinfoobject.h"
#include "library/dao/cue.h"
#include "cachingreader.h"

static const double CUE_MODE_MIXXX = 0.0;
static const double CUE_MODE_PIONEER = 1.0;
static const double CUE_MODE_DENON = 2.0;
static const double CUE_MODE_NUMARK = 3.0;

CueControl::CueControl(QString group,
                       ConfigObject<ConfigValue>* _config) :
        EngineControl(group, _config),
        m_bHotcueCancel(false),
        m_bPreviewing(false),
        m_bPreviewingHotcue(false),
        m_pPlayButton(ControlObject::getControl(ConfigKey(group, "play"))),
        m_pStopButton(ControlObject::getControl(ConfigKey(group, "stop"))),
        m_iCurrentlyPreviewingHotcues(0),
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
    // 0.0 -> Pioneer mode
    // 1.0 -> Denon mode

    m_pCueSet = new ControlPushButton(ConfigKey(group, "cue_set"));
    m_pCueSet->setButtonMode(ControlPushButton::TRIGGER);
    connect(m_pCueSet, SIGNAL(valueChanged(double)),
            this, SLOT(cueSet(double)),
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
}

CueControl::~CueControl() {
    delete m_pCuePoint;
    delete m_pCueMode;
    delete m_pCueSet;
    delete m_pCueGoto;
    delete m_pCueGotoAndPlay;
    delete m_pCueGotoAndStop;
    delete m_pCuePreview;
    delete m_pCueCDJ;
    delete m_pCueDefault;
    delete m_pPlayStutter;
    delete m_pCueIndicator;
    delete m_pPlayIndicator;
    while (m_hotcueControl.size() > 0) {
        HotcueControl* pControl = m_hotcueControl.takeLast();
        delete pControl;
    }
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

        m_hotcueControl.append(pControl);
    }
}

void CueControl::attachCue(Cue* pCue, int hotCue) {
    if (hotCue < 0 || hotCue >= m_iNumHotCues) {
        return;
    }
    HotcueControl* pControl = m_hotcueControl[hotCue];
    if (pControl->getCue() != NULL) {
        detachCue(pControl->getHotcueNumber());
    }
    pControl->setCue(pCue);
    connect(pCue, SIGNAL(updated()),
            this, SLOT(cueUpdated()),
            Qt::DirectConnection);

    pControl->getPosition()->set(pCue->getPosition());
    pControl->getEnabled()->set(pCue->getPosition() == -1 ? 0.0 : 1.0);
}

void CueControl::detachCue(int hotCue) {
    if (hotCue < 0 || hotCue >= m_iNumHotCues) {
        return;
    }
    HotcueControl* pControl = m_hotcueControl[hotCue];
    Cue* pCue = pControl->getCue();
    if (!pCue)
        return;
    disconnect(pCue, 0, this, 0);
    pControl->setCue(NULL);
    pControl->getPosition()->set(-1);
    pControl->getEnabled()->set(0);
}

void CueControl::trackLoaded(TrackPointer pTrack) {
    QMutexLocker lock(&m_mutex);
    if (m_pLoadedTrack)
        trackUnloaded(m_pLoadedTrack);

    if (!pTrack) {
        return;
    }

    m_pLoadedTrack = pTrack;
    connect(pTrack.data(), SIGNAL(cuesUpdated()),
            this, SLOT(trackCuesUpdated()),
            Qt::DirectConnection);

    Cue* loadCue = NULL;
    const QList<Cue*>& cuePoints = pTrack->getCuePoints();
    QListIterator<Cue*> it(cuePoints);
    while (it.hasNext()) {
        Cue* pCue = it.next();
        if (pCue->getType() == Cue::LOAD) {
            loadCue = pCue;
        } else if (pCue->getType() != Cue::CUE) {
            continue;
        }
        int hotcue = pCue->getHotCue();
        if (hotcue != -1)
            attachCue(pCue, hotcue);
    }

    double loadCuePoint = 0.0;
    if (loadCue != NULL) {
        m_pCuePoint->set(loadCue->getPosition());

        // If cue recall is ON in the prefs, then we're supposed to seek to the cue
        // point on song load. Note that [Controls],cueRecall == 0 corresponds to "ON", not OFF.
        if (!getConfig()->getValueString(
                ConfigKey("[Controls]","CueRecall")).toInt()) {
            loadCuePoint = loadCue->getPosition();
        }
    } else {
        // If no cue point is stored, set one at track start
        m_pCuePoint->set(0.0);
    }

    // Need to unlock before emitting any signals to prevent deadlock.
    lock.unlock();
    seekExact(loadCuePoint);
}

void CueControl::trackUnloaded(TrackPointer pTrack) {
    QMutexLocker lock(&m_mutex);
    disconnect(pTrack.data(), 0, this, 0);
    for (int i = 0; i < m_iNumHotCues; ++i) {
        detachCue(i);
    }

    // Store the cue point in a load cue.
    double cuePoint = m_pCuePoint->get();

    if (cuePoint != -1 && cuePoint != 0.0) {
        Cue* loadCue = NULL;
        const QList<Cue*>& cuePoints = pTrack->getCuePoints();
        QListIterator<Cue*> it(cuePoints);
        while (it.hasNext()) {
            Cue* pCue = it.next();
            if (pCue->getType() == Cue::LOAD) {
                loadCue = pCue;
                break;
            }
        }
        if (!loadCue) {
            loadCue = pTrack->addCue();
            loadCue->setType(Cue::LOAD);
            loadCue->setLength(0);
        }
        loadCue->setPosition(cuePoint);
    }

    m_pCueIndicator->setBlinkValue(ControlIndicator::OFF);
    m_pCuePoint->set(-1.0);
    m_pLoadedTrack.clear();
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

    const QList<Cue*>& cuePoints = m_pLoadedTrack->getCuePoints();
    QListIterator<Cue*> it(cuePoints);
    while (it.hasNext()) {
        Cue* pCue = it.next();

        if (pCue->getType() != Cue::CUE && pCue->getType() != Cue::LOAD)
            continue;

        int hotcue = pCue->getHotCue();
        if (hotcue != -1) {
            HotcueControl* pControl = m_hotcueControl[hotcue];
            Cue* pOldCue = pControl->getCue();

            // If the old hotcue is different than this one.
            if (pOldCue != pCue) {
                // If the old hotcue exists, detach it
                if (pOldCue != NULL)
                    detachCue(hotcue);
                attachCue(pCue, hotcue);
            } else {
                // If the old hotcue is the same, then we only need to update
                double dOldPosition = pControl->getPosition()->get();
                double dOldEnabled = pControl->getEnabled()->get();
                double dPosition = pCue->getPosition();
                double dEnabled = dPosition == -1 ? 0.0 : 1.0;
                if (dEnabled != dOldEnabled) {
                    pControl->getEnabled()->set(dEnabled);
                }
                if (dPosition != dOldPosition) {
                    pControl->getPosition()->set(dPosition);
                }
            }
            // Add the hotcue to the list of active hotcues
            active_hotcues.insert(hotcue);
        }
    }

    // Detach all hotcues that are no longer present
    for (int i = 0; i < m_iNumHotCues; ++i) {
        if (!active_hotcues.contains(i))
            detachCue(i);
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
    detachCue(hotcue);
    Cue* pCue = m_pLoadedTrack->addCue();
    double cuePosition =
            (m_pQuantizeEnabled->get() > 0.0 && m_pClosestBeat->get() != -1) ?
            floor(m_pClosestBeat->get()) : floor(getCurrentSample());
    if (!even(static_cast<int>(cuePosition)))
        cuePosition--;
    pCue->setPosition(cuePosition);
    pCue->setHotCue(hotcue);
    pCue->setLabel("");
    pCue->setType(Cue::CUE);
    // TODO(XXX) deal with spurious signals
    attachCue(pCue, hotcue);

    // If quantize is enabled and we are not playing, jump to the cue point
    // since it's not necessarily where we currently are. TODO(XXX) is this
    // potentially invalid for vinyl control?
    bool playing = m_pPlayButton->get() > 0;
    if (!playing && m_pQuantizeEnabled->get() > 0.0) {
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

    Cue* pCue = pControl->getCue();

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

    Cue* pCue = pControl->getCue();

    // Need to unlock before emitting any signals to prevent deadlock.
    lock.unlock();

    if (pCue) {
        int position = pCue->getPosition();
        if (position != -1) {
            m_pPlayButton->set(0.0);
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

    Cue* pCue = pControl->getCue();

    // Need to unlock before emitting any signals to prevent deadlock.
    lock.unlock();

    if (pCue) {
        int position = pCue->getPosition();
        if (position != -1) {
            seekAbs(position);
            m_pPlayButton->set(1.0);
        }
    }
}

void CueControl::hotcueActivate(HotcueControl* pControl, double v) {
    //qDebug() << "CueControl::hotcueActivate" << v;

    QMutexLocker lock(&m_mutex);

    if (!m_pLoadedTrack) {
        return;
    }

    Cue* pCue = pControl->getCue();

    lock.unlock();

    if (pCue) {
        if (v) {
            m_bHotcueCancel = false;
            if (pCue->getPosition() == -1) {
                hotcueSet(pControl, v);
            } else {
                if (!m_bPreviewingHotcue && m_pPlayButton->get() == 1.0) {
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
            m_bHotcueCancel = false;
            hotcueSet(pControl, v);
        } else if (m_bPreviewingHotcue) {
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
    Cue* pCue = pControl->getCue();

    if (v) {
        if (pCue && pCue->getPosition() != -1) {
            m_iCurrentlyPreviewingHotcues++;
            int iPosition = pCue->getPosition();
            m_bPreviewingHotcue = true;
            m_pPlayButton->set(1.0);
            pControl->setPreviewing(true);
            pControl->setPreviewingPosition(iPosition);

            // Need to unlock before emitting any signals to prevent deadlock.
            lock.unlock();

            seekAbs(iPosition);
        }
    } else if (m_bPreviewingHotcue) {
        // This is a activate release and we are previewing at least one
        // hotcue. If this hotcue is previewing:
        if (pControl->isPreviewing()) {
            // Mark this hotcue as not previewing.
            int iPosition = pControl->getPreviewingPosition();
            pControl->setPreviewing(false);
            pControl->setPreviewingPosition(-1);

            // If this is the last hotcue to leave preview.
            if (--m_iCurrentlyPreviewingHotcues == 0) {
                bool bHotcueCancel = m_bHotcueCancel;
                m_bPreviewingHotcue = false;
                m_bHotcueCancel = false;
                // If hotcue cancel was not marked then snap back to the
                // hotcue and stop.
                if (!bHotcueCancel) {
                    m_pPlayButton->set(0.0);
                    // Need to unlock before emitting any signals to prevent deadlock.
                    lock.unlock();
                    seekExact(iPosition);
                }
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

    Cue* pCue = pControl->getCue();
    if (pCue) {
        pCue->setHotCue(-1);
    }

    detachCue(pControl->getHotcueNumber());
}

void CueControl::hotcuePositionChanged(HotcueControl* pControl, double newPosition) {
    QMutexLocker lock(&m_mutex);
    if (!m_pLoadedTrack)
        return;

    Cue* pCue = pControl->getCue();
    if (pCue) {
        // Setting the position to -1 is the same as calling hotcue_x_clear
        if (newPosition == -1) {
            pCue->setHotCue(-1);
            detachCue(pControl->getHotcueNumber());
        } else if (newPosition > 0 && newPosition < m_pTrackSamples->get()) {
            int position = newPosition;
            // People writing from MIDI land, elsewhere might be careless.
            if (position % 2 != 0) {
                position--;
            }
            pCue->setPosition(position);
        }
    }
}

void CueControl::hintReader(QVector<Hint>* pHintList) {
    QMutexLocker lock(&m_mutex);

    Hint cue_hint;
    double cuePoint = m_pCuePoint->get();
    if (cuePoint >= 0) {
        cue_hint.sample = m_pCuePoint->get();
        cue_hint.length = 0;
        cue_hint.priority = 10;
        pHintList->append(cue_hint);
    }

    for (int i = 0; i < m_iNumHotCues; ++i) {
        HotcueControl* pControl = m_hotcueControl[i];
        Cue *pCue = pControl->getCue();
        if (pCue != NULL) {
            double position = pControl->getPosition()->get();
            if (position != -1) {
                cue_hint.sample = position;
                if (cue_hint.sample % 2 != 0)
                    cue_hint.sample--;
                cue_hint.length = 0;
                cue_hint.priority = 10;
                pHintList->push_back(cue_hint);
            }
        }
    }
}

void CueControl::saveCuePoint(double cuePoint) {
    if (m_pLoadedTrack) {
        m_pLoadedTrack->setCuePoint(cuePoint);
    }
}

void CueControl::cueSet(double v) {
    if (!v)
        return;

    QMutexLocker lock(&m_mutex);
    double cue = (m_pQuantizeEnabled->get() > 0.0 && m_pClosestBeat->get() != -1) ?
            floor(m_pClosestBeat->get()) : floor(getCurrentSample());
    if (!even(static_cast<int>(cue)))
        cue--;
    m_pCuePoint->set(cue);
    saveCuePoint(cue);
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
    if (!v)
        return;
    cueGoto(v);
    QMutexLocker lock(&m_mutex);
    // Start playing if not already
    if (m_pPlayButton->get()==0.) {
        m_pPlayButton->set(1.0);
    }
}

void CueControl::cueGotoAndStop(double v)
{
    if (!v)
        return;

    QMutexLocker lock(&m_mutex);
    m_pPlayButton->set(0.0);
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
        m_pPlayButton->set(1.0);
    } else if (!v && m_bPreviewing) {
        m_bPreviewing = false;
        m_pPlayButton->set(0.0);
    }

    double cuePoint = m_pCuePoint->get();

    // Need to unlock before emitting any signals to prevent deadlock.
    lock.unlock();

    seekAbs(cuePoint);
}

void CueControl::cueCDJ(double v) {
    // This is how Pioneer cue buttons work:
    // If pressed while playing, stop playback and go to cue.
    // If pressed while stopped and at cue, play while pressed.
    // If pressed while stopped and not at cue, set new cue point.
    // If play is pressed while holding cue, the deck is now playing. (Handled in playFromCuePreview().)

    QMutexLocker lock(&m_mutex);
    bool playing = (m_pPlayButton->get() == 1.0);

    if (v) {
        if (playing || getCurrentSample() >= getTotalSamples()) {
            // Jump to cue when playing or when at end position

            // Just in case.
            m_bPreviewing = false;
            m_pPlayButton->set(0.0);

            // Need to unlock before emitting any signals to prevent deadlock.
            lock.unlock();

            seekAbs(m_pCuePoint->get());
        } else if (isTrackAtCue()) {
            // pause at cue point
            m_bPreviewing = true;
            m_pPlayButton->set(1.0);
        } else {
            // Pause not at cue point and not at end position
            cueSet(v);
            // Just in case.
            m_bPreviewing = false;

            // If quantize is enabled, jump to the cue point since it's not
            // necessarily where we currently are
            if (m_pQuantizeEnabled->get() > 0.0) {
                lock.unlock();  // prevent deadlock.
                // Enginebuffer will quantize more exactly than we can.
                seekAbs(m_pCuePoint->get());
            }
        }
    } else if (m_bPreviewing) {
        m_bPreviewing = false;
        m_pPlayButton->set(0.0);

        // Need to unlock before emitting any signals to prevent deadlock.
        lock.unlock();

        seekAbs(m_pCuePoint->get());
    }
    // indicator may flash because the delayed adoption of seekAbs
    // Correct the Indicator set via play
    if (m_pLoadedTrack && !playing) {
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
    bool playing = (m_pPlayButton->get() == 1.0);

    if (v) {
        if (!playing && isTrackAtCue()) {
            // pause at cue point
            m_bPreviewing = true;
            m_pPlayButton->set(1.0);
        } else {
            // Just in case.
            m_bPreviewing = false;
            m_pPlayButton->set(0.0);

            // Need to unlock before emitting any signals to prevent deadlock.
            lock.unlock();

            seekAbs(m_pCuePoint->get());
        }
    } else if (m_bPreviewing) {
        m_bPreviewing = false;
        m_pPlayButton->set(0.0);

        // Need to unlock before emitting any signals to prevent deadlock.
        lock.unlock();

        seekAbs(m_pCuePoint->get());
    }
}

void CueControl::cueDefault(double v) {
    double cueMode = m_pCueMode->get();
    // Decide which cue implementation to call based on the user preference
    if (cueMode == CUE_MODE_DENON || cueMode == CUE_MODE_NUMARK) {
        cueDenon(v);
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
        m_pPlayButton->set(0.0);
    }
}

void CueControl::playStutter(double v) {
    QMutexLocker lock(&m_mutex);
    //qDebug() << "playStutter" << v;
    if (v != 0.0) {
        if (m_pPlayButton->get() != 0.0) {
            cueGoto(1.0);
        } else {
            m_pPlayButton->set(1.0);
        }
    }
}

double CueControl::updateIndicatorsAndModifyPlay(double play, bool playPossible) {
    QMutexLocker lock(&m_mutex);
    double cueMode = m_pCueMode->get();

    if ((cueMode == CUE_MODE_DENON || cueMode == CUE_MODE_NUMARK) &&
            play > 0.0 &&
            playPossible &&
            m_pPlayButton->get() == 0.0) {
        // in Denon mode each play from pause moves the cue point
        // if not previewing
        cueSet(1.0);
    }

    // when previewing, "play" was set by cue button, a following toggle request
    // (play = 0.0) is used for latching play.
    bool previewing = false;
    if (m_bPreviewing || m_bPreviewingHotcue) {
        if (play == 0.0) {
            // play latch request: stop previewing and go into normal play mode.
            m_bPreviewing = false;
            m_bHotcueCancel = true;
            play = 1.0;
        } else {
            previewing = true;
        }
    }

    if (!playPossible) {
        // play not possible
        play = 0.0;
        m_pPlayIndicator->setBlinkValue(ControlIndicator::OFF);
        m_pStopButton->set(0.0);
    } else if (play && !previewing) {
        // Play: Indicates a latched Play
        m_pPlayIndicator->setBlinkValue(ControlIndicator::ON);
        m_pStopButton->set(0.0);
    } else {
        // Pause:
        m_pStopButton->set(1.0);
        if (cueMode == CUE_MODE_DENON) {
            if (isTrackAtCue()) {
                m_pPlayIndicator->setBlinkValue(ControlIndicator::OFF);
            } else {
                // Flashing indicates that a following play would move cue point
                m_pPlayIndicator->setBlinkValue(ControlIndicator::RATIO1TO1_500MS);
            }
        } else if (cueMode == CUE_MODE_MIXXX || cueMode == CUE_MODE_NUMARK) {
            m_pPlayIndicator->setBlinkValue(ControlIndicator::OFF);
        } else {
            // Flashing indicates that play is possible in Pioneer mode
            m_pPlayIndicator->setBlinkValue(ControlIndicator::RATIO1TO1_500MS);
        }
    }

    if (cueMode != CUE_MODE_DENON && cueMode != CUE_MODE_NUMARK) {
        if (m_pCuePoint->get() != -1) {
            if (play == 0.0 && !isTrackAtCue() &&
                    getCurrentSample() < getTotalSamples()) {
                if (cueMode == CUE_MODE_MIXXX) {
                    // in Mixxx mode Cue Button is flashing slow if CUE will move Cue point
                    m_pCueIndicator->setBlinkValue(ControlIndicator::RATIO1TO1_500MS);
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
    m_pPlayStutter->set(play);

    return play;
}

void CueControl::updateIndicators() {
    // No need for mutex lock because we are only touching COs.
    double cueMode = m_pCueMode->get();

    if (cueMode == CUE_MODE_DENON || cueMode == CUE_MODE_NUMARK) {
        // Cue button is only lit at cue point
        bool playing = m_pPlayButton->get() > 0;
        if (isTrackAtCue()) {
            // at cue point
            if (!playing) {
                m_pCueIndicator->setBlinkValue(ControlIndicator::ON);
                m_pPlayIndicator->setBlinkValue(ControlIndicator::OFF);
            }
        } else {
            m_pCueIndicator->setBlinkValue(ControlIndicator::OFF);
            if (!playing) {
                if (getCurrentSample() < getTotalSamples() && cueMode != CUE_MODE_NUMARK) {
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
            bool playing = m_pPlayButton->get() > 0;
            if (!playing) {
                if (!isTrackAtCue()) {
                    if (getCurrentSample() < getTotalSamples()) {
                        if (cueMode == CUE_MODE_MIXXX) {
                            // in Mixxx mode Cue Button is flashing slow if CUE will move Cue point
                            m_pCueIndicator->setBlinkValue(ControlIndicator::RATIO1TO1_500MS);
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
            }
        }
    }
}

bool CueControl::isTrackAtCue() {
    return (fabs(getCurrentSample() - m_pCuePoint->get()) < 1.0f);
}

ConfigKey HotcueControl::keyForControl(int hotcue, QString name) {
    ConfigKey key;
    key.group = m_group;
    // Add one to hotcue so that we dont have a hotcue_0
    key.item = QString("hotcue_%1_%2").arg(QString::number(hotcue+1), name);
    return key;
}

HotcueControl::HotcueControl(QString group, int i)
        : m_group(group),
          m_iHotcueNumber(i),
          m_pCue(NULL),
          m_bPreviewing(false),
          m_iPreviewingPosition(-1) {
    m_hotcuePosition = new ControlObject(keyForControl(i, "position"));
    connect(m_hotcuePosition, SIGNAL(valueChanged(double)),
            this, SLOT(slotHotcuePositionChanged(double)),
            Qt::DirectConnection);
    m_hotcuePosition->set(-1);

    m_hotcueEnabled = new ControlObject(keyForControl(i, "enabled"));
    m_hotcueEnabled->set(0);

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
    emit(hotcuePositionChanged(this, newPosition));
}
