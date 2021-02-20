#include "track/beatfactory.h"

#include <QStringList>
#include <QtDebug>

#include "track/beats.h"
#include "track/beatutils.h"
#include "track/track.h"

mixxx::BeatsInternal BeatFactory::loadBeatsFromByteArray(const TrackPointer& track,
        const QString& beatsVersion,
        const QString& beatsSubVersion,
        const QByteArray& beatsSerialized) {
    auto deserializedBeats = mixxx::BeatsInternal(track->streamInfo());
    deserializedBeats.setSubVersion(beatsSubVersion);
    // Now that the serialized representation is the same for BeatGrids and BeatMaps,
    // they can be deserialized in a common function.
    if (beatsVersion == mixxx::BeatsInternal::kBeatGridVersion1 ||
            beatsVersion == mixxx::BeatsInternal::kBeatGridVersion2) {
        mixxx::track::io::LegacyBeatGrid legacyBeatGridProto;
        if (!legacyBeatGridProto.ParseFromArray(
                    beatsSerialized.constData(), beatsSerialized.size())) {
            qDebug()
                    << "ERROR: Could not parse legacy" << beatsVersion << "from QByteArray of size"
                    << beatsSerialized.size();
            return deserializedBeats;
        }
        deserializedBeats.setGrid(mixxx::Bpm(legacyBeatGridProto.bpm().bpm()),
                mixxx::FramePos(
                        legacyBeatGridProto.first_beat().frame_position()));
        qDebug() << "Successfully deserialized Beats from legacy data in format" << beatsVersion;
        return deserializedBeats;
    } else if (beatsVersion == mixxx::BeatsInternal::kBeatMapVersion) {
        mixxx::track::io::LegacyBeatMap legacyBeatMapProto;
        if (!legacyBeatMapProto.ParseFromArray(
                    beatsSerialized.constData(), beatsSerialized.size())) {
            qDebug() << "ERROR: Could not parse legacy" << beatsVersion << "from QByteArray of size"
                     << beatsSerialized.size();
            return deserializedBeats;
        }
        // Generate intermediate data from the old serialized representation.
        QVector<mixxx::FramePos> beatVector;
        for (int i = 0; i < legacyBeatMapProto.beat_size(); ++i) {
            const mixxx::track::io::LegacyBeat& beat = legacyBeatMapProto.beat(i);
            beatVector.append(mixxx::FramePos(beat.frame_position()));
        }
        deserializedBeats.initWithAnalyzer(beatVector);
        qDebug() << "Successfully deserialized Beats from legacy data in format" << beatsVersion;
        return deserializedBeats;
    } else if (beatsVersion == mixxx::BeatsInternal::kBeatsVersion) {
        deserializedBeats.initWithProtobuf(beatsSerialized);
        qDebug() << "Successfully deserialized Beats";
        return deserializedBeats;
    }
    qDebug() << "BeatFactory::loadBeatsFromByteArray could not parse serialized beats.";
    return deserializedBeats;
}

// static
QString BeatFactory::getPreferredVersion(
        const bool bEnableFixedTempoCorrection) {
    if (bEnableFixedTempoCorrection) {
        return mixxx::BeatsInternal::kBeatGridVersion2;
    }
    return mixxx::BeatsInternal::kBeatMapVersion;
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

mixxx::BeatsInternal BeatFactory::makePreferredBeats(const TrackPointer& track,
        const QVector<double>& beats,
        const QHash<QString, QString>& extraVersionInfo,
        const bool bEnableFixedTempoCorrection,
        const bool bEnableOffsetCorrection,
        const int iTotalSamples,
        const int iMinBpm,
        const int iMaxBpm) {
    const int iSampleRate = track->getSampleRate();
    mixxx::BeatsInternal beatsInternal(track->streamInfo());

    const QString version = getPreferredVersion(bEnableFixedTempoCorrection);
    const QString subVersion = getPreferredSubVersion(bEnableFixedTempoCorrection,
                                                      bEnableOffsetCorrection,
                                                      iMinBpm, iMaxBpm,
                                                      extraVersionInfo);
    beatsInternal.setSubVersion(subVersion);

    BeatUtils::printBeatStatistics(beats, iSampleRate);
    if (version == mixxx::BeatsInternal::kBeatGridVersion2) {
        mixxx::Bpm globalBpm = BeatUtils::calculateBpm(beats, iSampleRate, iMinBpm, iMaxBpm);
        mixxx::FramePos firstBeat(BeatUtils::calculateFixedTempoFirstBeat(
                bEnableOffsetCorrection,
                beats,
                iSampleRate,
                iTotalSamples,
                globalBpm.getValue()));
        mixxx::FrameDiff_t beatLength = iSampleRate * 60 / globalBpm.getValue();
        double trackLengthSeconds = track->getDuration();
        int numberOfBeats = static_cast<int>(globalBpm.getValue() / 60.0 * trackLengthSeconds);
        QVector<mixxx::FramePos> generatedBeats;
        for (int i = 0; i < numberOfBeats; i++) {
            generatedBeats.append(firstBeat + beatLength * i);
        }
        beatsInternal.initWithAnalyzer(generatedBeats);
        return beatsInternal;
    } else if (version == mixxx::BeatsInternal::kBeatMapVersion) {
        QVector<mixxx::FramePos> intermediateBeatFrameVector;
        intermediateBeatFrameVector.reserve(beats.size());
        std::transform(beats.begin(),
                beats.end(),
                std::back_inserter(intermediateBeatFrameVector),
                [](double value) { return mixxx::FramePos(value); });
        beatsInternal.initWithAnalyzer(intermediateBeatFrameVector);
        return beatsInternal;
    } else {
        DEBUG_ASSERT("Could not determine what type of beatgrid to create.");
        return mixxx::BeatsInternal();
    }
}
