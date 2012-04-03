#include <QSqlQuery>
#include <QSqlResult>
#include <QSqlError>
#include <QDebug>

#include "waveform/waveform.h"
#include "library/dao/analysisdao.h"
#include "library/queryutil.h"

const QString AnalysisDao::s_analysisTableName = "track_analysis";

// For a track that takes 1.2MB to store the big waveform, the default
// compression level (-1) takes the size down to about 600KB. The difference
// between the default and 9 (the max) was only about 1-2KB for a lot of extra
// CPU time so I think we should stick with the default. rryan 4/3/2012
const int kCompressionLevel = -1;

AnalysisDao::AnalysisDao(const QSqlDatabase& database)
        : m_db(database) {
}

AnalysisDao::~AnalysisDao() {
}

void AnalysisDao::initialize() {
}

void AnalysisDao::setDatabase(const QSqlDatabase& database) {
    m_db = QSqlDatabase(database);
}

QList<AnalysisDao::AnalysisInfo> AnalysisDao::getAnalysesForTrack(int trackId) {
    QList<AnalysisDao::AnalysisInfo> analyses;
    if (!m_db.isOpen() || trackId == -1) {
        return analyses;
    }

    QSqlQuery query(m_db);
    query.prepare(QString(
        "SELECT id, type, description, version, data FROM %1 "
        "WHERE track_id=:trackId").arg(s_analysisTableName));
    query.bindValue(":trackId", trackId);
    if (!query.exec()) {
        LOG_FAILED_QUERY(query) << "couldn't get analyses for track by type";
        return analyses;
    }

    while (query.next()) {
        AnalysisDao::AnalysisInfo info;
        info.analysisId = query.value(query.record().indexOf("id")).toInt();
        info.trackId = trackId;
        info.type = static_cast<AnalysisType>(
            query.value(query.record().indexOf("type")).toInt());
        info.description = query.value(
            query.record().indexOf("description")).toString();
        info.version = query.value(
            query.record().indexOf("version")).toString();
        info.data = qUncompress(query.value(
            query.record().indexOf("data")).toByteArray());
        analyses.append(info);
    }
    return analyses;
}

QList<AnalysisDao::AnalysisInfo> AnalysisDao::getAnalysesForTrackByType(
    int trackId, AnalysisType type) {
    QList<AnalysisDao::AnalysisInfo> analyses;
    if (!m_db.isOpen() || trackId == -1) {
        return analyses;
    }

    QSqlQuery query(m_db);
    query.prepare(QString(
        "SELECT id, description, version, data FROM %1 "
        "WHERE track_id=:trackId AND type=:type").arg(s_analysisTableName));
    query.bindValue(":trackId", trackId);
    query.bindValue(":type", type);
    if (!query.exec()) {
        LOG_FAILED_QUERY(query) << "couldn't get analyses for track by type";
        return analyses;
    }

    while (query.next()) {
        AnalysisDao::AnalysisInfo info;
        info.analysisId = query.value(query.record().indexOf("id")).toInt();
        info.trackId = trackId;
        info.type = type;
        info.description = query.value(
            query.record().indexOf("description")).toString();
        info.version = query.value(
            query.record().indexOf("version")).toString();
        info.data = qUncompress(query.value(
            query.record().indexOf("data")).toByteArray());
        analyses.append(info);
    }
    return analyses;
}

bool AnalysisDao::saveAnalysis(AnalysisDao::AnalysisInfo* info) {
    if (!m_db.isOpen() || info == NULL) {
        return false;
    }

    if (info->trackId == -1) {
        qDebug() << "Can't save analysis since trackId is invalid.";
        return false;
    }

    QSqlQuery query(m_db);
    if (info->analysisId == -1) {
        query.prepare(QString(
            "INSERT INTO %1 (track_id, type, description, version, data) "
            "VALUES (:trackId,:type,:description,:version,:data)")
                      .arg(s_analysisTableName));

        QByteArray waveformBytes;
        query.bindValue(":trackId", info->trackId);
        query.bindValue(":type", info->type);
        query.bindValue(":description", info->description);
        query.bindValue(":version", info->version);
        query.bindValue(":data", qCompress(info->data, kCompressionLevel));

        if (!query.exec()) {
            LOG_FAILED_QUERY(query) << "couldn't save new analysis";
            return false;
        }
        info->analysisId = query.lastInsertId().toInt();
        return true;
    }

    query.prepare(QString(
        "UPDATE %1 SET "
        "track_id = :trackId,"
        "type = :type,"
        "description = :description,"
        "version = :version,"
        "data = :data "
        "WHERE id = :analysisId").arg(s_analysisTableName));

    query.bindValue(":analysisId", info->analysisId);
    query.bindValue(":trackId", info->trackId);
    query.bindValue(":type", info->type);
    query.bindValue(":description", info->description);
    query.bindValue(":version", info->version);
    query.bindValue(":data", qCompress(info->data, kCompressionLevel));

    if (!query.exec()) {
        LOG_FAILED_QUERY(query) << "couldn't update existing analysis";
        return false;
    }
    return true;
}

bool AnalysisDao::deleteAnalysis(int analysisId) {
    if (analysisId == -1) {
        return false;
    }
    QSqlQuery query(m_db);
    query.prepare(QString(
        "DELETE FROM %1 WHERE id = :id").arg(s_analysisTableName));
    query.bindValue(":id", analysisId);
    if (!query.exec()) {
        LOG_FAILED_QUERY(query) << "couldn't delete analysis";
        return false;
    }
    return true;
}

bool AnalysisDao::deleteAnalysesForTrack(int trackId) {
    if (trackId == -1) {
        return false;
    }
    QSqlQuery query(m_db);
    query.prepare(QString(
        "DELETE FROM %1 WHERE id = :id").arg(s_analysisTableName));
    query.bindValue(":id", trackId);
    if (!query.exec()) {
        LOG_FAILED_QUERY(query) << "couldn't delete analyses for track";
        return false;
    }
    return true;
}
