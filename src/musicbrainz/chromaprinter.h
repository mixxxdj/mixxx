#pragma once

#include <QObject>

#include "track/track_decl.h"

class ChromaPrinter: public QObject {
  Q_OBJECT

public:
      explicit ChromaPrinter(QObject* parent = NULL);
      QString getFingerprint(TrackPointer pTrack);
};
