#include <QFile>
#include <QtDebug>
#include <QStringList>

#include "track/beatgrid.h"
#include "track/beatmap.h"
#include "track/beatfactory.h"
#include "track/beatutils.h"
#include "util/cmdlineargs.h"


namespace {

void debugBeats(const Track& track, const QVector<double>& rawBeats,
        const QVector<double>& correctedBeats, QString beatsVersion, QString beatsSubVersion) {
    if(!CmdlineArgs::Instance().getAnalyzerDebug()) {
        return;
    }
    QString debugFilename = QDir(CmdlineArgs::Instance().getSettingsPath()).filePath("beatAnalyzerOutput.csv");
    QFile debugFile(debugFilename);
    if (!debugFile.open(QIODevice::Append | QIODevice::Text)) {
        qWarning() << "ERROR: Could not open debug file:" << debugFilename;
        return;
    }
    QString trackHeader;
    trackHeader += track.getInfo();
    trackHeader += ", analysed at ";
    trackHeader += QDateTime::currentDateTime().toString("yyyy-MM-dd_hh'h'mm'm'ss's'");
    trackHeader += ", with " + beatsVersion + beatsSubVersion;
    debugFile.write(trackHeader.toLocal8Bit());
    QString sRawBeats;
    QString sRawBeatLenght;
    auto previousBeat = rawBeats.begin();
    sRawBeats += QString::number(*previousBeat, 'f') + ",";
    for (auto beat = std::begin(rawBeats) + 1, end = std::end(rawBeats); beat != end; beat += 1) {
        sRawBeats += QString::number(*beat, 'f') + ",";
        sRawBeatLenght += QString::number(*beat - *previousBeat, 'f') + ",";
        previousBeat = beat; 
    }
    QString sCorrectedBeats;
    QString sCorrectedBeatLenght;
    previousBeat = correctedBeats.begin();
    sCorrectedBeats += QString::number(*previousBeat, 'f') + ",";
    for (auto beat = std::begin(correctedBeats) + 1, end = std::end(correctedBeats); beat != end; beat += 1) {
        sCorrectedBeats += QString::number(*beat, 'f') + ",";
        sCorrectedBeatLenght += QString::number(*beat - *previousBeat, 'f') + ",";
        previousBeat = beat; 
    }
    QString resultHeader = "\nRaw beats\n";
    debugFile.write(resultHeader.toLocal8Bit());
    debugFile.write(sRawBeats.toLocal8Bit());
    resultHeader = "\nCorrected beats\n";
    debugFile.write(resultHeader.toLocal8Bit());
    debugFile.write(sCorrectedBeats.toLocal8Bit());
    resultHeader = "\nRaw beat length\n";
    debugFile.write(resultHeader.toLocal8Bit());
    debugFile.write(sRawBeatLenght.toLocal8Bit());
    resultHeader = "\nCorrected beat length\n";
    debugFile.write(resultHeader.toLocal8Bit());
    debugFile.write(sCorrectedBeatLenght.toLocal8Bit());
    debugFile.write("\n");
    debugFile.close();
}

}

mixxx::BeatsPointer BeatFactory::loadBeatsFromByteArray(const Track& track,
        QString beatsVersion,
        QString beatsSubVersion,
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
        const bool bEnableFixedTempoCorrection,
        const bool bEnableOffsetCorrection,
        const bool bEnableIroning,
        const bool bEnableArrytimicRemoval,
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

    if (bEnableIroning && !bEnableFixedTempoCorrection) {
        fragments << QString("enable_ironing%1%2")
                             .arg(kSubVersionKeyValueSeparator,
                                     QString::number(1));
    }

    if (bEnableArrytimicRemoval && !bEnableFixedTempoCorrection) {
        fragments << QString("enable_arrytimic_removal%1%2")
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

mixxx::BeatsPointer BeatFactory::makePreferredBeats(const Track& track,
        QVector<double> beats,
        const QHash<QString, QString>& extraVersionInfo,
        const bool bEnableFixedTempoCorrection,
        const bool bEnableOffsetCorrection,
        const bool bEnableIroning,
        const bool bEnableArrytimicRemoval,
        const int iSampleRate,
        const int iTotalSamples,
        const int iMinBpm,
        const int iMaxBpm) {
    const QString version = getPreferredVersion(bEnableFixedTempoCorrection);
    const QString subVersion = getPreferredSubVersion(bEnableFixedTempoCorrection,
            bEnableOffsetCorrection,
            bEnableIroning,
            bEnableArrytimicRemoval,
            iMinBpm,
            iMaxBpm,
            extraVersionInfo);

    BeatUtils::printBeatStatistics(beats, iSampleRate);
    if (version == BEAT_GRID_2_VERSION) {
        double globalBpm = BeatUtils::calculateBpm(beats, iSampleRate, iMinBpm, iMaxBpm);
        double firstBeat = BeatUtils::calculateFixedTempoFirstBeat(
            bEnableOffsetCorrection,
            beats, iSampleRate, iTotalSamples, globalBpm);
        mixxx::BeatGrid* pGrid = new mixxx::BeatGrid(track, iSampleRate);
        // firstBeat is in frames here and setGrid() takes samples.
        pGrid->setGrid(globalBpm, firstBeat * 2);
        pGrid->setSubVersion(subVersion);
        return mixxx::BeatsPointer(pGrid, &BeatFactory::deleteBeats);
    } else if (version == BEAT_MAP_VERSION) {
        if (bEnableIroning) {
            QVector<double> correctedBeats = BeatUtils::correctBeatmap(
                    beats, mixxx::audio::SampleRate(iSampleRate), bEnableArrytimicRemoval);
            debugBeats(track, beats, correctedBeats, version, subVersion);
            beats = correctedBeats;
        }
        auto pMap = new mixxx::BeatMap(track, iSampleRate, beats);
        pMap->setSubVersion(subVersion);
        return mixxx::BeatsPointer(pMap, &BeatFactory::deleteBeats);

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
