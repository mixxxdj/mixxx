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
    // Now that the serialized representation is the same for BeatGrids and BeatMaps,
    // they can be deserialized in a common function.
    if (beatsVersion == mixxx::BeatsInternal::BEAT_GRID_1_VERSION ||
            beatsVersion == mixxx::BeatsInternal::BEAT_GRID_2_VERSION ||
            beatsVersion == mixxx::BeatsInternal::BEAT_MAP_VERSION) {
        mixxx::Beats* pMap = new mixxx::Beats(track.get(), beatsSerialized);
        pMap->setSubVersion(beatsSubVersion);
        qDebug() << "Successfully deserialized Beats";
        return mixxx::BeatsPointer(pMap, &BeatFactory::deleteBeats);
    }
    qDebug() << "BeatFactory::loadBeatsFromByteArray could not parse serialized beats.";
    // TODO(JVC) May be launching a reanalysis to fix the data?
    return mixxx::BeatsPointer();
}

// static
QString BeatFactory::getPreferredVersion(
        const bool bEnableFixedTempoCorrection) {
    if (bEnableFixedTempoCorrection) {
        return mixxx::BeatsInternal::BEAT_GRID_2_VERSION;
    }
    return mixxx::BeatsInternal::BEAT_MAP_VERSION;
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
        const int iTotalSamples,
        const int iMinBpm,
        const int iMaxBpm) {
    const int iSampleRate = track->getSampleRate();
    const QString version = getPreferredVersion(bEnableFixedTempoCorrection);
    const QString subVersion = getPreferredSubVersion(bEnableFixedTempoCorrection,
                                                      bEnableOffsetCorrection,
                                                      iMinBpm, iMaxBpm,
                                                      extraVersionInfo);

    BeatUtils::printBeatStatistics(beats, iSampleRate);
    if (version == mixxx::BeatsInternal::BEAT_GRID_2_VERSION) {
        mixxx::Bpm globalBpm = BeatUtils::calculateBpm(beats, iSampleRate, iMinBpm, iMaxBpm);
        mixxx::FramePos firstBeat(BeatUtils::calculateFixedTempoFirstBeat(
                bEnableOffsetCorrection,
                beats,
                iSampleRate,
                iTotalSamples,
                globalBpm.getValue()));
        mixxx::FrameDiff_t beatLength = iSampleRate * 60 / globalBpm.getValue();
        double trackLengthSeconds = track->getDuration();
        int numberOfBeats = globalBpm.getValue() / 60.0 * trackLengthSeconds;
        QVector<mixxx::FramePos> generatedBeats;
        for (int i = 0; i < numberOfBeats; i++) {
            generatedBeats.append(firstBeat + beatLength * i);
        }
        mixxx::Beats* pBeats = new mixxx::Beats(
                track.get(), generatedBeats);
        pBeats->setSubVersion(subVersion);
        return mixxx::BeatsPointer(pBeats, &BeatFactory::deleteBeats);
    } else if (version == mixxx::BeatsInternal::BEAT_MAP_VERSION) {
        QVector<mixxx::FramePos> intermediateBeatFrameVector;
        intermediateBeatFrameVector.reserve(beats.size());
        std::transform(beats.begin(),
                beats.end(),
                std::back_inserter(intermediateBeatFrameVector),
                [](double value) { return mixxx::FramePos(value); });
        mixxx::Beats* pBeats = new mixxx::Beats(
                track.get(), intermediateBeatFrameVector);
        pBeats->setSubVersion(subVersion);
        return mixxx::BeatsPointer(pBeats, &BeatFactory::deleteBeats);
    } else {
        DEBUG_ASSERT("Could not determine what type of beatgrid to create.");
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
