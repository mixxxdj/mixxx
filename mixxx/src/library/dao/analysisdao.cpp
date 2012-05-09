#include <QSqlQuery>
#include <QSqlResult>
#include <QSqlError>
#include <QTime>
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

AnalysisDao::AnalysisDao(QSqlDatabase& database)
        : m_db(database) {
    QDir storagePath = getAnalysisStoragePath();
    if (!QDir().mkpath(storagePath.absolutePath())) {
        qDebug() << "WARNING: Could not create analysis storage path. Mixxx will be unable to store analyses.";
    }
}

AnalysisDao::~AnalysisDao() {
}

void AnalysisDao::initialize() {
}

void AnalysisDao::setDatabase(QSqlDatabase& database) {
    m_db = database;
}

QList<AnalysisDao::AnalysisInfo> AnalysisDao::getAnalysesForTrack(int trackId) {
    if (!m_db.isOpen() || trackId == -1) {
        return QList<AnalysisInfo>();
    }

    QSqlQuery query(m_db);
    query.prepare(QString(
        "SELECT id, type, description, version, data_checksum FROM %1 "
        "WHERE track_id=:trackId").arg(s_analysisTableName));
    query.bindValue(":trackId", trackId);

    return loadAnalysesFromQuery(trackId, query);
}

QList<AnalysisDao::AnalysisInfo> AnalysisDao::getAnalysesForTrackByType(
    int trackId, AnalysisType type) {
    if (!m_db.isOpen() || trackId == -1) {
        return QList<AnalysisInfo>();
    }

    QSqlQuery query(m_db);
    query.prepare(QString(
        "SELECT id, type, description, version, data_checksum FROM %1 "
        "WHERE track_id=:trackId AND type=:type").arg(s_analysisTableName));
    query.bindValue(":trackId", trackId);
    query.bindValue(":type", type);

    return loadAnalysesFromQuery(trackId, query);
}

QList<AnalysisDao::AnalysisInfo> AnalysisDao::loadAnalysesFromQuery(int trackId, QSqlQuery& query) {
    QList<AnalysisDao::AnalysisInfo> analyses;
    QTime time;
    time.start();

    if (!query.exec()) {
        LOG_FAILED_QUERY(query) << "couldn't get analyses for track" << trackId;
        return analyses;
    }

    int bytes = 0;
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
        int checksum = query.value(
            query.record().indexOf("data_checksum")).toInt();
        QString dataPath = getAnalysisStoragePath().absoluteFilePath(
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
             << trackId << "in" << time.elapsed() << "ms";
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
    QTime time;
    time.start();

    QByteArray compressedData = qCompress(info->data, kCompressionLevel);
    int checksum = qChecksum(compressedData.constData(),
                             compressedData.length());

    QSqlQuery query(m_db);
    if (info->analysisId == -1) {
        query.prepare(QString(
            "INSERT INTO %1 (track_id, type, description, version, data_checksum) "
            "VALUES (:trackId,:type,:description,:version,:data_checksum)")
                      .arg(s_analysisTableName));

        QByteArray waveformBytes;
        query.bindValue(":trackId", info->trackId);
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
        query.bindValue(":trackId", info->trackId);
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
             << info->trackId << "in" << time.elapsed() << "ms";
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

    QString dataPath = getAnalysisStoragePath().absoluteFilePath(
        QString::number(analysisId));
    deleteFile(dataPath);
    return true;
}

bool AnalysisDao::deleteAnalysesForTrack(int trackId) {
    if (trackId == -1) {
        return false;
    }
    QSqlQuery query(m_db);
    query.prepare(QString(
        "SELECT id FROM %1 where track_id = :track_id").arg(s_analysisTableName));
    query.bindValue(":track_id", trackId);

    if (!query.exec()) {
        LOG_FAILED_QUERY(query) << "couldn't delete analyses for track" << trackId;
        return false;
    }

    QList<int> analysesToDelete;
    while (query.next()) {
        analysesToDelete.append(
            query.value(query.record().indexOf("id")).toInt());
    }
    foreach (int analysisId, analysesToDelete) {
        deleteAnalysis(analysisId);
    }
    return true;
}

QDir AnalysisDao::getAnalysisStoragePath() const {
    return QDir(QDir::homePath().append("/").append(SETTINGS_PATH).append("analysis/"));
}

QByteArray AnalysisDao::loadDataFromFile(QString filename) const {
    QFile file(filename);
    if (!file.exists()) {
        return QByteArray();
    }
    if (!file.open(QIODevice::ReadOnly)) {
        return QByteArray();
    }
    return file.readAll();
}

bool AnalysisDao::deleteFile(QString fileName) const {
    QFile file(fileName);
    return file.remove();
}

bool AnalysisDao::saveDataToFile(QString fileName, QByteArray data) const {
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
