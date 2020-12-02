#ifndef CHROMAPRINTER_H
#define CHROMAPRINTER_H

#include <stddef.h>

#include <QByteArrayData>
#include <QObject>
#include <QString>

#include "track/track_decl.h"

class ChromaPrinter: public QObject {
  Q_OBJECT

public:
      explicit ChromaPrinter(QObject* parent = NULL);
      QString getFingerprint(TrackPointer pTrack);
};

#endif //CHROMAPRINTER_H
