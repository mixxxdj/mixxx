#include <QtDebug>
#include <QStringList>

#include "track/beatgrid.h"
#include "track/beatmap.h"
#include "track/beatfactory.h"
#include "track/beatutils.h"

namespace {

const QString kRoundingVersion = QStringLiteral("V2");

} // namespace

mixxx::BeatsPointer BeatFactory::loadBeatsFromByteArray(const Track& track,
        const QString& beatsVersion,
        const QString& beatsSubVersion,
        const QByteArray& beatsSerialized) {
    if (beatsVersion == BEAT_GRID_1_VERSION ||
        beatsVersion == BEAT_GRID_2_VERSION) {
        mixxx::BeatGrid* pGrid = new mixxx::BeatGrid(track, 0, beatsSerialized);
        pGrid->setSubVersion(beatsSubVersion);
        qDebug() << "Successfully deserialized BeatGrid";
        return mixxx::BeatsPointer(pGrid, &BeatFactory::deleteBeats);
    } else if (beatsVersion == BEAT_MAP_VERSION) {
        mixxx::BeatMap* pMap = new mixxx::BeatMap(track, 0, beatsSerialized);
        pMap->setSubVersion(beatsSubVersion);
        qDebug() << "Successfully deserialized BeatMap";
        return mixxx::BeatsPointer(pMap, &BeatFactory::deleteBeats);
    }
    qDebug() << "BeatFactory::loadBeatsFromByteArray could not parse serialized beats.";
    return mixxx::BeatsPointer();
}

mixxx::BeatsPointer BeatFactory::makeBeatGrid(
        const Track& track, double dBpm, double dFirstBeatSample) {
    mixxx::BeatGrid* pGrid = new mixxx::BeatGrid(track, 0);
    pGrid->setGrid(dBpm, dFirstBeatSample);
    return mixxx::BeatsPointer(pGrid, &BeatFactory::deleteBeats);
}

// static
QString BeatFactory::getPreferredVersion(
        const bool bEnableFixedTempoCorrection) {
    if (bEnableFixedTempoCorrection) {
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

mixxx::BeatsPointer BeatFactory::makePreferredBeats(const Track& track,
        const QVector<double>& beats,
        const QHash<QString, QString>& extraVersionInfo,
        bool bEnableFixedTempoCorrection,
        const mixxx::audio::SampleRate& sampleRate) {
    const QString version = getPreferredVersion(bEnableFixedTempoCorrection);
    const QString subVersion = getPreferredSubVersion(extraVersionInfo);

    QVector<BeatUtils::ConstRegion> constantRegions =
            BeatUtils::retrieveConstRegions(beats, sampleRate);

    if (version == BEAT_GRID_2_VERSION) {
        double firstBeat = 0;
        double constBPM = BeatUtils::makeConstBpm(constantRegions, sampleRate, &firstBeat);
        qDebug() << ":-) " << constBPM << "(-:";
        firstBeat = BeatUtils::adjustPhase(firstBeat, constBPM, sampleRate, beats);
        mixxx::BeatGrid* pGrid = new mixxx::BeatGrid(track, sampleRate);
        // firstBeat is in frames here and setGrid() takes samples.
        pGrid->setGrid(constBPM, firstBeat * 2);
        pGrid->setSubVersion(subVersion);
        return mixxx::BeatsPointer(pGrid, &BeatFactory::deleteBeats);
    } else if (version == BEAT_MAP_VERSION) {
        QVector<double> ironedBeats = BeatUtils::getBeats(constantRegions);
        mixxx::BeatMap* pBeatMap = new mixxx::BeatMap(track, sampleRate, ironedBeats);
        pBeatMap->setSubVersion(subVersion);
        return mixxx::BeatsPointer(pBeatMap, &BeatFactory::deleteBeats);
    } else {
        qDebug() << "ERROR: Could not determine what type of beatgrid to create.";
        return mixxx::BeatsPointer();
    }
}

void BeatFactory::deleteBeats(mixxx::Beats* pBeats) {
    // BeatGrid/BeatMap objects have no parent and live in the same thread as
    // their associated TIO. QObject::deleteLater does not have the desired
    // effect when the QObject's thread does not have an event loop (i.e. when
    // the main thread has already shut down) so we delete the BeatMap/BeatGrid
    // directly when its reference count drops to zero.
    delete pBeats;
}
