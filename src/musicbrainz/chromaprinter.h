#ifndef CHROMAPRINTER_H
#define CHROMAPRINTER_H

#include <QObject>

#include "trackinfoobject.h"

class ChromaPrinter: public QObject {
  Q_OBJECT

public:
      explicit ChromaPrinter(QObject* parent = NULL);
      QString getFingerprint(TrackPointer pTrack);
};

#endif //CHROMAPRINTER_H
