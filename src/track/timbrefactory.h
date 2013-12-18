#ifndef TIMBREFACTORY_H
#define TIMBREFACTORY_H

#include <QHash>

#include "track/timbre.h"
#include "proto/timbre.pb.h"
#include "trackinfoobject.h"

class TimbreFactory {
  public:
    static TimbrePointer loadTimbreFromByteArray(TrackPointer pTrack,
        QString timbreVersion, QString timbreSubVersion,
        QByteArray* timbreSerialized);
    static TimbrePointer makeTimbreModel(std::vector<double> mean,
        std::vector<double> variance, std::vector<double> beatSpectrum);
    static TimbrePointer makeTimbreModelFromVamp(QVector<double> timbreVector);
    static QString getPreferredVersion();
    static QString getPreferredSubVersion(
        const QHash<QString, QString> extraVersionInfo);
    static TimbrePointer makePreferredTimbreModel(TrackPointer pTrack,
        QVector<double> timbreVector, const QHash<QString, QString> extraVersionInfo,
        const int iSampleRate, const int iTotalSamples);

  private:
    static void deleteTimbre(Timbre* pTimbre);
};
#endif // TIMBREFACTORY_H
