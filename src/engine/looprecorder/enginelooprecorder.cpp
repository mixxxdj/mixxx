//  enginelooprecorder.cpp
//  Created by Carl Pillot on 6/22/13.
//  Adapted from EngineLoopRecorder.cpp

#include "engine/looprecorder/enginelooprecorder.h"

#include "configobject.h"
#include "errordialoghandler.h"
#include "engine/looprecorder/loopwriter.h"

EngineLoopRecorder::EngineLoopRecorder()
        : m_loopSource(""),
          m_bThreadReady(false) {

    m_pLoopWriter = new LoopWriter();

    LoopRecorderThread = new QThread;
    LoopRecorderThread->setObjectName("LoopRecorder");

    connect(LoopRecorderThread, SIGNAL(started()), this, SLOT(slotThreadStarted()));

    // TODO(carl) make sure Thread exits properly.
    connect(m_pLoopWriter, SIGNAL(destroyed()), LoopRecorderThread, SLOT(quit()));
    connect(LoopRecorderThread, SIGNAL(finished()), LoopRecorderThread, SLOT(deleteLater()));
}

EngineLoopRecorder::~EngineLoopRecorder() {
    m_pLoopWriter->deleteLater();
}

void EngineLoopRecorder::writeSamples(const CSAMPLE* pBuffer, const int iBufferSize) {
    if(!m_bThreadReady) {
        return;
    }

    m_pLoopWriter->process(pBuffer, iBufferSize);
}

void EngineLoopRecorder::startThread() {
    //qDebug() << "!~!~!~! EngineLoopRecorder::startThread() !~!~!~!";
    m_pLoopWriter->moveToThread(LoopRecorderThread);
    LoopRecorderThread->start();
}

void EngineLoopRecorder::slotSourceChanged(QString source) {
    m_loopSource = source;
    //qDebug() << "!~!~!~! EngineLoopRecorder::slotSourceChanged: " << m_loopSource << " !~!~!~!";
}

void EngineLoopRecorder::slotThreadStarted() {
    //qDebug() << "!~!~!~! EngineLoopRecorder::slotThreadStarted() !~!~!~!";
    m_bThreadReady = true;
}
