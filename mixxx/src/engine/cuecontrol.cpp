// cuecontrol.cpp
// Created 11/5/2009 by RJ Ryan (rryan@mit.edu)

#include <QMutexLocker>

#include "engine/cuecontrol.h"

#include "controlobject.h"
#include "controlpushbutton.h"
#include "trackinfoobject.h"
#include "library/dao/cue.h"
#include "cachingreader.h"

#define NUM_HOT_CUES 32

CueControl::CueControl(const char * _group,
                       const ConfigObject<ConfigValue> * _config) :
        EngineControl(_group, _config),
        m_bPreviewing(false),
        m_pPlayButton(ControlObject::getControl(ConfigKey(_group, "play"))),
        m_iNumHotCues(NUM_HOT_CUES),
        m_pLoadedTrack(NULL),
        m_mutex(QMutex::Recursive) {
    createControls();
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
        m_controlMap[hotcuePosition] = i;
        m_hotcuePosition.append(hotcuePosition);

        ControlObject* hotcueEnabled = new ControlObject(
            keyForControl(i, "enabled"));
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

void CueControl::loadTrack(TrackInfoObject* pTrack) {
    Q_ASSERT(pTrack);

    QMutexLocker lock(&m_mutex);
    if (m_pLoadedTrack)
        unloadTrack(m_pLoadedTrack);

    m_pLoadedTrack = pTrack;
    const QList<Cue*>& cuePoints = pTrack->getCuePoints();
    QListIterator<Cue*> it(cuePoints);
    while (it.hasNext()) {
        Cue* pCue = it.next();
        if (pCue->getType() != Cue::CUE && pCue->getType() != Cue::LOAD)
            continue;
        int hotcue = pCue->getHotCue();
        if (hotcue != -1)
            attachCue(pCue, hotcue);
    }
}

void CueControl::unloadTrack(TrackInfoObject* pTrack) {
    QMutexLocker lock(&m_mutex);
    disconnect(pTrack, 0, this, 0);
    for (int i = 0; i < m_iNumHotCues; ++i) {
        detachCue(i);
    }
    m_pLoadedTrack = NULL;
}

void CueControl::cueUpdated() {
    QMutexLocker lock(&m_mutex);
    // We should get a trackCuesUpdated call anyway, so do nothing.
}

void CueControl::trackCuesUpdated() {
    QMutexLocker lock(&m_mutex);

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
            if (m_hotcue[hotcue] != pCue) {
                detachCue(hotcue);
                attachCue(pCue, hotcue);
            }
        }
    }
}

int CueControl::senderHotcue(QObject* pSender) {
    Q_ASSERT(pSender);
    Q_ASSERT(m_controlMap.contains(pSender));
    return m_controlMap[pSender];
}

void CueControl::hotcueSet(double v) {
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
    if (pCue) {
        int position = pCue->getPosition();
        if (position != -1) {
            emit(seekAbs(position));
            m_pPlayButton->set(0.0);
        }
    }
}

void CueControl::hotcueActivate(double v) {
    if (v != 1.0)
        return;

    QMutexLocker lock(&m_mutex);
    if (!m_pLoadedTrack)
        return;

    int hotcue = senderHotcue(sender());
    Cue* pCue = m_hotcue[hotcue];

    if (pCue) {
        if (pCue->getPosition() == -1) {
            hotcueSet(v);
        } else {
            hotcueGoto(v);
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
            emit(seekAbs(iPosition));
            m_pPlayButton->set(1.0);
            m_bPreviewing = true;
        }
    } else {
        if (m_bPreviewing && pCue && pCue->getPosition() != -1) {
            int iPosition = pCue->getPosition();
            emit(seekAbs(iPosition));
            m_pPlayButton->set(0.0);
            m_bPreviewing = false;
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
    detachCue(hotcue);
}

void CueControl::hintReader(QList<Hint>& hintList) {
    Hint hint;
    for (int i = 0; i < m_iNumHotCues; ++i) {
        if (m_hotcue[i] != NULL) {
            double position = m_hotcuePosition[i]->get();
            if (position != -1) {
                hint.sample = position;
                if (hint.sample % 2 != 0)
                    hint.sample--;
                hint.length = 0;
                hint.priority = 10;
                hintList.push_back(hint);
            }
        }
    }
}
