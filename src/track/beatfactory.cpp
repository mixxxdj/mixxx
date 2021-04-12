#include <QtDebug>
#include <QStringList>

#include "track/beatgrid.h"
#include "track/beatmap.h"
#include "track/beatfactory.h"
#include "track/beatutils.h"

namespace {

const QString kRoundingVersion = QStringLiteral("V2");

} // namespace

mixxx::BeatsPointer BeatFactory::loadBeatsFromByteArray(
        mixxx::audio::SampleRate sampleRate,
        const QString& beatsVersion,
        const QString& beatsSubVersion,
        const QByteArray& beatsSerialized) {
    if (beatsVersion == BEAT_GRID_1_VERSION ||
        beatsVersion == BEAT_GRID_2_VERSION) {
        auto pGrid = mixxx::BeatGrid::makeBeatGrid(sampleRate, beatsSubVersion, beatsSerialized);
        qDebug() << "Successfully deserialized BeatGrid";
        return pGrid;
    } else if (beatsVersion == BEAT_MAP_VERSION) {
        auto pMap = mixxx::BeatMap::makeBeatMap(sampleRate, beatsSubVersion, beatsSerialized);
        qDebug() << "Successfully deserialized BeatMap";
        return pMap;
    }
    qDebug() << "BeatFactory::loadBeatsFromByteArray could not parse serialized beats.";
    return mixxx::BeatsPointer();
}

mixxx::BeatsPointer BeatFactory::makeBeatGrid(
        mixxx::audio::SampleRate sampleRate,
        double dBpm,
        double dFirstBeatSample) {
    return mixxx::BeatGrid::makeBeatGrid(sampleRate, QString(), dBpm, dFirstBeatSample);
}

// static
QString BeatFactory::getPreferredVersion(bool fixedTempo) {
    if (fixedTempo) {
        return BEAT_GRID_2_VERSION;
    }
    return BEAT_MAP_VERSION;
}

QString BeatFactory::getPreferredSubVersion(
        const QHash<QString, QString>& extraVersionInfo) {
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
            qDebug() << "ERROR: Your analyzer key/value contains invalid "
                        "characters:"
                     << it.key() << ":" << it.value() << "Skipping.";
            continue;
        }
        fragments << QString("%1%2%3").arg(
                it.key(), kSubVersionKeyValueSeparator, it.value());
    }

    fragments << QString("rounding%1%2")
                         .arg(kSubVersionKeyValueSeparator, kRoundingVersion);

    std::sort(fragments.begin(), fragments.end());
    return (fragments.size() > 0) ? fragments.join(kSubVersionFragmentSeparator)
                                  : "";
}

mixxx::BeatsPointer BeatFactory::makePreferredBeats(
        const QVector<double>& beats,
        const QHash<QString, QString>& extraVersionInfo,
        bool fixedTempo,
        mixxx::audio::SampleRate sampleRate) {
    const QString version = getPreferredVersion(fixedTempo);
    const QString subVersion = getPreferredSubVersion(extraVersionInfo);

    QVector<BeatUtils::ConstRegion> constantRegions =
            BeatUtils::retrieveConstRegions(beats, sampleRate);

    if (version == BEAT_GRID_2_VERSION) {
        double firstBeat = 0;
        double constBPM = BeatUtils::makeConstBpm(constantRegions, sampleRate, &firstBeat);
        firstBeat = BeatUtils::adjustPhase(firstBeat, constBPM, sampleRate, beats);
        auto pGrid = mixxx::BeatGrid::makeBeatGrid(
                sampleRate, subVersion, constBPM, firstBeat * 2);
        return pGrid;
    } else if (version == BEAT_MAP_VERSION) {
        QVector<double> ironedBeats = BeatUtils::getBeats(constantRegions);
        auto pBeatMap = mixxx::BeatMap::makeBeatMap(sampleRate, subVersion, ironedBeats);
        return pBeatMap;
    } else {
        qDebug() << "ERROR: Could not determine what type of beatgrid to create.";
        return mixxx::BeatsPointer();
    }
}
