//  looptracker.cpp
//  Created by Carl Pillot on 8/23/13.
//
// TODO: Should this be declared inline in LoopRecordingManager?

#include "looprecording/looptracker.h"
#include "controlobjectthread.h"

LoopTracker::LoopTracker()
        : currentLayer(-1),
        m_bIsUndoAvailable(false),
        m_bIsRedoAvailable(false) {

    m_pLoopDeck1Play = new ControlObjectThread("[LoopRecorderDeck1]","play");
    m_pLoopDeck1Stop = new ControlObjectThread("[LoopRecorderDeck1]","stop");
    m_pLoopDeck1Eject = new ControlObjectThread("[LoopRecorderDeck1]","eject");
}

LoopTracker::~LoopTracker() {
    clear();

    delete m_pLoopDeck1Eject;
    delete m_pLoopDeck1Stop;
    delete m_pLoopDeck1Play;
}

void LoopTracker::addLoopLayer(QString path, unsigned int length) {
    LayerInfo* layer = new LayerInfo();
    layer->path = path;
    layer->length = length;
    m_layers.append(layer);
    currentLayer++;
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
    currentLayer = -1;
}

bool LoopTracker::finalizeLoop(QString newPath) {
    // TODO: implement multiple layer mixing.
    if (currentLayer > -1) {

        QString oldFileLocation = m_layers.at(currentLayer)->path;
        QFile file(oldFileLocation);

        if (file.exists()) {
            return file.copy(newPath);
        }
    }
    return false;
}

void LoopTracker::play() {
    m_pLoopDeck1Play->slotSet(1.0);
}

void LoopTracker::stop(bool clearDeck) {
    m_pLoopDeck1Play->slotSet(0.0);
    if(clearDeck) {
        m_pLoopDeck1Eject->slotSet(1.0);
        m_pLoopDeck1Eject->slotSet(0.0);
    }
}

QString LoopTracker::getCurrentPath() {
    if (m_layers.empty()) {
        return QString::QString("");
    } else {
        return m_layers.at(currentLayer)->path;
    }
}

void LoopTracker::setCurrentLength(unsigned int length) {

}