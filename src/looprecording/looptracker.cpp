//  looptracker.cpp
//  Created by Carl Pillot on 8/23/13.
//
// TODO: Should this be declared inline in LoopRecordingManager?

#include "looprecording/looptracker.h"


LoopTracker::LoopTracker() : currentLayer(0) { }

LoopTracker::~LoopTracker() {
    clearLayers();
}

void LoopTracker::addLoopLayer(QString path, unsigned int length) {
    LayerInfo* layer = new LayerInfo();
    layer->path = path;
    layer->length = length;
    m_layers.append(layer);
}

void LoopTracker::clearLayers() {
    while (!m_layers.empty()) {
        LayerInfo* pFile = m_layers.takeLast();
        qDebug() << "!~!~!~! LoopTracker::clearLayers deleteing: " << pFile->path;
        QFile file(pFile->path);

        if (file.exists()) {
            file.remove();
        }
        delete pFile;
    }
}

void LoopTracker::setCurrentLength(unsigned int length) {

}