#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlResult>
#include <QSqlError>
#include <QDebug>
#include <QStringBuilder>

#include "library/queryutil.h"
#include "library/dao/socialtagdao.h"
#include "lastfm/lastfmclient.h"
#include "track/tagutils.h"


SocialTagDao::SocialTagDao(QSqlDatabase& database)
            : m_db(database) {
}

SocialTagDao::~SocialTagDao() {
}

void SocialTagDao::initialize() {
}

void SocialTagDao::setDatabase(QSqlDatabase& database) {
    m_db = database;
}

TagCounts SocialTagDao::getTagsForTrack(int trackId) {
    if (!m_db.isOpen() || trackId == -1) {
        return TagCounts();
    }

    QSqlQuery query(m_db);
    query.prepare(
        "SELECT name, count FROM " TAG_TABLE " as tags, "
        TRACK_TAG_TABLE " as tracktags "
        "WHERE tracktags.track_id = :trackId "
        "AND tags.id = tracktags.tag_id");
    query.bindValue(":trackId", trackId);

    return loadTagsFromQuery(trackId, query);
}

void SocialTagDao::saveTrackTags(int trackId, TrackInfoObject* pTrack) {
    qDebug() << "Saving track tags";
    TagCounts tags = pTrack->getTags();
    if (!tags.isEmpty()) {
        setTagsForTrack(trackId, tags);
    }
}

bool SocialTagDao::setTagsForTrack(int trackId, TagCounts tags) {
    QSqlQuery tagQuery(m_db);
    tagQuery.prepare(
        "INSERT OR REPLACE INTO " TAG_TABLE " (name, source) "
        "VALUES(:name, :source)");

    QList<QVariant> trackIds;
    QList<QVariant> tagIds;
    QList<QVariant> counts;

    foreach (QString tag , tags.keys()) {
        tagQuery.bindValue(":name", tag);

        //TODO(chrisjr): allow for multiple sources, rather than hard-coding
        tagQuery.bindValue(":source", "last.fm");
        if (!tagQuery.exec()) {
            LOG_FAILED_QUERY(tagQuery) << "tag insert failed";
            return false;
        }
        trackIds << trackId;
        tagIds << tagQuery.lastInsertId();
        counts << tags[tag];
    }

    QSqlQuery query(m_db);
    query.prepare("INSERT OR REPLACE INTO " TRACK_TAG_TABLE
        " (track_id, tag_id, count) "
        "VALUES (:trackId,:tagId,:count)");

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
    query.prepare("DELETE FROM " TAG_TABLE " WHERE track_id = :track_id");
    query.bindValue(":track_id", trackId);
    if (!query.exec()) {
        LOG_FAILED_QUERY(query) << "couldn't clear tags for" << trackId;
        return false;
    }
    return true;

}

bool SocialTagDao::clearTagsForTracks(QList<int> ids) {
    QStringList idList;
    foreach (int id, ids) {
        idList << QString::number(id);
    }

    QSqlQuery query(m_db);
    query.prepare(QString("DELETE FROM " TRACK_TAG_TABLE
            " WHERE track_id in (") % idList.join(",") % QString(")"));
    if (!query.exec()) {
        LOG_FAILED_QUERY(query) << "couldn't delete analysis";
        return false;
    }

    return true;
}

TagCounts SocialTagDao::loadTagsFromQuery(int trackId, QSqlQuery &query) {
    TagCounts tags;
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
