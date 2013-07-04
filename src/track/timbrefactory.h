#ifndef TIMBREFACTORY_H
#define TIMBREFACTORY_H

#include <QHash>

#include "track/timbre.h"
#include "proto/timbre.pb.h"
#include "trackinfoobject.h"

class TimbreFactory {
  public:
    static Timbre loadTimbreFromByteArray(TrackPointer pTrack,
                                               QString timbreVersion,
                                               QString timbreSubVersion,
                                               QByteArray* timbreSerialized);
    static QString getPreferredVersion();

    static QString getPreferredSubVersion(
        const QHash<QString, QString> extraVersionInfo);

};

#endif // TIMBREFACTORY_H
