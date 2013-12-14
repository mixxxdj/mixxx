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

    // Passes buffers to the LoopWriter object
    void writeSamples(const CSAMPLE* pBuffer, const int iBufferSize);

    // Moves the recorder object to the new thread and starts execution.
    void startThread();

    // Returns the currently selected loop recording source channel.
    QString getLoopSource() {
        return m_loopSource;
    }

    // Returns a pointer to the LoopWriter object.
    // Used to connect signals and slots with the LoopRecordingManager.
    LoopWriter* getLoopWriter() {
        return m_pLoopWriter;
    }

  public slots:
    void slotSourceChanged(QString);

  private slots:
    void slotThreadStarted();

  private:
    QThread* LoopRecorderThread;
    LoopWriter* m_pLoopWriter;

    QString m_loopSource;
    bool m_bThreadReady;
};

#endif
