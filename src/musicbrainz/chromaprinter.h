#ifndef CHROMAPRINTER_H
#define CHROMAPRINTER_H

#include <QObject>

#include "trackinfoobject.h"

class SoundSourceProxy;

class chromaprinter: public QObject {
  Q_OBJECT

  public:
    chromaprinter(QObject* parent=NULL);
    QString getFingerPrint(TrackPointer pTrack);
    QString getFingerPrint(QString location);

  private:

    QString calcFingerPrint(SoundSourceProxy& soundSource);
    unsigned int m_NumSamples;
    unsigned int m_SampleRate;
};

#endif //CHROMAPRINTER_H
