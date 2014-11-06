#ifndef CHROMAPRINTER_H
#define CHROMAPRINTER_H

#include <QObject>

#include "trackinfoobject.h"

// forward declaration(s)
namespace Mixxx
{
    class SoundSource;
}

class chromaprinter: public QObject {
  Q_OBJECT

  public:
    chromaprinter(QObject* parent=NULL);
    QString getFingerPrint(TrackPointer pTrack);

  private:

    QString calcFingerPrint(Mixxx::SoundSource* pSoundSource);
    unsigned int m_NumSamples;
    unsigned int m_SampleRate;
};

#endif //CHROMAPRINTER_H
