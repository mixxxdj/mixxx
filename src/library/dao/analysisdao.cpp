#include <QSqlQuery>
#include <QSqlResult>
#include <QSqlError>
#include <QtDebug>

#include "library/dao/analysisdao.h"
#include "library/queryutil.h"
#include "preferences/waveformsettings.h"
#include "util/performancetimer.h"
#include "waveform/waveform.h"

const QString AnalysisDao::s_analysisTableName = "track_analysis";

// For a track that takes 1.2MB to store the big waveform, the default
// compression level (-1) takes the size down to about 600KB. The difference
// between the default and 9 (the max) was only about 1-2KB for a lot of extra
// CPU time so I think we should stick with the default. rryan 4/3/2012
const int kCompressionLevel = -1;

AnalysisDao::AnalysisDao(UserSettingsPointer pConfig)
        : m_pConfig(pConfig) {
    QDir storagePath = getAnalysisStoragePath();
    if (!QDir().mkpath(storagePath.absolutePath())) {
        qDebug() << "WARNING: Could not create analysis storage path. Mixxx will be unable to store analyses.";
    }
}

QList<AnalysisDao::AnalysisInfo> AnalysisDao::getAnalysesForTrack(TrackId trackId) {
    if (!m_database.isOpen() || !trackId.isValid()) {
        return QList<AnalysisInfo>();
    }

    QSqlQuery query(m_database);
    query.prepare(QString(
        "SELECT id, type, description, version, data_checksum FROM %1 "
        "WHERE track_id=:trackId").arg(s_analysisTableName));
    query.bindValue(":trackId", trackId.toVariant());

    return loadAnalysesFromQuery(trackId, &query);
}

QList<AnalysisDao::AnalysisInfo> AnalysisDao::getAnalysesForTrackByType(
    TrackId trackId, AnalysisType type) {
    if (!m_database.isOpen() || !trackId.isValid()) {
        return QList<AnalysisInfo>();
    }

    QSqlQuery query(m_database);
    query.prepare(QString(
        "SELECT id, type, description, version, data_checksum FROM %1 "
        "WHERE track_id=:trackId AND type=:type").arg(s_analysisTableName));
    query.bindValue(":trackId", trackId.toVariant());
    query.bindValue(":type", type);

    return loadAnalysesFromQuery(trackId, &query);
}

QList<AnalysisDao::AnalysisInfo> AnalysisDao::loadAnalysesFromQuery(TrackId trackId, QSqlQuery* query) {
    QList<AnalysisDao::AnalysisInfo> analyses;
    PerformanceTimer time;
    time.start();

    if (!query->exec()) {
        LOG_FAILED_QUERY(*query) << "couldn't get analyses for track" << trackId;
        return analyses;
    }

    int bytes = 0;
    QSqlRecord queryRecord = query->record();
    const int idColumn = queryRecord.indexOf("id");
    const int typeColumn = queryRecord.indexOf("type");
    const int descriptionColumn = queryRecord.indexOf("description");
    const int versionColumn = queryRecord.indexOf("version");
    const int dataChecksumColumn = queryRecord.indexOf("data_checksum");

    QDir analysisPath(getAnalysisStoragePath());
    while (query->next()) {
        AnalysisDao::AnalysisInfo info;
        info.analysisId = query->value(idColumn).toInt();
        info.trackId = trackId;
        info.type = static_cast<AnalysisType>(query->value(typeColumn).toInt());
        info.description = query->value(descriptionColumn).toString();
        info.version = query->value(versionColumn).toString();
        int checksum = query->value(dataChecksumColumn).toInt();
        QString dataPath = analysisPath.absoluteFilePath(
            QString::number(info.analysisId));
        QByteArray compressedData = loadDataFromFile(dataPath);
        int file_checksum = qChecksum(compressedData.constData(),
                                      compressedData.length());
        if (checksum != file_checksum) {
            qDebug() << "WARNING: Corrupt analysis loaded from" << dataPath
                     << "length" << compressedData.length();
            continue;
        }
        info.data = qUncompress(compressedData);
        bytes += info.data.length();
        analyses.append(info);
    }
    qDebug() << "AnalysisDAO fetched" << analyses.size() << "analyses,"
             << bytes << "bytes for track"
             << trackId << "in" << time.elapsed().debugMillisWithUnit();
    return analyses;
}

bool AnalysisDao::saveAnalysis(AnalysisDao::AnalysisInfo* info) {
    if (!m_database.isOpen() || info == nullptr) {
        return false;
    }

    if (!info->trackId.isValid()) {
        qDebug() << "Can't save analysis since trackId is invalid.";
        return false;
    }
    PerformanceTimer time;
    time.start();

    QByteArray compressedData = qCompress(info->data, kCompressionLevel);
    int checksum = qChecksum(compressedData.constData(),
                             compressedData.length());

    QSqlQuery query(m_database);
    if (info->analysisId == -1) {
        query.prepare(QString(
            "INSERT INTO %1 (track_id, type, description, version, data_checksum) "
            "VALUES (:trackId,:type,:description,:version,:data_checksum)")
                      .arg(s_analysisTableName));

        query.bindValue(":trackId", info->trackId.toVariant());
        query.bindValue(":type", info->type);
        query.bindValue(":description", info->description);
        query.bindValue(":version", info->version);
        query.bindValue(":data_checksum", checksum);

        if (!query.exec()) {
            LOG_FAILED_QUERY(query) << "couldn't save new analysis";
            return false;
        }
        info->analysisId = query.lastInsertId().toInt();
    } else {
        query.prepare(QString(
            "UPDATE %1 SET "
            "track_id = :trackId,"
            "type = :type,"
            "description = :description,"
            "version = :version,"
            "data_checksum = :data_checksum "
            "WHERE id = :analysisId").arg(s_analysisTableName));

        query.bindValue(":analysisId", info->analysisId);
        query.bindValue(":trackId", info->trackId.toVariant());
        query.bindValue(":type", info->type);
        query.bindValue(":description", info->description);
        query.bindValue(":version", info->version);
        query.bindValue(":data_checksum", checksum);

        if (!query.exec()) {
            LOG_FAILED_QUERY(query) << "couldn't update existing analysis";
            return false;
        }
    }

    QString dataPath = getAnalysisStoragePath().absoluteFilePath(
        QString::number(info->analysisId));
    if (!saveDataToFile(dataPath, compressedData)) {
        qDebug() << "WARNING: Couldn't save analysis data to file" << dataPath;
        return false;
    }

    qDebug() << "AnalysisDAO saved analysis" << info->analysisId
             << QString("%1 (%2 compressed)").arg(QString::number(info->data.length()),
                                                  QString::number(compressedData.length()))
             << "bytes for track"
             << info->trackId << "in" << time.elapsed().debugMillisWithUnit();
    return true;
}

bool AnalysisDao::deleteAnalysis(const int analysisId) {
    if (analysisId == -1) {
        return false;
    }
    QSqlQuery query(m_database);
    query.prepare(QString(
        "DELETE FROM %1 WHERE id = :id").arg(s_analysisTableName));
    query.bindValue(":id", analysisId);

    if (!query.exec()) {
        LOG_FAILED_QUERY(query) << "couldn't delete analysis";
        return false;
    }

    QString dataPath = getAnalysisStoragePath().absoluteFilePath(
        QString::number(analysisId));
    deleteFile(dataPath);
    return true;
}

void AnalysisDao::deleteAnalyses(const QList<TrackId>& trackIds) {
    QStringList idList;
    for (const auto& trackId: trackIds) {
        idList << trackId.toString();
    }
    QSqlQuery query(m_database);
    query.prepare(QString("SELECT track_analysis.id FROM track_analysis WHERE "
                          "track_id in (%1)").arg(idList.join(",")));
    if (!query.exec()) {
        LOG_FAILED_QUERY(query) << "couldn't delete analysis";
    }
    const int idColumn = query.record().indexOf("id");
    QDir analysisPath(getAnalysisStoragePath());
    while (query.next()) {
        int id = query.value(idColumn).toInt();
        QString dataPath = analysisPath.absoluteFilePath(QString::number(id));
        deleteFile(dataPath);
    }
    query.prepare(QString("DELETE FROM track_analysis "
                          "WHERE track_id in (%1)").arg(idList.join(",")));
    if (!query.exec()) {
        LOG_FAILED_QUERY(query) << "couldn't delete analysis";
    }
}

bool AnalysisDao::deleteAnalysesForTrack(TrackId trackId) {
    if (!trackId.isValid()) {
        return false;
    }
    QSqlQuery query(m_database);
    query.prepare(QString(
        "SELECT id FROM %1 where track_id = :track_id").arg(s_analysisTableName));
    query.bindValue(":track_id", trackId.toVariant());

    if (!query.exec()) {
        LOG_FAILED_QUERY(query) << "couldn't delete analyses for track" << trackId;
        return false;
    }

    QList<int> analysesToDelete;
    const int idColumn = query.record().indexOf("id");
    while (query.next()) {
        analysesToDelete.append(
            query.value(idColumn).toInt());
    }
    foreach (int analysisId, analysesToDelete) {
        deleteAnalysis(analysisId);
    }
    return true;
}

QDir AnalysisDao::getAnalysisStoragePath() const {
    QString settingsPath = m_pConfig->getSettingsPath();
    QDir dir(settingsPath.append("/analysis/"));
    return dir.absolutePath().append("/");
}

QByteArray AnalysisDao::loadDataFromFile(const QString& filename) const {
    QFile file(filename);
    if (!file.exists()) {
        return QByteArray();
    }
    if (!file.open(QIODevice::ReadOnly)) {
        return QByteArray();
    }
    return file.readAll();
}

bool AnalysisDao::deleteFile(const QString& fileName) const {
    QFile file(fileName);
    return file.remove();
}

bool AnalysisDao::saveDataToFile(const QString& fileName, const QByteArray& data) const {
    QFile file(fileName);

    // If the file exists, do the right thing. Write to a temp file, unlink the
    // existing file, and then move the temp file to the real file's name.
    if (file.exists()) {
        QString tempFileName = fileName + ".tmp";
        QFile tempFile(tempFileName);
        if (!tempFile.open(QIODevice::WriteOnly)) {
            return false;
        }
        int bytesWritten = tempFile.write(data);
        if (bytesWritten == -1 || bytesWritten != data.length()) {
            return false;
        }
        tempFile.close();
        if (!file.remove()) {
            return false;
        }
        if (!tempFile.rename(fileName)) {
            return false;
        }
        return true;
    }

    // If the file doesn't exist, just create a new file and write the data.
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    int bytesWritten = file.write(data);
    if (bytesWritten == -1 || bytesWritten != data.length()) {
        return false;
    }
    file.close();
    return true;
}

void AnalysisDao::saveTrackAnalyses(
        TrackId trackId,
        ConstWaveformPointer pWaveform,
        ConstWaveformPointer pWaveSummary) {
    // The only analyses we have at the moment are waveform analyses so we have
    // nothing to do if it is disabled.
    WaveformSettings waveformSettings(m_pConfig);
    if (!waveformSettings.waveformCachingEnabled()) {
        return;
    }

    // Don't try to save invalid or non-dirty waveforms.
    if (!pWaveform || pWaveform->saveState() != Waveform::SaveState::SavePending ||
        !pWaveSummary || pWaveSummary->saveState() != Waveform::SaveState::SavePending) {
        return;
    }

    AnalysisDao::AnalysisInfo analysis;
    analysis.trackId = trackId;
    if (pWaveform->getId() != -1) {
        analysis.analysisId = pWaveform->getId();
    }
    analysis.type = AnalysisDao::TYPE_WAVEFORM;
    analysis.description = pWaveform->getDescription();
    analysis.version = pWaveform->getVersion();
    analysis.data = pWaveform->toByteArray();
    bool success = saveAnalysis(&analysis);
    if (success) {
        pWaveform->setSaveState(Waveform::SaveState::Saved);
    }

    qDebug() << (success ? "Saved" : "Failed to save")
                 << "waveform analysis for trackId" << trackId
                 << "analysisId" << analysis.analysisId;

    // Clear analysisId since we are re-using the AnalysisInfo
    analysis.analysisId = -1;
    analysis.type = AnalysisDao::TYPE_WAVESUMMARY;
    analysis.description = pWaveSummary->getDescription();
    analysis.version = pWaveSummary->getVersion();
    analysis.data = pWaveSummary->toByteArray();

    success = saveAnalysis(&analysis);
    if (success) {
        pWaveSummary->setSaveState(Waveform::SaveState::Saved);
    }
    qDebug() << (success ? "Saved" : "Failed to save")
             << "waveform summary analysis for trackId" << trackId
             << "analysisId" << analysis.analysisId;
}

size_t AnalysisDao::getDiskUsageInBytes(
        const QSqlDatabase& database,
        AnalysisType type) const {
    QDir analysisPath(getAnalysisStoragePath());

    QSqlQuery query(database);
    query.prepare(QString("SELECT id FROM %1 WHERE type=:type").arg(s_analysisTableName));
    query.bindValue(":type", type);

    if (!query.exec()) {
        LOG_FAILED_QUERY(query) << "couldn't get analyses of type" << type;
        return 0;
    }

    const int idColumn = query.record().indexOf("id");
    size_t total = 0;
    while (query.next()) {
        total += QFileInfo(analysisPath.absoluteFilePath(
                query.value(idColumn).toString())).size();
    }
    return total;
}

bool AnalysisDao::deleteAnalysesByType(
        const QSqlDatabase& database,
        AnalysisType type) const {
    QDir analysisPath(getAnalysisStoragePath());

    QSqlQuery query(database);
    query.prepare(QString("SELECT id FROM %1 WHERE type=:type").arg(s_analysisTableName));
    query.bindValue(":type", type);

    if (!query.exec()) {
        LOG_FAILED_QUERY(query) << "couldn't get analyses of type" << type;
        return false;
    }

    const int idColumn = query.record().indexOf("id");
    while (query.next()) {
        QString dataPath = analysisPath.absoluteFilePath(query.value(idColumn).toString());
        deleteFile(dataPath);
    }
    query.prepare(QString("DELETE FROM %1 WHERE type=:type").arg(s_analysisTableName));
    query.bindValue(":type", type);
    if (!query.exec()) {
        LOG_FAILED_QUERY(query) << "couldn't delete analysis";
    }

    return true;
}
