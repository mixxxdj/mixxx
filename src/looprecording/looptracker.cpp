//  looptracker.cpp
//  Created by Carl Pillot on 8/23/13.
//
// TODO: Should this be declared inline in LoopRecordingManager?

#include "looprecording/defs_looprecording.h"
#include "looprecording/looptracker.h"
#include "controlobjectthread.h"
#include "trackinfoobject.h"

LoopTracker::LoopTracker()
        : m_iCurrentLayer(-1),
        m_bIsUndoAvailable(false),
        m_bIsRedoAvailable(false) {

    m_pLoopDeck1Play = new ControlObjectThread("[LoopRecorderDeck1]","play");
    m_pLoopDeck1Stop = new ControlObjectThread("[LoopRecorderDeck1]","stop");
    m_pLoopDeck1Eject = new ControlObjectThread("[LoopRecorderDeck1]","eject");
    m_pLoopDeck2Play = new ControlObjectThread("[LoopRecorderDeck2]","play");
    m_pLoopDeck2Stop = new ControlObjectThread("[LoopRecorderDeck2]","stop");
    m_pLoopDeck2Eject = new ControlObjectThread("[LoopRecorderDeck2]","eject");


    m_pTogglePlayback = new ControlObjectThread(LOOP_RECORDING_PREF_KEY, "toggle_playback");

}

LoopTracker::~LoopTracker() {
    clear();

    delete m_pTogglePlayback;
    delete m_pLoopDeck2Eject;
    delete m_pLoopDeck2Stop;
    delete m_pLoopDeck2Play;
    delete m_pLoopDeck1Eject;
    delete m_pLoopDeck1Stop;
    delete m_pLoopDeck1Play;
}

void LoopTracker::addLoopLayer(QString path, unsigned int length) {
    LayerInfo* layer = new LayerInfo();
    layer->path = path;
    layer->length = length;
    m_layers.append(layer);
    m_iCurrentLayer++;
}

void LoopTracker::clear() {
    while (!m_layers.empty()) {
        LayerInfo* pFile = m_layers.takeLast();
        qDebug() << "!~!~!~! LoopTracker::clearLayers deleteing: " << pFile->path;
        QFile file(pFile->path);

        if (file.exists()) {
            file.remove();
        }
        delete pFile;
    }
    m_iCurrentLayer = -1;
}

bool LoopTracker::finalizeLoop(QString newPath) {
    // TODO: implement multiple layer mixing.
    if (m_iCurrentLayer > -1) {

        QString oldFileLocation = m_layers.at(m_iCurrentLayer)->path;
        QFile file(oldFileLocation);

        if (file.exists()) {
            return file.copy(newPath);
        }
    }
    return false;
}

void LoopTracker::play() {
    m_pLoopDeck1Play->slotSet(1.0);
    m_pLoopDeck2Play->slotSet(1.0);
}

void LoopTracker::stop(bool clearDeck) {
    m_pLoopDeck1Play->slotSet(0.0);
    m_pLoopDeck2Play->slotSet(0.0);
    if(clearDeck) {
        m_pLoopDeck1Eject->slotSet(1.0);
        m_pLoopDeck2Eject->slotSet(1.0);
        m_pLoopDeck1Eject->slotSet(0.0);
        m_pLoopDeck2Eject->slotSet(0.0);
    }
}

QString LoopTracker::getCurrentPath() {
    if (m_layers.empty()) {
        return QString::QString("");
    } else {
        return m_layers.at(m_iCurrentLayer)->path;
    }
}

void LoopTracker::setCurrentLength(unsigned int length) {

}

void LoopTracker::slotLoadToLoopDeck() {
    //qDebug() << "LoopRecordingManager::loadToLoopDeck m_filesRecorded: " << m_filesRecorded;
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
