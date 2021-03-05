#include <QtDebug>
#include <QStringList>

#include "track/beatgrid.h"
#include "track/beatmap.h"
#include "track/beatfactory.h"
#include "track/beatutils.h"

mixxx::BeatsPointer BeatFactory::loadBeatsFromByteArray(
        SINT sampleRate,
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
        SINT sampleRate, double dBpm, double dFirstBeatSample) {
    return mixxx::BeatGrid::makeBeatGrid(sampleRate, QString(), dBpm, dFirstBeatSample);
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
        const bool bEnableFixedTempoCorrection,
        const bool bEnableOffsetCorrection,
        const int iMinBpm,
        const int iMaxBpm,
        const QHash<QString, QString>& extraVersionInfo) {
    const char* kSubVersionKeyValueSeparator = "=";
    const char* kSubVersionFragmentSeparator = "|";
    QStringList fragments;

    // min/max BPM limits only apply to fixed-tempo assumption
    if (bEnableFixedTempoCorrection) {
        fragments << QString("min_bpm%1%2")
                             .arg(kSubVersionKeyValueSeparator,
                                     QString::number(iMinBpm));
        fragments << QString("max_bpm%1%2")
                             .arg(kSubVersionKeyValueSeparator,
                                     QString::number(iMaxBpm));
    }

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
    if (bEnableFixedTempoCorrection && bEnableOffsetCorrection) {
        fragments << QString("offset_correction%1%2")
                             .arg(kSubVersionKeyValueSeparator,
                                     QString::number(1));
    }

    fragments << QString("rounding%1%2")
                         .arg(kSubVersionKeyValueSeparator,
                                 QString::number(0.05));

    std::sort(fragments.begin(), fragments.end());
    return (fragments.size() > 0) ? fragments.join(kSubVersionFragmentSeparator)
                                  : "";
}

mixxx::BeatsPointer BeatFactory::makePreferredBeats(
        const QVector<double>& beats,
        const QHash<QString, QString>& extraVersionInfo,
        const bool bEnableFixedTempoCorrection,
        const bool bEnableOffsetCorrection,
        const int iSampleRate,
        const int iTotalSamples,
        const int iMinBpm,
        const int iMaxBpm) {
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
        auto pGrid = mixxx::BeatGrid::makeBeatGrid(
                iSampleRate, subVersion, globalBpm, firstBeat * 2);
        return pGrid;
    } else if (version == BEAT_MAP_VERSION) {
        auto pBeatMap = mixxx::BeatMap::makeBeatMap(iSampleRate, subVersion, beats);
        return pBeatMap;
    } else {
        qDebug() << "ERROR: Could not determine what type of beatgrid to create.";
        return mixxx::BeatsPointer();
    }
}
