#include <QtDebug>
#include <QStringList>

#include "track/beatgrid.h"
#include "track/beatmap.h"
#include "track/beatfactory.h"
#include "track/beatutils.h"

BeatsPointer BeatFactory::loadBeatsFromByteArray(TrackPointer pTrack,
                                                 QString beatsVersion,
                                                 QString beatsSubVersion,
                                                 QByteArray* beatsSerialized) {

    if (beatsVersion == BEAT_GRID_1_VERSION ||
        beatsVersion == BEAT_GRID_2_VERSION) {
        BeatGrid* pGrid = new BeatGrid(pTrack.data(), beatsSerialized);
        pGrid->moveToThread(pTrack->thread());
        pGrid->setParent(pTrack.data());
        pGrid->setSubVersion(beatsSubVersion);
        qDebug() << "Successfully deserialized BeatGrid";
        return BeatsPointer(pGrid, &BeatFactory::deleteBeats);
    } else if (beatsVersion == BEAT_MAP_VERSION) {
        BeatMap* pMap = new BeatMap(pTrack, beatsSerialized);
        pMap->moveToThread(pTrack->thread());
        pMap->setParent(pTrack.data());
        pMap->setSubVersion(beatsSubVersion);
        qDebug() << "Successfully deserialized BeatMap";
        return BeatsPointer(pMap, &BeatFactory::deleteBeats);
    }
    qDebug() << "BeatFactory::loadBeatsFromByteArray could not parse serialized beats.";
    return BeatsPointer();
}

BeatsPointer BeatFactory::makeBeatGrid(TrackInfoObject* pTrack, double dBpm, double dFirstBeatSample) {
    BeatGrid* pGrid = new BeatGrid(pTrack);
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

const char* kSubVersionKeyValueSeparator = "=";
const char* kSubVersionFragmentSeparator = "|";
QString BeatFactory::getPreferredSubVersion(
    const bool bEnableFixedTempoCorrection,
    const bool bEnableOffsetCorrection,
    const int iMinBpm, const int iMaxBpm,
    const QHash<QString, QString> extraVersionInfo) {
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
            qDebug() << "ERROR: Your analyser key/value contains invalid characters:"
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

    qSort(fragments);
    return (fragments.size() > 0) ? fragments.join(kSubVersionFragmentSeparator) : "";
}


BeatsPointer BeatFactory::makePreferredBeats(
    TrackPointer pTrack, QVector<double> beats,
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

        BeatGrid* pGrid = new BeatGrid(pTrack.data());
        // firstBeat is in frames here and setGrid() takes samples.
        pGrid->setGrid(globalBpm, firstBeat * 2);
        pGrid->setSubVersion(subVersion);
        return BeatsPointer(pGrid, &BeatFactory::deleteBeats);
    } else if (version == BEAT_MAP_VERSION) {
        BeatMap* pBeatMap = new BeatMap(pTrack, beats);
        pBeatMap->setSubVersion(subVersion);
        return BeatsPointer(pBeatMap, &BeatFactory::deleteBeats);
    } else {
        qDebug() << "ERROR: Could not determine what type of beatgrid to create.";
        return BeatsPointer();
    }
}

void BeatFactory::deleteBeats(Beats* pBeats) {
    // This assumes all Beats* variants multiply-inherit from QObject. Kind of
    // ugly. Oh well.
    QObject* pObject = dynamic_cast<QObject*>(pBeats);

    if (pObject != NULL) {
        pObject->deleteLater();
    }
}
