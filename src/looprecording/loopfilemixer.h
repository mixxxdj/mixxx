// LoopFileMixer.h
// Create by Carl Pillot on 8/22/13

#ifndef LOOPFILEMIXER_H
#define LOOPFILEMIXER_H

#include <QObject>
#include <sndfile.h>

class LoopFileMixer : public QObject {
    Q_OBJECT
  public:
    LoopFileMixer(QString file1, QString file2);
    virtual ~LoopFileMixer();

  private:
    QString m_filePath1;
    QString m_filePath2;
};
#endif