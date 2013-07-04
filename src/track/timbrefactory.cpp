#include <QtDebug>

#include "track/timbre.h"
#include "track/timbrefactory.h"

using mixxx::track::io::timbre::TimbreModel;
using mixxx::track::io::timbre::BeatSpectrum;

TimbrePointer TimbreFactory::loadTimbreFromByteArray(TrackPointer pTrack,
                                       QString timbreVersion,
                                       QString timbreSubVersion,
                                       QByteArray* timbreSerialized) {
    if (timbreVersion == TIMBRE_MODEL_VERSION) {
        Timbre* pTimbre = new Timbre(timbreSerialized);
        pTimbre->moveToThread(pTrack->thread());
        pTimbre->setParent(pTrack.data());
        pTimbre->setSubVersion(timbreSubVersion);
        qDebug() << "Successfully deserialized TimbreModel";
        return TimbrePointer(pTimbre, &TimbreFactory::deleteTimbre);
    }
    qDebug() << "TimbreFactory::loadTimbreFromByteArray could not parse serialized timbre model.";
    return TimbrePointer();
}

TimbrePointer TimbreFactory::makeTimbreModel(std::vector<double> mean,
                                      std::vector<double> variance,
                                      std::vector<double> beatSpectrum) {
    TimbreModel timbre_model;
    BeatSpectrum* beat_spectrum = timbre_model.mutable_beat_spectrum();
    for (std::vector<double>::iterator it = mean.begin(); it != mean.end(); ++it) {
        timbre_model.add_mean(*it);
    }
    for (std::vector<double>::iterator it = variance.begin(); it != variance.end(); ++it) {
        timbre_model.add_variance(*it);
    }
    for (std::vector<double>::iterator it = beatSpectrum.begin(); it != beatSpectrum.end(); ++it) {
        beat_spectrum->add_feature(*it);
    }
    Timbre* pTimbre = new Timbre(timbre_model);
    return TimbrePointer(pTimbre, &TimbreFactory::deleteTimbre);
}
TimbrePointer TimbreFactory::makeTimbreModelFromVamp(QVector<double> timbreVector) {
    TimbreModel timbre_model;
    BeatSpectrum* beat_spectrum = timbre_model.mutable_beat_spectrum();

    int meanIndex = timbreVector[0] + 3;
    int varianceIndex = timbreVector[1] + meanIndex;
    int beatSpectrumIndex = timbreVector[2] + varianceIndex;

    int i = 3, n = timbreVector.size();

    for(; i < n; i++) {
        double val = timbreVector[i];
        if (i < meanIndex)
            timbre_model.add_mean(val);
        else if (i < varianceIndex)
            timbre_model.add_variance(val);
        else if (i < beatSpectrumIndex)
            beat_spectrum->add_feature(val);
    }
    Timbre* pTimbre = new Timbre(timbre_model);
    return TimbrePointer(pTimbre, &TimbreFactory::deleteTimbre);
}

void TimbreFactory::deleteTimbre(Timbre* pTimbre) {
    QObject* pObject = dynamic_cast<QObject*>(pTimbre);
    if (pObject != NULL) {
        pTimbre->deleteLater();
    }
}
