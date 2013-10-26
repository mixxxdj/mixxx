// loopfilemixer.h
// Create by Carl Pillot on 9/21/13

#ifndef LOOPFILEMIXER_H
#define LOOPFILEMIXER_H

#include <QObject>
#include <sndfile.h>

#include "defs.h"

class LoopFileMixer : public QObject {
    Q_OBJECT
  public:
    LoopFileMixer(QString file1, int length, QString dest, QString encoding);
    LoopFileMixer(QString file1, QString file2, int length, QString dest, QString encoding);
    virtual ~LoopFileMixer();

  public slots:
    void slotProcess();

  signals:
    void fileFinished(QString);
    void finished();

  private:
    QString m_dest;
    QString m_encoding;
    QString m_filePath1;
    QString m_filePath2;

    int m_iLength;
    int m_iNumFiles;

    CSAMPLE* m_pWorkBufferIn1;
    CSAMPLE* m_pWorkBufferIn2;
    CSAMPLE* m_pWorkBufferOut;
};
#endif
