#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlResult>
#include <QSqlError>
#include <QDebug>

#include "library/dao/socialtagdao.h"
#include "library/queryutil.h"
#include "lastfm/lastfmclient.h"

SocialTagDao::SocialTagDao(QSqlDatabase& database)
    : m_db(database) {
}

SocialTagDao::~SocialTagDao() {
}

void SocialTagDao::setDatabase(QSqlDatabase& database) {
    m_db = database;
}

LastFmClient::TagCounts SocialTagDao::getTagsForTrack(int trackId) {
    if (!m_db.isOpen() || trackId == -1) {
        return LastFmClient::TagCounts();
    }

    QSqlQuery query(m_db);
    query.prepare(QString(
        "SELECT name, count FROM %1 as tags, %2 as tracktags "
        "WHERE tracktags.track_id=:trackId "
        "AND tags.id = tracktags.tag_id")
        .arg(m_sTagTableName)
        .arg(m_sTrackTagTableName));
    query.bindValue(":trackId", trackId);

    return loadTagsFromQuery(trackId, query);
}

bool SocialTagDao::setTagsForTrack(int trackId, LastFmClient::TagCounts tags) {
    QSqlQuery tagQuery(m_db);
    tagQuery.prepare(QString(
        "INSERT OR REPLACE INTO %1 (name, source) "
        "VALUES(:name, :source)").arg(m_sTagTableName));

    QMapIterator<QString, int> mapIt(tags);

    QList<QVariant> trackIds;
    QList<QVariant> tagIds;
    QList<QVariant> counts;

    while (mapIt.hasNext()) {
        mapIt.next();
        tagQuery.bindValue(":name", mapIt.key());

        //TODO(chrisjr): allow for multiple sources, rather than hard-coding
        tagQuery.bindValue(":source", "last.fm");
        if (!tagQuery.exec()) {
            LOG_FAILED_QUERY(tagQuery) << "tag insert failed";
            return false;
        }
        trackIds << trackId;
        tagIds << tagQuery.lastInsertId();
        counts << mapIt.value();
    }

    QSqlQuery query(m_db);
    query.prepare(QString(
        "INSERT INTO %1 (track_id, tag_id, count) "
        "VALUES (:trackId,:tagId,:count)")
                  .arg(m_sTrackTagTableName));

    query.bindValue(":trackId", trackIds);
    query.bindValue(":tagId", tagIds);
    query.bindValue(":count", counts);

    if (!query.execBatch()) {
        LOG_FAILED_QUERY(query) << "couldn't save tags for track" << trackId;
        return false;
    }

    return true;
}

bool SocialTagDao::clearTagsForTrack(int trackId) {
    if (trackId == -1) {
        return false;
    }
    QSqlQuery query(m_db);
    query.prepare(QString(
        "DELETE FROM %1 WHERE track_id = :track_id").arg(m_sTrackTagTableName));
    query.bindValue(":track_id", trackId);
    if (!query.exec()) {
        LOG_FAILED_QUERY(query) << "couldn't clear tags for" << trackId;
        return false;
    }
    return true;

}

LastFmClient::TagCounts SocialTagDao::loadTagsFromQuery(int trackId,
                                                        QSqlQuery &query) {
    LastFmClient::TagCounts tags;
    if (!query.exec()) {
        LOG_FAILED_QUERY(query) << "couldn't get tags for track" << trackId;
        return tags;
    }

    while (query.next()) {
        QSqlRecord queryRecord = query.record();
        QString tag = query.value(
            queryRecord.indexOf("name")).toString();
        int count = query.value(
            queryRecord.indexOf("count")).toInt();
        tags.insert(tag, count);
    }
    return tags;
}

const QString SocialTagDao::m_sTagTableName = "social_tags";
const QString SocialTagDao::m_sTrackTagTableName = "track_social_tags";
