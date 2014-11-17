#ifndef CHROMAPRINTER_H
#define CHROMAPRINTER_H

#include <QObject>

#include "soundsource.h"
#include "trackinfoobject.h"

class ChromaPrinter: public QObject {
  Q_OBJECT

  public:
    ChromaPrinter(QObject* parent=NULL);
    QString getFingerPrint(TrackPointer pTrack);

  private:

    QString calcFingerPrint(const Mixxx::SoundSourcePointer& pSoundSource);
};

#endif //CHROMAPRINTER_H
