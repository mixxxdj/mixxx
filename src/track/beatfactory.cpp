#include <QtDebug>
#include <QStringList>

#include "track/beatgrid.h"
#include "track/beatmap.h"
#include "track/beatfactory.h"
#include "track/beatutils.h"

BeatsPointer BeatFactory::loadBeatsFromByteArray(const Track& track,
                                                 QString beatsVersion,
                                                 QString beatsSubVersion,
                                                 const QByteArray& beatsSerialized) {
    if (beatsVersion == BEAT_GRID_1_VERSION ||
        beatsVersion == BEAT_GRID_2_VERSION) {
        BeatGrid* pGrid = new BeatGrid(track, 0, beatsSerialized);
        pGrid->setSubVersion(beatsSubVersion);
        qDebug() << "Successfully deserialized BeatGrid";
        return BeatsPointer(pGrid, &BeatFactory::deleteBeats);
    } else if (beatsVersion == BEAT_MAP_VERSION) {
        BeatMap* pMap = new BeatMap(track, 0, beatsSerialized);
        pMap->setSubVersion(beatsSubVersion);
        qDebug() << "Successfully deserialized BeatMap";
        return BeatsPointer(pMap, &BeatFactory::deleteBeats);
    }
    qDebug() << "BeatFactory::loadBeatsFromByteArray could not parse serialized beats.";
    return BeatsPointer();
}

BeatsPointer BeatFactory::makeBeatGrid(const Track& track, double dBpm,
                                       double dFirstBeatSample) {
    BeatGrid* pGrid = new BeatGrid(track, 0);
    pGrid->setGrid(dBpm, dFirstBeatSample);
    return BeatsPointer(pGrid, &BeatFactory::deleteBeats);
}

// static
QString BeatFactory::getPreferredVersion(const bool bEnableFixedTempoCorrection) {
    if (bEnableFixedTempoCorrection) {
        return BEAT_GRID_2_VERSION;
    }
    return BEAT_MAP_VERSION;
}

QString BeatFactory::getPreferredSubVersion(
    const bool bEnableFixedTempoCorrection,
    const bool bEnableOffsetCorrection,
    const int iMinBpm, const int iMaxBpm,
    const QHash<QString, QString> extraVersionInfo) {
    const char* kSubVersionKeyValueSeparator = "=";
    const char* kSubVersionFragmentSeparator = "|";
    QStringList fragments;

    // min/max BPM limits only apply to fixed-tempo assumption
    if (bEnableFixedTempoCorrection) {
        fragments << QString("min_bpm%1%2").arg(kSubVersionKeyValueSeparator,
                                                QString::number(iMinBpm));
        fragments << QString("max_bpm%1%2").arg(kSubVersionKeyValueSeparator,
                                                QString::number(iMaxBpm));
    }

    QHashIterator<QString, QString> it(extraVersionInfo);
    while (it.hasNext()) {
        it.next();
        if (it.key().contains(kSubVersionKeyValueSeparator) ||
            it.key().contains(kSubVersionFragmentSeparator) ||
            it.value().contains(kSubVersionKeyValueSeparator) ||
            it.value().contains(kSubVersionFragmentSeparator)) {
            qDebug() << "ERROR: Your analyzer key/value contains invalid characters:"
                     << it.key() << ":" << it.value() << "Skipping.";
            continue;
        }
        fragments << QString("%1%2%3").arg(
            it.key(), kSubVersionKeyValueSeparator, it.value());
    }
    if (bEnableFixedTempoCorrection && bEnableOffsetCorrection) {
        fragments << QString("offset_correction%1%2")
                .arg(kSubVersionKeyValueSeparator, QString::number(1));
    }

    fragments << QString("rounding%1%2").
            arg(kSubVersionKeyValueSeparator, QString::number(0.05));

    std::sort(fragments.begin(), fragments.end());
    return (fragments.size() > 0) ? fragments.join(kSubVersionFragmentSeparator) : "";
}


BeatsPointer BeatFactory::makePreferredBeats(
    const Track& track, QVector<double> beats,
    const QHash<QString, QString> extraVersionInfo,
    const bool bEnableFixedTempoCorrection, const bool bEnableOffsetCorrection,
    const int iSampleRate, const int iTotalSamples,
    const int iMinBpm, const int iMaxBpm) {
    const QString version = getPreferredVersion(bEnableFixedTempoCorrection);
    const QString subVersion = getPreferredSubVersion(bEnableFixedTempoCorrection,
                                                      bEnableOffsetCorrection,
                                                      iMinBpm, iMaxBpm,
                                                      extraVersionInfo);

    BeatUtils::printBeatStatistics(beats, iSampleRate);
    if (version == BEAT_GRID_2_VERSION) {
        double globalBpm = BeatUtils::calculateBpm(beats, iSampleRate, iMinBpm, iMaxBpm);
        double firstBeat = BeatUtils::calculateFixedTempoFirstBeat(
            bEnableOffsetCorrection,
            beats, iSampleRate, iTotalSamples, globalBpm);
        BeatGrid* pGrid = new BeatGrid(track, iSampleRate);
        // firstBeat is in frames here and setGrid() takes samples.
        pGrid->setGrid(globalBpm, firstBeat * 2);
        pGrid->setSubVersion(subVersion);
        return BeatsPointer(pGrid, &BeatFactory::deleteBeats);
    } else if (version == BEAT_MAP_VERSION) {
        BeatMap* pBeatMap = new BeatMap(track, iSampleRate, beats);
        pBeatMap->setSubVersion(subVersion);
        return BeatsPointer(pBeatMap, &BeatFactory::deleteBeats);
    } else {
        qDebug() << "ERROR: Could not determine what type of beatgrid to create.";
        return BeatsPointer();
    }
}

void BeatFactory::deleteBeats(Beats* pBeats) {
    // BeatGrid/BeatMap objects have no parent and live in the same thread as
    // their associated TIO. QObject::deleteLater does not have the desired
    // effect when the QObject's thread does not have an event loop (i.e. when
    // the main thread has already shut down) so we delete the BeatMap/BeatGrid
    // directly when its reference count drops to zero.
    delete pBeats;
}
