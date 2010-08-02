// cuecontrol.cpp
// Created 11/5/2009 by RJ Ryan (rryan@mit.edu)

#include <QMutexLocker>

#include "engine/cuecontrol.h"

#include "controlobject.h"
#include "controlpushbutton.h"
#include "trackinfoobject.h"
#include "library/dao/cue.h"
#include "cachingreader.h"
#include "mathstuff.h"

#define NUM_HOT_CUES 32

CueControl::CueControl(const char * _group,
                       ConfigObject<ConfigValue> * _config) :
        EngineControl(_group, _config),
        m_bPreviewing(false),
        m_bPreviewingHotcue(false),
        m_pPlayButton(ControlObject::getControl(ConfigKey(_group, "play"))),
        m_iNumHotCues(NUM_HOT_CUES),
        m_pLoadedTrack(),
        m_mutex(QMutex::Recursive) {
    createControls();

    m_pCuePoint = new ControlObject(ConfigKey(_group, "cue_point"));
    m_pCueMode = new ControlObject(ConfigKey(_group,"cue_mode"));
    m_pCuePoint->set(-1);

    m_pCueSet = new ControlPushButton(ConfigKey(_group, "cue_set"));
    connect(m_pCueSet, SIGNAL(valueChanged(double)),
            this, SLOT(cueSet(double)));

    m_pCueGoto = new ControlPushButton(ConfigKey(_group, "cue_goto"));
    connect(m_pCueGoto, SIGNAL(valueChanged(double)),
            this, SLOT(cueGoto(double)));

    m_pCueGotoAndStop =
            new ControlPushButton(ConfigKey(_group, "cue_gotoandstop"));
    connect(m_pCueGotoAndStop, SIGNAL(valueChanged(double)),
            this, SLOT(cueGotoAndStop(double)));

    m_pCueSimple = new ControlPushButton(ConfigKey(_group, "cue_simple"));
    connect(m_pCueSimple, SIGNAL(valueChanged(double)),
            this, SLOT(cueSimple(double)));

    m_pCuePreview = new ControlPushButton(ConfigKey(_group, "cue_preview"));
    connect(m_pCuePreview, SIGNAL(valueChanged(double)),
            this, SLOT(cuePreview(double)));

    m_pCueCDJ = new ControlPushButton(ConfigKey(_group, "cue_cdj"));
    connect(m_pCueCDJ, SIGNAL(valueChanged(double)),
            this, SLOT(cueCDJ(double)));

    m_pCueDefault = new ControlPushButton(ConfigKey(_group, "cue_default"));
    connect(m_pCueDefault, SIGNAL(valueChanged(double)),
            this, SLOT(cueDefault(double)));
}

CueControl::~CueControl() {
}

ConfigKey CueControl::keyForControl(int hotcue, QString name) {
    ConfigKey key;
    key.group = getGroup();
    key.item = QString("hotcue_%1_%2").arg(hotcue).arg(name);
    return key;
}

void CueControl::createControls() {
    for (int i = 0; i < m_iNumHotCues; ++i) {
        ControlObject* hotcuePosition = new ControlObject(
            keyForControl(i, "position"));
        hotcuePosition->set(-1);
        m_controlMap[hotcuePosition] = i;
        m_hotcuePosition.append(hotcuePosition);

        ControlObject* hotcueEnabled = new ControlObject(
            keyForControl(i, "enabled"));
        hotcueEnabled->set(0);
        m_controlMap[hotcueEnabled] = i;
        m_hotcueEnabled.append(hotcueEnabled);

        ControlObject* hotcueSet = new ControlPushButton(
            keyForControl(i, "set"));
        m_controlMap[hotcueSet] = i;
        connect(hotcueSet, SIGNAL(valueChanged(double)),
                this, SLOT(hotcueSet(double)));
        m_hotcueSet.append(hotcueSet);

        ControlObject* hotcueGoto = new ControlPushButton(
            keyForControl(i, "goto"));
        m_controlMap[hotcueGoto] = i;
        connect(hotcueGoto, SIGNAL(valueChanged(double)),
                this, SLOT(hotcueGoto(double)));
        m_hotcueGoto.append(hotcueGoto);

        ControlObject* hotcueGotoAndStop = new ControlPushButton(
            keyForControl(i, "gotoandstop"));
        m_controlMap[hotcueGotoAndStop] = i;
        connect(hotcueGotoAndStop, SIGNAL(valueChanged(double)),
                this, SLOT(hotcueGotoAndStop(double)));
        m_hotcueGotoAndStop.append(hotcueGotoAndStop);

        ControlObject* hotcueActivate = new ControlPushButton(
            keyForControl(i, "activate"));
        m_controlMap[hotcueActivate] = i;
        connect(hotcueActivate, SIGNAL(valueChanged(double)),
                this, SLOT(hotcueActivate(double)));
        m_hotcueActivate.append(hotcueActivate);

        ControlObject* hotcueActivatePreview = new ControlPushButton(
            keyForControl(i, "activate_preview"));
        m_controlMap[hotcueActivatePreview] = i;
        connect(hotcueActivatePreview, SIGNAL(valueChanged(double)),
                this, SLOT(hotcueActivatePreview(double)));
        m_hotcueActivatePreview.append(hotcueActivatePreview);

        ControlObject* hotcueClear = new ControlPushButton(
            keyForControl(i, "clear"));
        m_controlMap[hotcueClear] = i;
        connect(hotcueClear, SIGNAL(valueChanged(double)),
                this, SLOT(hotcueClear(double)));
        m_hotcueClear.append(hotcueClear);

        m_hotcue.append(NULL);
    }
}

void CueControl::attachCue(Cue* pCue, int hotCue) {
    Q_ASSERT(hotCue >= 0 && hotCue < m_iNumHotCues);
    if (m_hotcue[hotCue] != NULL) {
        detachCue(hotCue);
    }
    Q_ASSERT(m_hotcue[hotCue] == NULL);
    m_hotcue[hotCue] = pCue;
    connect(pCue, SIGNAL(updated()),
            this, SLOT(cueUpdated()));

    m_hotcuePosition[hotCue]->set(pCue->getPosition());
    m_hotcueEnabled[hotCue]->set(pCue->getPosition() == -1 ? 0.0 : 1.0);
}

void CueControl::detachCue(int hotCue) {
    Q_ASSERT(hotCue >= 0 && hotCue < m_iNumHotCues);
    Cue* pCue = m_hotcue[hotCue];
    if (!pCue)
        return;
    disconnect(pCue, 0, this, 0);
    m_hotcue[hotCue] = NULL;
    m_hotcuePosition[hotCue]->set(-1);
    m_hotcueEnabled[hotCue]->set(0);
}

void CueControl::loadTrack(TrackPointer pTrack) {
    Q_ASSERT(pTrack);

    QMutexLocker lock(&m_mutex);
    if (m_pLoadedTrack)
        unloadTrack(m_pLoadedTrack);

    m_pLoadedTrack = pTrack;
    connect(pTrack.data(), SIGNAL(cuesUpdated()),
            this, SLOT(trackCuesUpdated()));

    Cue* loadCue = NULL;
    Cue* otherCue = NULL;
    const QList<Cue*>& cuePoints = pTrack->getCuePoints();
    QListIterator<Cue*> it(cuePoints);
    while (it.hasNext()) {
        Cue* pCue = it.next();
        if (pCue->getType() == Cue::LOAD) {
            loadCue = pCue;
        } else if (pCue->getType() == Cue::CUE) {
            otherCue = pCue;
        } else {
            continue;
        }
        int hotcue = pCue->getHotCue();
        if (hotcue != -1)
            attachCue(pCue, hotcue);
    }

    // Prefer to load a LOAD cue, otherwise load any cue.
    if (loadCue == NULL && otherCue) {
        loadCue = otherCue;
    }

    if (loadCue != NULL) {
        m_pCuePoint->set(loadCue->getPosition());
    } else {
        m_pCuePoint->set(0.0f);
    }

    int cueRecall = getConfig()->getValueString(
        ConfigKey("[Controls]","CueRecall")).toInt();
    //If cue recall is ON in the prefs, then we're supposed to seek to the cue
    //point on song load. Note that cueRecall == 0 corresponds to "ON", not OFF.
    if (loadCue && cueRecall == 0) {
        double loadCuePoint = loadCue->getPosition();

        // Need to unlock before emitting any signals to prevent deadlock.
        lock.unlock();

        emit(seekAbs(loadCuePoint));
    }
}

void CueControl::unloadTrack(TrackPointer pTrack) {
    QMutexLocker lock(&m_mutex);
    disconnect(pTrack.data(), 0, this, 0);
    for (int i = 0; i < m_iNumHotCues; ++i) {
        detachCue(i);
    }

    // Store the cue point in a load cue.
    double cuePoint = m_pCuePoint->get();

    if (cuePoint != -1 && cuePoint != 0.0f) {
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

    m_pLoadedTrack.clear();
}

void CueControl::cueUpdated() {
    QMutexLocker lock(&m_mutex);
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
            Cue* pOldCue = m_hotcue[hotcue];

            // If the old hotcue is different than this one.
            if (pOldCue != pCue) {
                // If the old hotcue exists, detach it
                if (pOldCue != NULL)
                    detachCue(hotcue);
                attachCue(pCue, hotcue);
            } else {
                // If the old hotcue is the same, then we only need to update
                double dOldPosition = m_hotcuePosition[hotcue]->get();
                double dOldEnabled = m_hotcueEnabled[hotcue]->get();
                double dPosition = pCue->getPosition();
                double dEnabled = dPosition == -1 ? 0.0 : 1.0;
                if (dEnabled != dOldEnabled) {
                    m_hotcueEnabled[hotcue]->set(dEnabled);
                }
                if (dPosition != dOldPosition) {
                    m_hotcuePosition[hotcue]->set(dPosition);
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

int CueControl::senderHotcue(QObject* pSender) {
    Q_ASSERT(pSender);
    Q_ASSERT(m_controlMap.contains(pSender));
    return m_controlMap[pSender];
}

void CueControl::hotcueSet(double v) {
    qDebug() << "CueControl::hotcueSet" << v;

    if (v != 1.0)
        return;

    QMutexLocker lock(&m_mutex);
    if (!m_pLoadedTrack)
        return;

    int hotcue = senderHotcue(sender());
    detachCue(hotcue);
    Cue* pCue = m_pLoadedTrack->addCue();
    pCue->setPosition(getCurrentSample());
    pCue->setHotCue(hotcue);
    pCue->setLabel("");
    pCue->setType(Cue::CUE);
    // TODO(XXX) deal with spurious signals
    attachCue(pCue, hotcue);
}

void CueControl::hotcueGoto(double v) {
    if (v != 1.0)
        return;

    QMutexLocker lock(&m_mutex);
    if (!m_pLoadedTrack)
        return;

    int hotcue = senderHotcue(sender());
    Cue* pCue = m_hotcue[hotcue];

    // Need to unlock before emitting any signals to prevent deadlock.
    lock.unlock();

    if (pCue) {
        int position = pCue->getPosition();
        if (position != -1) {
            emit(seekAbs(position));
        }
    }
}

void CueControl::hotcueGotoAndStop(double v) {
    if (v != 1.0)
        return;

    QMutexLocker lock(&m_mutex);
    if (!m_pLoadedTrack)
        return;

    int hotcue = senderHotcue(sender());
    Cue* pCue = m_hotcue[hotcue];

    // Need to unlock before emitting any signals to prevent deadlock.
    lock.unlock();

    if (pCue) {
        int position = pCue->getPosition();
        if (position != -1) {
            m_pPlayButton->set(0.0);

            emit(seekAbs(position));
        }
    }
}

void CueControl::hotcueActivate(double v) {
    qDebug() << "CueControl::hotcueActivate" << v;

    QMutexLocker lock(&m_mutex);

    if (!m_pLoadedTrack)
        return;

    int hotcue = senderHotcue(sender());
    Cue* pCue = m_hotcue[hotcue];

    lock.unlock();

    if (pCue) {
        if (v == 1.0f) {
            if (pCue->getPosition() == -1) {
                hotcueSet(v);
            } else {
                if (m_pPlayButton->get() == 1.0f) {
                    hotcueGoto(v);
                } else {
                    hotcueActivatePreview(v);
                }
            }
        } else {
            if (pCue->getPosition() != -1) {
                hotcueActivatePreview(v);
            }
        }
    } else {
        if (v == 1.0f) {
            hotcueSet(v);
        }
    }
}

void CueControl::hotcueActivatePreview(double v) {
    QMutexLocker lock(&m_mutex);
    if (!m_pLoadedTrack)
        return;
    int hotcue = senderHotcue(sender());
    Cue* pCue = m_hotcue[hotcue];

    if (v == 1.0) {
        if (pCue && pCue->getPosition() != -1) {
            int iPosition = pCue->getPosition();
            m_pPlayButton->set(1.0);
            m_bPreviewingHotcue = true;

            // Need to unlock before emitting any signals to prevent deadlock.
            lock.unlock();

            emit(seekAbs(iPosition));
        }
    } else {
        if (m_bPreviewingHotcue && pCue && pCue->getPosition() != -1) {
            int iPosition = pCue->getPosition();
            m_pPlayButton->set(0.0);
            m_bPreviewingHotcue = false;

            // Need to unlock before emitting any signals to prevent deadlock.
            lock.unlock();

            emit(seekAbs(iPosition));
        }
    }
}

void CueControl::hotcueClear(double v) {
    if (v != 1.0)
        return;

    QMutexLocker lock(&m_mutex);
    if (!m_pLoadedTrack)
        return;

    int hotcue = senderHotcue(sender());

    Cue* pCue = m_hotcue[hotcue];
    if (pCue) {
        pCue->setHotCue(-1);
    }

    detachCue(hotcue);
}

void CueControl::hintReader(QList<Hint>& hintList) {
    QMutexLocker lock(&m_mutex);

    Hint cue_hint;
    double cuePoint = m_pCuePoint->get();
    if (cuePoint >= 0) {
        cue_hint.sample = m_pCuePoint->get();
        cue_hint.length = 0;
        cue_hint.priority = 10;
        hintList.append(cue_hint);
    }

    for (int i = 0; i < m_iNumHotCues; ++i) {
        if (m_hotcue[i] != NULL) {
            double position = m_hotcuePosition[i]->get();
            if (position != -1) {
                cue_hint.sample = position;
                if (cue_hint.sample % 2 != 0)
                    cue_hint.sample--;
                cue_hint.length = 0;
                cue_hint.priority = 10;
                hintList.push_back(cue_hint);
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
    if (v != 1.0)
        return;

    QMutexLocker lock(&m_mutex);
    double cue = math_max(0.,round(getCurrentSample()));
    if (!even((int)cue))
        cue--;
    m_pCuePoint->set(cue);
    saveCuePoint(cue);
}

void CueControl::cueGoto(double v)
{
    if (v != 1.0f)
        return;

    QMutexLocker lock(&m_mutex);
    // Set cue point if play is not pressed
    if (m_pPlayButton->get()==0.) {
        // Set the cue point and play
        cueSet(v);
        m_pPlayButton->set(1.0);
    } else {
        // Seek to cue point
        double cuePoint = m_pCuePoint->get();

        // Need to unlock before emitting any signals to prevent deadlock.
        lock.unlock();

        emit(seekAbs(cuePoint));
    }
}

void CueControl::cueGotoAndStop(double v)
{
    if (v != 1.0f)
        return;

    QMutexLocker lock(&m_mutex);
    m_pPlayButton->set(0.0);
    double cuePoint = m_pCuePoint->get();

    // Need to unlock before emitting any signals to prevent deadlock.
    lock.unlock();

    emit(seekAbs(cuePoint));
}

void CueControl::cuePreview(double v)
{
    QMutexLocker lock(&m_mutex);

    if (v == 1.0f) {
        m_pPlayButton->set(1.0);
        m_bPreviewing = true;
    } else if (v == 0.0f && m_bPreviewing) {
        m_pPlayButton->set(0.0);
        m_bPreviewing = false;
    }

    double cuePoint = m_pCuePoint->get();

    // Need to unlock before emitting any signals to prevent deadlock.
    lock.unlock();

    emit(seekAbs(cuePoint));
}

void CueControl::cueSimple(double v) {
    if (v != 1.0f)
        return;

    QMutexLocker lock(&m_mutex);
    // Simple cueing is if the player is not playing, set the cue point --
    // otherwise seek to the cue point.
    if (m_pPlayButton->get() == 0.0f) {
        return cueSet(v);
    }

    double cuePoint = m_pCuePoint->get();

    // Need to unlock before emitting any signals to prevent deadlock.
    lock.unlock();

    emit(seekAbs(cuePoint));
}

void CueControl::cueCDJ(double v) {
    /* This is how CDJ cue buttons work:
     * If pressed while playing, stop playback at go to cue.
     * If pressed while stopped and at cue, play while pressed.
     * If pressed while stopped and not at cue, set new cue point.
     * TODO: If play is pressed while holding cue, the deck is now playing.
     */

    QMutexLocker lock(&m_mutex);
    bool playing = (m_pPlayButton->get() == 1.0);
    double cuePoint = m_pCuePoint->get();

    if (v == 1.0f) {
        if (playing) {
            m_pPlayButton->set(0.0);

            // Just in case.
            m_bPreviewing = false;

            // Need to unlock before emitting any signals to prevent deadlock.
            lock.unlock();

            emit(seekAbs(cuePoint));
        } else {
            if (fabs(getCurrentSample() - m_pCuePoint->get()) < 1.0f) {
                m_pPlayButton->set(1.0);
                m_bPreviewing = true;
            } else {
                cueSet(v);
                // Just in case.
                m_bPreviewing = false;
            }
        }
    } else if (m_bPreviewing) {
        m_pPlayButton->set(0.0);
        m_bPreviewing = false;

        // Need to unlock before emitting any signals to prevent deadlock.
        lock.unlock();

        emit(seekAbs(cuePoint));
    }
}

void CueControl::cueDefault(double v) {
    // Decide which cue implementation to call based on the user preference
    if (m_pCueMode->get() == 0.0f) {
        cueCDJ(v);
    } else {
        cueSimple(v);
    }
}

