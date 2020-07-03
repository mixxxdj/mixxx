#pragma once

#include <QDateTime>
#include <QList>
#include <QObject>
#include <QString>

class RecordingManagerBase : public QObject {
    Q_OBJECT
  public:
    virtual void startRecording() = 0;
    virtual void stopRecording() = 0;
    virtual bool isRecordingActive() = 0;

  public slots:
    void slotSetRecording(bool recording);
    void slotToggleRecording(double value);
};
