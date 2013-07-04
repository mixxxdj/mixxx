#include <QtDebug>

#include "track/timbre.h"
#include "track/timbrefactory.h"

using mixxx::track::io::timbre::TimbreModel;
using mixxx::track::io::timbre::BeatSpectrum;

Timbre TimbreFactory::loadTimbreFromByteArray(TrackPointer pTrack,
                                       QString timbreVersion,
                                       QString timbreSubVersion,
                                       QByteArray* timbreSerialized) {
    Q_UNUSED(pTrack);
    if (timbreVersion == TIMBRE_MODEL_VERSION) {
        Timbre timbre(timbreSerialized);
        timbre.setSubVersion(timbreSubVersion);
        qDebug() << "Successfully deserialized TimbreModel";
        return timbre;
    }

    return Timbre();
}

Timbre TimbreFactory::makeTimbreModel(std::vector<double> mean,
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
    return Timbre(timbre_model);
}
Timbre TimbreFactory::makeTimbreModelFromVamp(QVector<double> timbreVector) {
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
    return Timbre(timbre_model);
}

