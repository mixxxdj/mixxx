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

    static Timbre makeTimbreModel(std::vector<double> mean,
                                  std::vector<double> variance,
                                  std::vector<double> beatSpectrum);

    static Timbre makeTimbreModelFromVamp(QVector<double> timbreVector);

    static QString getPreferredVersion();

    static QString getPreferredSubVersion(
        const QHash<QString, QString> extraVersionInfo);

};

#endif // TIMBREFACTORY_H
