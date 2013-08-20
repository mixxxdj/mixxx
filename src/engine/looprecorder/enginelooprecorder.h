//  enginelooprecorder.h
//  Created by Carl Pillot on 6/22/13.
//  Adapted from EngineLoopRecorder.h

#ifndef ENGINELOOPRECORDER_H
#define ENGINELOOPRECORDER_H

#include <QtCore>

#include "defs.h"

class LoopWriter;

class EngineLoopRecorder : public QObject {
    Q_OBJECT
  public:
    EngineLoopRecorder();
    virtual ~EngineLoopRecorder();

    // Should this be process instead?
    // Passes buffers to the LoopWriter object
    void writeSamples(const CSAMPLE* pBuffer, const int iBufferSize);

    // Moves the recorder object to the new thread and starts execution.
    void startThread();

    // Returns a pointer to the LoopWriter object.
    // Used to connect signals and slots with the LoopRecordingManager.
    LoopWriter* getLoopWriter() {
        return m_pLoopWriter;
    }

  private slots:
    void slotThreadStarted();

  private:
    QThread* LoopRecorderThread;
    LoopWriter* m_pLoopWriter;

    bool m_bIsThreadReady;
};

#endif
