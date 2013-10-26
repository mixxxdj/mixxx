// looplayerracker.cpp
// Created by Carl Pillot on 8/23/13.

#include <QThread>

#include "looprecording/looplayertracker.h"

#include "looprecording/loopfilemixer.h"
#include "recording/defs_recording.h"
#include "controlobjectthread.h"
#include "controllogpotmeter.h"
#include "trackinfoobject.h"

LoopLayerTracker::LoopLayerTracker(ConfigObject<ConfigValue>* pConfig)
        : m_pConfig(pConfig),
          m_iCurrentLayer(-1),
          m_bIsUndoAvailable(false),
          m_bIsRedoAvailable(false) {

    m_pLoopDeck1Play = new ControlObjectThread("[LoopRecorderDeck1]","play");
    m_pLoopDeck1Stop = new ControlObjectThread("[LoopRecorderDeck1]","stop");
    m_pLoopDeck1Eject = new ControlObjectThread("[LoopRecorderDeck1]","eject");
    m_pLoopDeck1Pregain = new ControlObjectThread("[LoopRecorderDeck1]","pregain");
    m_pLoopDeck1LoopIn = new ControlObjectThread("[LoopRecorderDeck1]","loop_start_position");
    m_pLoopDeck1LoopOut = new ControlObjectThread("[LoopRecorderDeck1]","loop_end_position");
    m_pLoopDeck1Reloop = new ControlObjectThread("[LoopRecorderDeck1]","reloop_exit");
    m_pLoopDeck1LoopEnabled = new ControlObjectThread("[LoopRecorderDeck1]","loop_enabled");
    m_pLoopDeck2Play = new ControlObjectThread("[LoopRecorderDeck2]","play");
    m_pLoopDeck2Stop = new ControlObjectThread("[LoopRecorderDeck2]","stop");
    m_pLoopDeck2Eject = new ControlObjectThread("[LoopRecorderDeck2]","eject");
    m_pLoopDeck2Pregain = new ControlObjectThread("[LoopRecorderDeck2]","pregain");

    m_pTogglePlayback = new ControlObjectThread(LOOP_RECORDING_PREF_KEY, "toggle_playback");
    m_pLoopPregain = new ControlLogpotmeter(ConfigKey(LOOP_RECORDING_PREF_KEY, "pregain"));

    connect(m_pLoopPregain, SIGNAL(valueChanged(double)),
            this, SLOT(slotChangeLoopPregain(double)));

    m_pLoopDeck1LoopIn->slotSet(0.0);
}

LoopLayerTracker::~LoopLayerTracker() {
    clear();

    delete m_pLoopPregain;
    delete m_pTogglePlayback;
    delete m_pLoopDeck2Pregain;
    delete m_pLoopDeck2Eject;
    delete m_pLoopDeck2Stop;
    delete m_pLoopDeck2Play;
    delete m_pLoopDeck1LoopEnabled;
    delete m_pLoopDeck1Reloop;
    delete m_pLoopDeck1LoopOut;
    delete m_pLoopDeck1LoopIn;
    delete m_pLoopDeck1Pregain;
    delete m_pLoopDeck1Eject;
    delete m_pLoopDeck1Stop;
    delete m_pLoopDeck1Play;
}

void LoopLayerTracker::addLoopLayer(QString path, int length, int iSampleRate) {
    LayerInfo* layer = new LayerInfo();
    layer->path = path;
    layer->length = length;
    layer->sampleRate = iSampleRate;
    m_layers.append(layer);
    m_iCurrentLayer++;
}

void LoopLayerTracker::clear() {
    while (!m_layers.empty()) {
        LayerInfo* pFile = m_layers.takeLast();
        qDebug() << "!~!~!~! LoopLayerTracker::clearLayers deleteing: " << pFile->path;
        QFile file(pFile->path);

        if (file.exists()) {
            file.remove();
        }
        delete pFile;
    }
    m_iCurrentLayer = -1;
}

void LoopLayerTracker::finalizeLoop(QString newPath, double bpm) {

    if (m_iCurrentLayer < 0) {
        return;
    }

    qDebug() << "Finalize Loop: " << m_iCurrentLayer << " Layers Size" << m_layers.size();

    QThread* pMixerThread = new QThread();
    pMixerThread->setObjectName("LoopLayerMixer");

    QString encoding = m_pConfig->getValueString(ConfigKey(LOOP_RECORDING_PREF_KEY, "Encoding"));
    LoopFileMixer* pMixerWorker = NULL;

    pMixerWorker = new LoopFileMixer(*m_layers[0], newPath, encoding);

    connect(pMixerThread, SIGNAL(started()), pMixerWorker, SLOT(slotProcess()));
    connect(pMixerWorker, SIGNAL(fileFinished(QString)), this, SLOT(slotFileFinished(QString)));
    connect(pMixerWorker, SIGNAL(finished()), pMixerThread, SLOT(quit()));
    connect(pMixerWorker, SIGNAL(finished()), pMixerWorker, SLOT(deleteLater()));
    connect(pMixerThread, SIGNAL(finished()), pMixerThread, SLOT(deleteLater()));

    pMixerWorker->moveToThread(pMixerThread);
    pMixerThread->start();
}

void LoopLayerTracker::play() {
    m_pLoopDeck1Play->slotSet(1.0);
    m_pLoopDeck2Play->slotSet(1.0);
}

void LoopLayerTracker::stop(bool clearDeck) {
    m_pLoopDeck1Play->slotSet(0.0);
    m_pLoopDeck2Play->slotSet(0.0);

    if(clearDeck) {
        // Turn off looping
        if (m_pLoopDeck1LoopEnabled->get()) {
            m_pLoopDeck1Reloop->slotSet(1.0);
            m_pLoopDeck1Reloop->slotSet(0.0);
        }

        m_pLoopDeck1Eject->slotSet(1.0);
        m_pLoopDeck2Eject->slotSet(1.0);
        m_pLoopDeck1Eject->slotSet(0.0);
        m_pLoopDeck2Eject->slotSet(0.0);
    }
}

QString LoopLayerTracker::getCurrentPath() {
    if (m_layers.empty()) {
        return QString("");
    } else {
        return m_layers.at(m_iCurrentLayer)->path;
    }
}

int LoopLayerTracker::getCurrentLength() {
    if (m_layers.empty()) {
        return 0;
    } else {
        return m_layers.at(m_iCurrentLayer)->length;
    }
}

void LoopLayerTracker::setCurrentLength(int length) {
    if (m_layers.empty()) {
        return;
    }
    m_layers.at(m_iCurrentLayer)->length = length;
}

void LoopLayerTracker::slotFileFinished(QString path) {
    emit(exportLoop(path));
}

void LoopLayerTracker::slotLoadToLoopDeck(int totalSamplesRecorded) {

    // If we're recording a loop without a specified length, we need to set it here.
    if (getCurrentLength() == 0) {
        // We have to subtract some samples so that it doesn't end at the end of a file.
        setCurrentLength(totalSamplesRecorded-10);
    }

    QString path = getCurrentPath();
    if (path != "") {
        TrackPointer pTrackToPlay = TrackPointer(new TrackInfoObject(path), &QObject::deleteLater);

        // Signal to Player manager to load and play track.
        if (m_iCurrentLayer == 0) {
            emit(loadToLoopDeck(pTrackToPlay, QString("[LoopRecorderDeck1]"), true));

        } else if (m_iCurrentLayer == 1) {
            emit(loadToLoopDeck(pTrackToPlay, QString("[LoopRecorderDeck2]"), true));
        }
        m_pTogglePlayback->set(1.0);
    }
}

void LoopLayerTracker::slotLoop1Loaded(TrackPointer pTrack) {
    m_pLoopDeck1LoopIn->slotSet(0.0);
    m_pLoopDeck1LoopOut->slotSet(getCurrentLength());
    m_pLoopDeck1Reloop->slotSet(1.0);
    m_pLoopDeck1Reloop->slotSet(0.0);

    qDebug() << "slotLoop1Loaded Loop Enabled" << m_pLoopDeck1LoopEnabled->get();

}

void LoopLayerTracker::slotChangeLoopPregain(double v) {
    m_pLoopDeck1Pregain->slotSet(v);
    m_pLoopDeck2Pregain->slotSet(v);
}
