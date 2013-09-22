// LoopFileMixer.h
// Create by Carl Pillot on 8/22/13

#ifndef LOOPFILEMIXER_H
#define LOOPFILEMIXER_H

#include <QObject>
#include <sndfile.h>

#include "configobject.h"

class LoopFileMixer : public QObject {
    Q_OBJECT
  public:
    LoopFileMixer(ConfigObject<ConfigValue>* pConfig, QString file1, QString file2, QString dest);
    virtual ~LoopFileMixer();

  public slots:
    void slotProcess();

  signals:
    void fileFinished(QString);
    void finished();

  private:
    ConfigObject<ConfigValue>* m_pConfig;
    QString m_filePath1;
    QString m_filePath2;
    QString m_dest;
};
#endif