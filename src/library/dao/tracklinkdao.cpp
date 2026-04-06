#include "library/dao/tracklinkdao.h"

#include <QSqlError>
#include <QSqlQuery>
#include <QtDebug>

bool TrackLinkDao::linkTracks(TrackId trackId,
        TrackId targetTrackId,
        double offsetMs,
        TrackLink::Type type) {
    if (!trackId.isValid() || !targetTrackId.isValid()) {
        return false;
    }

    QSqlQuery query(m_database);
    query.prepare(
            "INSERT INTO track_links (track_id, target_track_id, offset_ms, link_type) "
            "VALUES (:track_id, :target_track_id, :offset_ms, :link_type) "
            "ON CONFLICT(track_id, target_track_id, link_type) DO UPDATE SET "
            "offset_ms = excluded.offset_ms");
    query.bindValue(":track_id", trackId.toVariant());
    query.bindValue(":target_track_id", targetTrackId.toVariant());
    query.bindValue(":offset_ms", offsetMs);
    query.bindValue(":link_type", static_cast<int>(type));

    if (!query.exec()) {
        qWarning() << "TrackLinkDao::linkTracks failed:" << query.lastError().text();
        return false;
    }
    return true;
}

bool TrackLinkDao::unlinkTracks(TrackId trackId,
        TrackId targetTrackId,
        TrackLink::Type type) {
    if (!trackId.isValid() || !targetTrackId.isValid()) {
        return false;
    }

    QSqlQuery query(m_database);
    query.prepare(
            "DELETE FROM track_links "
            "WHERE track_id = :track_id AND target_track_id = :target_track_id "
            "AND link_type = :link_type");
    query.bindValue(":track_id", trackId.toVariant());
    query.bindValue(":target_track_id", targetTrackId.toVariant());
    query.bindValue(":link_type", static_cast<int>(type));

    if (!query.exec()) {
        qWarning() << "TrackLinkDao::unlinkTracks failed:" << query.lastError().text();
        return false;
    }
    return true;
}

QList<TrackLink> TrackLinkDao::getLinksForTrack(TrackId trackId) {
    QList<TrackLink> links;
    if (!trackId.isValid()) {
        return links;
    }

    QSqlQuery query(m_database);
    query.prepare(
            "SELECT id, track_id, target_track_id, offset_ms, link_type "
            "FROM track_links WHERE track_id = :track_id");
    query.bindValue(":track_id", trackId.toVariant());

    if (!query.exec()) {
        qWarning() << "TrackLinkDao::getLinksForTrack failed:" << query.lastError().text();
        return links;
    }

    while (query.next()) {
        TrackLink link;
        link.id = query.value(0).toInt();
        link.trackId = TrackId(query.value(1));
        link.targetTrackId = TrackId(query.value(2));
        link.offsetMs = query.value(3).toDouble();
        link.type = static_cast<TrackLink::Type>(query.value(4).toInt());
        links.append(link);
    }
    return links;
}

QList<TrackLink> TrackLinkDao::getLinksToTarget(TrackId targetTrackId) {
    QList<TrackLink> links;
    if (!targetTrackId.isValid()) {
        return links;
    }

    QSqlQuery query(m_database);
    query.prepare(
            "SELECT id, track_id, target_track_id, offset_ms, link_type "
            "FROM track_links WHERE target_track_id = :target_track_id");
    query.bindValue(":target_track_id", targetTrackId.toVariant());

    if (!query.exec()) {
        qWarning() << "TrackLinkDao::getLinksToTarget failed:" << query.lastError().text();
        return links;
    }

    while (query.next()) {
        TrackLink link;
        link.id = query.value(0).toInt();
        link.trackId = TrackId(query.value(1));
        link.targetTrackId = TrackId(query.value(2));
        link.offsetMs = query.value(3).toDouble();
        link.type = static_cast<TrackLink::Type>(query.value(4).toInt());
        links.append(link);
    }
    return links;
}

bool TrackLinkDao::updateOffset(TrackId trackId,
        TrackId targetTrackId,
        TrackLink::Type type,
        double offsetMs) {
    if (!trackId.isValid() || !targetTrackId.isValid()) {
        return false;
    }

    QSqlQuery query(m_database);
    query.prepare(
            "UPDATE track_links SET offset_ms = :offset_ms "
            "WHERE track_id = :track_id AND target_track_id = :target_track_id "
            "AND link_type = :link_type");
    query.bindValue(":offset_ms", offsetMs);
    query.bindValue(":track_id", trackId.toVariant());
    query.bindValue(":target_track_id", targetTrackId.toVariant());
    query.bindValue(":link_type", static_cast<int>(type));

    if (!query.exec()) {
        qWarning() << "TrackLinkDao::updateOffset failed:" << query.lastError().text();
        return false;
    }
    return query.numRowsAffected() > 0;
}
