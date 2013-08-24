//  looptracker.cpp
//  Created by Carl Pillot on 8/23/13.
//
// TODO: Should this be declared inline in LoopRecordingManager?

#include "looprecording/looptracker.h"


LoopTracker::LoopTracker()
        : currentLayer(-1),
        m_bIsUndoAvailable(false),
        m_bIsRedoAvailable(false) { }

LoopTracker::~LoopTracker() {
    clear();
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

QString LoopTracker::getCurrentPath() {
    if (m_layers.empty()) {
        return QString::QString("");
    } else {
        return m_layers.at(currentLayer)->path;
    }
}

void LoopTracker::setCurrentLength(unsigned int length) {

}