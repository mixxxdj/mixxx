#include <QtDebug>
#include <QStringList>

#include "track/timbre.h"
#include "track/timbrefactory.h"

using mixxx::track::io::timbre::TimbreModel;
using mixxx::track::io::timbre::BeatSpectrum;

TimbrePointer TimbreFactory::loadTimbreFromByteArray(TrackPointer pTrack,
        QString timbreVersion,QString timbreSubVersion,
        QByteArray* timbreSerialized) {
    if (timbreVersion == TIMBRE_MODEL_VERSION) {
        Timbre* pTimbre = new Timbre(timbreSerialized);
        pTimbre->moveToThread(pTrack->thread());
        pTimbre->setParent(pTrack.data());
        pTimbre->setSubVersion(timbreSubVersion);
        qDebug() << "Successfully deserialized TimbreModel";
        return TimbrePointer(pTimbre, &TimbreFactory::deleteTimbre);
    }
    qDebug() << "TimbreFactory::loadTimbreFromByteArray could not parse"
                "serialized timbre model.";
    return TimbrePointer();
}

TimbrePointer TimbreFactory::makeTimbreModel(std::vector<double> mean,
                                             std::vector<double> variance,
                                             std::vector<double> beatSpectrum) {
    TimbreModel timbre_model;
    BeatSpectrum* beat_spectrum = timbre_model.mutable_beat_spectrum();
    // TODO (kain88) use foreach
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

    for(int i = 3; i < timbreVector.size(); i++) {
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

//static
QString TimbreFactory::getPreferredVersion() {
    return TIMBRE_MODEL_VERSION;
}

QString TimbreFactory::getPreferredSubVersion(
    const QHash<QString, QString> extraVersionInfo) {
    const char* kSubVersionKeyValueSeparator = "=";
    const char* kSubVersionFragmentSeparator = "|";
    QStringList fragments;

    QHashIterator<QString, QString> it(extraVersionInfo);
    while (it.hasNext()) {
        it.next();
        if (it.key().contains(kSubVersionKeyValueSeparator) ||
            it.key().contains(kSubVersionFragmentSeparator) ||
            it.value().contains(kSubVersionKeyValueSeparator) ||
            it.value().contains(kSubVersionFragmentSeparator)) {
            qDebug() << "ERROR: Your analyser key/value contains invalid characters:"
                     << it.key() << ":" << it.value() << "Skipping.";
            continue;
        }
        fragments << QString("%1%2%3").arg(
            it.key(), kSubVersionKeyValueSeparator, it.value());
    }

    qSort(fragments);
    return (fragments.size() > 0) ? fragments.join(kSubVersionFragmentSeparator) : "";
}

TimbrePointer TimbreFactory::makePreferredTimbreModel(TrackPointer pTrack,
        QVector<double> timbreVector, const QHash<QString, QString> extraVersionInfo,
        const int iSampleRate, const int iTotalSamples) {
    Q_UNUSED(pTrack);
    Q_UNUSED(iSampleRate);
    const QString version = getPreferredVersion();
    const QString subVersion = getPreferredSubVersion(extraVersionInfo);

    if (version == TIMBRE_MODEL_VERSION) {
        TimbrePointer pTimbre = makeTimbreModelFromVamp(timbreVector);
        pTimbre->setSubVersion(subVersion);
        return pTimbre;
    }
    qDebug() << "ERROR: Could not determine what type of TimbreModel to create.";
    return TimbrePointer();
}

void TimbreFactory::deleteTimbre(Timbre* pTimbre) {
    QObject* pObject = dynamic_cast<QObject*>(pTimbre);
    if (pObject != NULL) {
        pTimbre->deleteLater();
    }
}
