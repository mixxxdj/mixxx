//  enginelooprecorder.cpp
//  Created by Carl Pillot on 6/22/13.
//  Adapted from EngineLoopRecorder.cpp

#include "engine/looprecorder/enginelooprecorder.h"

#include "configobject.h"
#include "errordialoghandler.h"
#include "looprecording/defs_looprecording.h"
#include "engine/looprecorder/loopwriter.h"

EngineLoopRecorder::EngineLoopRecorder()
        : m_bIsThreadReady(false) {

    m_pLoopWriter = new LoopWriter();

    LoopRecorderThread = new QThread;
    LoopRecorderThread->setObjectName(QString("LoopRecorder"));

    connect(LoopRecorderThread, SIGNAL(started()), this, SLOT(slotThreadStarted()));

    // TODO(carl) make sure Thread exits properly.
    connect(m_pLoopWriter, SIGNAL(finished()), LoopRecorderThread, SLOT(quit()));
    connect(LoopRecorderThread, SIGNAL(finished()), LoopRecorderThread, SLOT(deleteLater()));
}

EngineLoopRecorder::~EngineLoopRecorder() {
    m_pLoopWriter->deleteLater();
}

void EngineLoopRecorder::writeSamples(const CSAMPLE* pBuffer, const int iBufferSize) {
    if(!m_bIsThreadReady) {
        return;
    }

    m_pLoopWriter->process(pBuffer, iBufferSize);
}

void EngineLoopRecorder::startThread() {
    qDebug() << "!~!~!~! EngineLoopRecorder::startThread() !~!~!~!";
    m_pLoopWriter->moveToThread(LoopRecorderThread);
    LoopRecorderThread->start();
}

void EngineLoopRecorder::slotThreadStarted() {
    qDebug() << "!~!~!~! EngineLoopRecorder::slotThreadStarted() !~!~!~!";
    m_bIsThreadReady = true;
}
