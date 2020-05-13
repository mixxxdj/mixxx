#include "track/beatfactory.h"

#include <QStringList>
#include <QtDebug>

#include "track/beats.h"
#include "track/beatutils.h"
#include "track/track.h"

mixxx::BeatsPointer BeatFactory::loadBeatsFromByteArray(const TrackPointer& track,
        QString beatsVersion,
        QString beatsSubVersion,
        const QByteArray& beatsSerialized) {
    mixxx::Beats* pMap = new mixxx::Beats(track.get(), beatsSerialized);
    pMap->setSubVersion(beatsSubVersion);
    qDebug() << "Successfully deserialized Beats";
    if (beatsVersion == BEAT_GRID_1_VERSION ||
        beatsVersion == BEAT_GRID_2_VERSION) {
        pMap->setIsTempoConst(true);
    } else if (beatsVersion == BEAT_MAP_VERSION) {
        pMap->setIsTempoConst(false);      
    }
    else {
        qDebug() << "BeatFactory::loadBeatsFromByteArray could not parse serialized beats.";
        // TODO(JVC) May be launching a reanalisys to fix the data?
        return mixxx::BeatsPointer();
    }
    return mixxx::BeatsPointer(pMap, &BeatFactory::deleteBeats);
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
        const QHash<QString, QString> extraVersionInfo) {
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

mixxx::BeatsPointer BeatFactory::makePreferredBeats(const TrackPointer& track,
        QVector<double> beats,
        const QHash<QString, QString> extraVersionInfo,
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
    double globalBpm = BeatUtils::calculateBpm(beats, iSampleRate, iMinBpm, iMaxBpm);
    mixxx::Beats* pBeats = new mixxx::Beats(track.get(), beats, iSampleRate);
    pBeats->setGlobalBpm(globalBpm);
    pBeats->setSubVersion(subVersion);
    if (version == BEAT_GRID_2_VERSION) {
        pBeats->setIsTempoConst(true);
    } else if (version == BEAT_MAP_VERSION) {
        pBeats->setIsTempoConst(false);
        DEBUG_ASSERT(version == BEAT_MAP_VERSION);
    } else {
        DEBUG_ASSERT("Could not determine what type of beatgrid to create.");
        return mixxx::BeatsPointer();
    }
    return mixxx::BeatsPointer(pBeats, &BeatFactory::deleteBeats);
}

void BeatFactory::deleteBeats(mixxx::Beats* pBeats) {
    // BeatGrid/BeatMap objects have no parent and live in the same thread as
    // their associated TIO. QObject::deleteLater does not have the desired
    // effect when the QObject's thread does not have an event loop (i.e. when
    // the main thread has already shut down) so we delete the BeatMap/BeatGrid
    // directly when its reference count drops to zero.
    delete pBeats;
}
