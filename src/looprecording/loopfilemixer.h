// loopfilemixer.h
// Create by Carl Pillot on 9/21/13
// Note this currently only copies one loop layer.
// In the future it will handle multiple layers.

#ifndef LOOPFILEMIXER_H
#define LOOPFILEMIXER_H

#include <QObject>
#include <sndfile.h>

#include "defs.h"
#include "looprecording/layerinfo.h"


class LoopFileMixer : public QObject {
    Q_OBJECT
  public:
    LoopFileMixer(LayerInfo file1, QString dest, QString encoding);
    virtual ~LoopFileMixer();

  public slots:
    void slotProcess();

  signals:
    void fileFinished(QString);
    void finished();

  private:
    QString m_dest;
    QString m_encoding;
    LayerInfo m_file1;

    int m_iNumFiles;
    int m_iLength;

    CSAMPLE* m_pWorkBufferIn1;
    CSAMPLE* m_pWorkBufferOut;
};
#endif
