// Thanks to Clementine
#ifndef CHROMAPRINTER_H
#define CHROMAPRINTER_H

#include <QObject>
#include <QFile>

#include "trackinfoobject.h"

class SoundSourceProxy;

class chromaprinter: public QObject {
  Q_OBJECT

  public:
    chromaprinter(QObject* parent=NULL);
    QString getFingerPrint(TrackPointer pTrack, bool mixxx=true);
    QString getFingerPrint(QString location, bool mixxx=true);

  private:

    QString calcFingerPrint(SoundSourceProxy& soundSource, bool mixxx);
    unsigned int m_NumSamples;
    unsigned int m_SampleRate;
};

#endif //CHROMAPRINTER_H
