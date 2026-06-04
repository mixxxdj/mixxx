#include "library/dao/trackfingerprintdao.h"

#include <QSqlQuery>
#include <QSqlRecord>
#include <QVariant>
#include <QtDebug>

#include "library/queryutil.h"

namespace {
const QString kFingerprintTableName = QStringLiteral("fingerprint_metadata");
const QString kCmrtGroupsTableName = QStringLiteral("cmrt_groups");
const QString kCmrtMembersTableName = QStringLiteral("cmrt_members");
const QString kAcoustIdQueueTableName = QStringLiteral("acoustid_queue");
const QString kAcoustIdCacheTableName = QStringLiteral("acoustid_cache");
const QString kStatusQueued = QStringLiteral("queued");
const QString kStatusProcessing = QStringLiteral("processing");
const QString kStatusCompleted = QStringLiteral("completed");
const QString kStatusFailed = QStringLiteral("failed");
const bool sDebugTrackFingerprintDao = true;
} // namespace

TrackFingerprintDao::TrackFingerprintDao(UserSettingsPointer pConfig)
        : m_pConfig(pConfig) {
    QDir storagePath = getFingerprintStoragePath();
    if (!QDir().mkpath(storagePath.absolutePath())) {
        qDebug() << "WARNING: Could not create fingerprint storage path."
                    " Mixxx will be unable to store .chroma files.";
    }
}

bool TrackFingerprintDao::saveFingerprintMetadata(const FingerprintMetadata& metadata) const {
    if (sDebugTrackFingerprintDao) {
        qDebug() << "TrackFingerprintDao -> [saveFingerprintMetadata] -> entry"
                 << "trackId:" << metadata.trackId
                 << "hash:" << metadata.fingerprintHash
                 << "sha256:" << metadata.chromaSha256;
    }

    if (!m_database.isOpen() || !metadata.trackId.isValid()) {
        qDebug() << "TrackFingerprintDao -> [saveFingerprintMetadata] -> "
                    "aborting: database not open or invalid trackId";
        return false;
    }

    QSqlQuery query(m_database);
    // Try to update first (standard Mixxx pattern when trackId is the unique identifier)
    query.prepare(QString(
            "UPDATE %1 SET "
            "fingerprint_hash=:hash, chroma_sha256=:sha256, fingerprint_duration=:duration, "
            "fingerprint_version=:version, cmrt_group_id=:group_id, "
            "cmrt_offset_seconds=:offset, is_canonical=:canonical, "
            "fingerprint_valid=:valid, fingerprint_needs_regen=:needs_regen, "
            "computed_at=:computed_at "
            "WHERE track_id=:track_id")
                    .arg(kFingerprintTableName));

    query.bindValue(":track_id", metadata.trackId.toVariant());
    query.bindValue(":hash", metadata.fingerprintHash);
    query.bindValue(":sha256", metadata.chromaSha256);
    query.bindValue(":duration", metadata.fingerprintDuration);
    query.bindValue(":version", metadata.fingerprintVersion);
    // Support NULL for group_id if it's -1
    if (metadata.cmrtGroupId == -1) {
        query.bindValue(":group_id", QVariant(QMetaType(QMetaType::Int)));
    } else {
        query.bindValue(":group_id", metadata.cmrtGroupId);
    }
    query.bindValue(":offset", metadata.cmrtOffsetSeconds);
    query.bindValue(":canonical", metadata.isCanonical ? 1 : 0);
    query.bindValue(":valid", metadata.fingerprintValid ? 1 : 0);
    query.bindValue(":needs_regen", metadata.fingerprintNeedsRegen ? 1 : 0);
    // Schema stores computed_at as INTEGER (Unix timestamp)
    query.bindValue(":computed_at", metadata.computedAt.toSecsSinceEpoch());

    if (!query.exec()) {
        LOG_FAILED_QUERY(query)
                << "couldn't update fingerprint_metadata for track"
                << metadata.trackId;
        return false;
    }

    // If no row was updated, it means the row doesn't exist yet, so we INSERT
    if (query.numRowsAffected() == 0) {
        if (sDebugTrackFingerprintDao) {
            qDebug() << "TrackFingerprintDao -> [saveFingerprintMetadata] -> "
                        "no existing row, inserting"
                     << "trackId:" << metadata.trackId;
        }

        query.prepare(QString(
                "INSERT INTO %1 "
                "(track_id, fingerprint_hash, chroma_sha256, fingerprint_duration, "
                "fingerprint_version, cmrt_group_id, cmrt_offset_seconds, is_canonical, "
                "fingerprint_valid, fingerprint_needs_regen, computed_at) "
                "VALUES (:track_id, :hash, :sha256, :duration, :version, :group_id, "
                ":offset, :canonical, :valid, :needs_regen, :computed_at)")
                        .arg(kFingerprintTableName));

        query.bindValue(":track_id", metadata.trackId.toVariant());
        query.bindValue(":hash", metadata.fingerprintHash);
        query.bindValue(":sha256", metadata.chromaSha256);
        query.bindValue(":duration", metadata.fingerprintDuration);
        query.bindValue(":version", metadata.fingerprintVersion);
        if (metadata.cmrtGroupId == -1) {
            query.bindValue(":group_id", QVariant(QMetaType(QMetaType::Int)));
        } else {
            query.bindValue(":group_id", metadata.cmrtGroupId);
        }
        query.bindValue(":offset", metadata.cmrtOffsetSeconds);
        query.bindValue(":canonical", metadata.isCanonical ? 1 : 0);
        query.bindValue(":valid", metadata.fingerprintValid ? 1 : 0);
        query.bindValue(":needs_regen", metadata.fingerprintNeedsRegen ? 1 : 0);
        query.bindValue(":computed_at", metadata.computedAt.toSecsSinceEpoch());

        if (!query.exec()) {
            LOG_FAILED_QUERY(query)
                    << "couldn't insert fingerprint_metadata for track"
                    << metadata.trackId;
            return false;
        }
    } else {
        if (sDebugTrackFingerprintDao) {
            qDebug() << "TrackFingerprintDao -> [saveFingerprintMetadata] -> "
                        "updated existing row"
                     << "trackId:" << metadata.trackId;
        }
    }
    return true;
}

std::unique_ptr<FingerprintMetadata> TrackFingerprintDao::getFingerprintMetadata(
        TrackId trackId) const {
    if (sDebugTrackFingerprintDao) {
        qDebug() << "TrackFingerprintDao -> [getFingerprintMetadata] -> entry"
                 << "trackId:" << trackId;
    }

    if (!m_database.isOpen() || !trackId.isValid()) {
        qDebug() << "TrackFingerprintDao -> [getFingerprintMetadata] -> "
                    "aborting: database not open or invalid trackId";
        return nullptr;
    }

    QSqlQuery query(m_database);
    query.prepare(QString(
            "SELECT fingerprint_hash, chroma_sha256, fingerprint_duration, fingerprint_version, "
            "cmrt_group_id, cmrt_offset_seconds, is_canonical, fingerprint_valid, "
            "fingerprint_needs_regen, computed_at "
            "FROM %1 WHERE track_id=:track_id")
                    .arg(kFingerprintTableName));
    query.bindValue(":track_id", trackId.toVariant());

    if (!query.exec()) {
        LOG_FAILED_QUERY(query) << "couldn't fetch fingerprint_metadata for track" << trackId;
        return nullptr;
    }

    if (query.next()) {
        auto metadata = std::make_unique<FingerprintMetadata>();
        metadata->trackId = trackId;

        QSqlRecord record = query.record();
        metadata->fingerprintHash = query.value(record.indexOf("fingerprint_hash")).toUInt();
        metadata->chromaSha256 = query.value(record.indexOf("chroma_sha256")).toString();
        metadata->fingerprintDuration =
                query.value(record.indexOf("fingerprint_duration")).toDouble();
        metadata->fingerprintVersion =
                query.value(record.indexOf("fingerprint_version")).toInt();

        QVariant groupIdVar = query.value(record.indexOf("cmrt_group_id"));
        metadata->cmrtGroupId = groupIdVar.isNull() ? -1 : groupIdVar.toInt();

        metadata->cmrtOffsetSeconds =
                query.value(record.indexOf("cmrt_offset_seconds")).toDouble();
        metadata->isCanonical = query.value(record.indexOf("is_canonical")).toBool();
        metadata->fingerprintValid = query.value(record.indexOf("fingerprint_valid")).toBool();
        metadata->fingerprintNeedsRegen =
                query.value(record.indexOf("fingerprint_needs_regen")).toBool();
        // Schema stores computed_at as INTEGER (Unix timestamp)
        metadata->computedAt = QDateTime::fromSecsSinceEpoch(
                query.value(record.indexOf("computed_at")).toLongLong());

        if (sDebugTrackFingerprintDao) {
            qDebug() << "TrackFingerprintDao -> [getFingerprintMetadata] -> found row"
                     << "trackId:" << trackId
                     << "hash:" << metadata->fingerprintHash
                     << "groupId:" << metadata->cmrtGroupId
                     << "valid:" << metadata->fingerprintValid
                     << "needsRegen:" << metadata->fingerprintNeedsRegen;
        }

        return metadata;
    }

    if (sDebugTrackFingerprintDao) {
        qDebug() << "TrackFingerprintDao -> [getFingerprintMetadata] -> "
                    "no row found for trackId:"
                 << trackId;
    }
    return nullptr;
}

bool TrackFingerprintDao::markFingerprintNeedsRegen(TrackId trackId) const {
    if (sDebugTrackFingerprintDao) {
        qDebug() << "TrackFingerprintDao -> [markFingerprintNeedsRegen] -> entry"
                 << "trackId:" << trackId;
    }

    if (!m_database.isOpen() || !trackId.isValid()) {
        qDebug() << "TrackFingerprintDao -> [markFingerprintNeedsRegen] -> "
                    "aborting: database not open or invalid trackId";
        return false;
    }

    QSqlQuery query(m_database);
    query.prepare(QString(
            "UPDATE %1 SET fingerprint_needs_regen=1, fingerprint_valid=0 "
            "WHERE track_id=:track_id")
                    .arg(kFingerprintTableName));
    query.bindValue(":track_id", trackId.toVariant());

    if (!query.exec()) {
        LOG_FAILED_QUERY(query) << "couldn't mark fingerprint regen for track" << trackId;
        return false;
    }

    const bool affected = query.numRowsAffected() > 0;
    if (sDebugTrackFingerprintDao) {
        qDebug() << "TrackFingerprintDao -> [markFingerprintNeedsRegen] ->"
                 << (affected ? "marked" : "no row found")
                 << "trackId:" << trackId;
    }
    return affected;
}

bool TrackFingerprintDao::deleteFingerprintMetadata(TrackId trackId) const {
    if (sDebugTrackFingerprintDao) {
        qDebug() << "TrackFingerprintDao -> [deleteFingerprintMetadata] -> entry"
                 << "trackId:" << trackId;
    }

    if (!m_database.isOpen() || !trackId.isValid()) {
        qDebug() << "TrackFingerprintDao -> [deleteFingerprintMetadata] -> "
                    "aborting: database not open or invalid trackId";
        return false;
    }

    QSqlQuery query(m_database);
    query.prepare(QString("DELETE FROM %1 WHERE track_id=:track_id")
                    .arg(kFingerprintTableName));
    query.bindValue(":track_id", trackId.toVariant());

    if (!query.exec()) {
        LOG_FAILED_QUERY(query)
                << "couldn't delete fingerprint_metadata for track" << trackId;
        return false;
    }

    if (sDebugTrackFingerprintDao) {
        qDebug() << "TrackFingerprintDao -> [deleteFingerprintMetadata] -> done"
                 << "trackId:" << trackId
                 << "rowsAffected:" << query.numRowsAffected();
    }
    return true;
}

int TrackFingerprintDao::createCmrtGroup(const CmrtGroup& group) const {
    if (sDebugTrackFingerprintDao) {
        qDebug() << "TrackFingerprintDao -> [createCmrtGroup] -> entry"
                 << "sha256:" << group.chromaSha256
                 << "hash:" << group.fingerprintHash
                 << "canonicalTrackId:" << group.canonicalTrackId;
    }

    if (!m_database.isOpen()) {
        qDebug() << "TrackFingerprintDao -> [createCmrtGroup] -> "
                    "aborting: database not open";
        return -1;
    }

    QSqlQuery query(m_database);
    query.prepare(QString(
            "INSERT INTO %1 "
            "(fingerprint_hash, chroma_sha256, canonical_track_id, "
            "track_count, created_at, last_updated) "
            "VALUES (:hash, :sha256, :canonical_track, :count, :created, :updated)")
                    .arg(kCmrtGroupsTableName));

    query.bindValue(":hash", group.fingerprintHash);
    query.bindValue(":sha256", group.chromaSha256);
    query.bindValue(":canonical_track", group.canonicalTrackId.toVariant());
    query.bindValue(":count", group.trackCount);
    // Schema stores created_at and last_updated as INTEGER (Unix timestamps)
    query.bindValue(":created", group.createdAt.toSecsSinceEpoch());
    if (group.lastUpdated.isValid()) {
        query.bindValue(":updated", group.lastUpdated.toSecsSinceEpoch());
    } else {
        query.bindValue(":updated", QVariant(QMetaType(QMetaType::LongLong)));
    }

    if (!query.exec()) {
        LOG_FAILED_QUERY(query)
                << "couldn't create new cmrt_group with SHA256" << group.chromaSha256;
        return -1;
    }

    const int newGroupId = query.lastInsertId().toInt();
    if (sDebugTrackFingerprintDao) {
        qDebug() << "TrackFingerprintDao -> [createCmrtGroup] -> created group"
                 << "groupId:" << newGroupId
                 << "sha256:" << group.chromaSha256;
    }
    return newGroupId;
}

std::unique_ptr<CmrtGroup> TrackFingerprintDao::getCmrtGroup(int groupId) const {
    if (sDebugTrackFingerprintDao) {
        qDebug() << "TrackFingerprintDao -> [getCmrtGroup] -> entry"
                 << "groupId:" << groupId;
    }

    if (!m_database.isOpen() || groupId < 0) {
        qDebug() << "TrackFingerprintDao -> [getCmrtGroup] -> "
                    "aborting: database not open or invalid groupId";
        return nullptr;
    }

    QSqlQuery query(m_database);
    query.prepare(QString(
            "SELECT fingerprint_hash, chroma_sha256, canonical_track_id, track_count, "
            "created_at, last_updated, musicbrainz_cmrt_mbid, musicbrainz_synced, "
            "musicbrainz_last_sync, musicbrainz_community_score, musicbrainz_submitted, "
            "musicbrainz_submission_mbid, local_preferred, conflict_resolved_at, "
            "conflict_resolution "
            "FROM %1 WHERE group_id=:group_id")
                    .arg(kCmrtGroupsTableName));
    query.bindValue(":group_id", groupId);

    if (!query.exec()) {
        LOG_FAILED_QUERY(query) << "couldn't fetch cmrt_group with id" << groupId;
        return nullptr;
    }

    if (query.next()) {
        auto group = std::make_unique<CmrtGroup>();
        group->groupId = groupId;

        QSqlRecord record = query.record();
        group->fingerprintHash = query.value(record.indexOf("fingerprint_hash")).toUInt();
        group->chromaSha256 = query.value(record.indexOf("chroma_sha256")).toString();
        // TrackId must be constructed from QVariant, not int
        group->canonicalTrackId = TrackId(
                query.value(record.indexOf("canonical_track_id")));
        group->trackCount = query.value(record.indexOf("track_count")).toInt();
        // Schema stores created_at and last_updated as INTEGER (Unix timestamps)
        group->createdAt = QDateTime::fromSecsSinceEpoch(
                query.value(record.indexOf("created_at")).toLongLong());

        QVariant lastUpdatedVar = query.value(record.indexOf("last_updated"));
        if (!lastUpdatedVar.isNull()) {
            group->lastUpdated = QDateTime::fromSecsSinceEpoch(lastUpdatedVar.toLongLong());
        }

        group->musicbrainzCmrtMbid =
                query.value(record.indexOf("musicbrainz_cmrt_mbid")).toString();
        group->musicbrainzSynced =
                query.value(record.indexOf("musicbrainz_synced")).toBool();

        QVariant mbLastSync = query.value(record.indexOf("musicbrainz_last_sync"));
        if (!mbLastSync.isNull()) {
            group->musicbrainzLastSync =
                    QDateTime::fromSecsSinceEpoch(mbLastSync.toLongLong());
        }

        group->musicbrainzCommunityScore =
                query.value(record.indexOf("musicbrainz_community_score")).toDouble();
        group->musicbrainzSubmitted =
                query.value(record.indexOf("musicbrainz_submitted")).toBool();
        group->musicbrainzSubmissionMbid =
                query.value(record.indexOf("musicbrainz_submission_mbid")).toString();
        group->localPreferred =
                query.value(record.indexOf("local_preferred")).toBool();

        QVariant conflictAt = query.value(record.indexOf("conflict_resolved_at"));
        if (!conflictAt.isNull()) {
            group->conflictResolvedAt =
                    QDateTime::fromSecsSinceEpoch(conflictAt.toLongLong());
        }

        group->conflictResolution =
                query.value(record.indexOf("conflict_resolution")).toString();

        if (sDebugTrackFingerprintDao) {
            qDebug() << "TrackFingerprintDao -> [getCmrtGroup] -> found group"
                     << "groupId:" << groupId
                     << "sha256:" << group->chromaSha256
                     << "trackCount:" << group->trackCount
                     << "mbSynced:" << group->musicbrainzSynced;
        }

        return group;
    }

    if (sDebugTrackFingerprintDao) {
        qDebug() << "TrackFingerprintDao -> [getCmrtGroup] -> "
                    "no row found for groupId:"
                 << groupId;
    }
    return nullptr;
}

bool TrackFingerprintDao::addCmrtMember(const CmrtMember& member) const {
    if (sDebugTrackFingerprintDao) {
        qDebug() << "TrackFingerprintDao -> [addCmrtMember] -> entry"
                 << "trackId:" << member.trackId
                 << "groupId:" << member.groupId
                 << "offset:" << member.offsetFromCanonical;
    }

    if (!m_database.isOpen() || !member.trackId.isValid() || member.groupId < 0) {
        qDebug() << "TrackFingerprintDao -> [addCmrtMember] -> "
                    "aborting: database not open or invalid trackId/groupId";
        return false;
    }

    QSqlQuery query(m_database);
    query.prepare(QString(
            "INSERT INTO %1 "
            "(group_id, track_id, offset_from_canonical, quality_score, "
            "match_score, is_fake_lossless, added_at, user_quality_rating) "
            "VALUES (:group_id, :track_id, :offset, :quality_score, "
            ":match_score, :is_fake_lossless, :added_at, :user_quality_rating)")
                    .arg(kCmrtMembersTableName));

    query.bindValue(":group_id", member.groupId);
    query.bindValue(":track_id", member.trackId.toVariant());
    query.bindValue(":offset", member.offsetFromCanonical);
    // quality_score is nullable — -1.0 sentinel maps to NULL
    if (member.qualityScore < 0.0) {
        query.bindValue(":quality_score", QVariant(QMetaType(QMetaType::Double)));
    } else {
        query.bindValue(":quality_score", member.qualityScore);
    }
    // match_score is nullable — same -1.0 sentinel as quality_score
    if (member.matchScore < 0.0) {
        query.bindValue(":match_score", QVariant(QMetaType(QMetaType::Double)));
    } else {
        query.bindValue(":match_score", member.matchScore);
    }
    query.bindValue(":is_fake_lossless", member.isFakeLossless ? 1 : 0);
    // Schema stores added_at as INTEGER (Unix timestamp)
    query.bindValue(":added_at", member.addedAt.toSecsSinceEpoch());
    // user_quality_rating is nullable — -1 sentinel maps to NULL
    if (member.userQualityRating < 0) {
        query.bindValue(":user_quality_rating", QVariant(QMetaType(QMetaType::Int)));
    } else {
        query.bindValue(":user_quality_rating", member.userQualityRating);
    }

    if (!query.exec()) {
        LOG_FAILED_QUERY(query)
                << "couldn't add cmrt_member for track" << member.trackId
                << "to group" << member.groupId;
        return false;
    }

    if (sDebugTrackFingerprintDao) {
        qDebug() << "TrackFingerprintDao -> [addCmrtMember] -> inserted"
                 << "memberId:" << query.lastInsertId().toInt()
                 << "trackId:" << member.trackId
                 << "groupId:" << member.groupId;
    }
    return true;
}

bool TrackFingerprintDao::updateMemberMatchScore(TrackId trackId, double matchScore) const {
    if (sDebugTrackFingerprintDao) {
        qDebug() << "TrackFingerprintDao -> [updateMemberMatchScore] -> entry"
                 << "trackId:" << trackId
                 << "matchScore:" << matchScore;
    }

    if (!m_database.isOpen() || !trackId.isValid()) {
        qDebug() << "TrackFingerprintDao -> [updateMemberMatchScore] -> "
                    "aborting: database not open or invalid trackId";
        return false;
    }

    QSqlQuery query(m_database);
    query.prepare(QString(
            "UPDATE %1 SET match_score = :match_score WHERE track_id = :track_id")
                    .arg(kCmrtMembersTableName));
    // match_score is nullable -- same -1.0 sentinel as quality_score
    if (matchScore < 0.0) {
        query.bindValue(":match_score", QVariant(QMetaType(QMetaType::Double)));
    } else {
        query.bindValue(":match_score", matchScore);
    }
    query.bindValue(":track_id", trackId.toVariant());

    if (!query.exec()) {
        LOG_FAILED_QUERY(query) << "couldn't update match_score for track" << trackId;
        return false;
    }

    const bool affected = query.numRowsAffected() > 0;
    if (sDebugTrackFingerprintDao) {
        qDebug() << "TrackFingerprintDao -> [updateMemberMatchScore] ->"
                 << (affected ? "updated" : "no row found")
                 << "trackId:" << trackId;
    }
    return affected;
}

bool TrackFingerprintDao::updateMemberOffset(TrackId trackId, double offsetFromCanonical) const {
    if (sDebugTrackFingerprintDao) {
        qDebug() << "TrackFingerprintDao -> [updateMemberOffset] -> entry"
                 << "trackId:" << trackId
                 << "offsetFromCanonical:" << offsetFromCanonical;
    }

    if (!m_database.isOpen() || !trackId.isValid()) {
        qDebug() << "TrackFingerprintDao -> [updateMemberOffset] -> "
                    "aborting: database not open or invalid trackId";
        return false;
    }

    QSqlQuery query(m_database);
    query.prepare(QString(
            "UPDATE %1 SET offset_from_canonical = :offset WHERE track_id = :track_id")
                    .arg(kCmrtMembersTableName));
    query.bindValue(":offset", offsetFromCanonical);
    query.bindValue(":track_id", trackId.toVariant());

    if (!query.exec()) {
        LOG_FAILED_QUERY(query) << "couldn't update offset for track" << trackId;
        return false;
    }

    const bool affected = query.numRowsAffected() > 0;
    if (sDebugTrackFingerprintDao) {
        qDebug() << "TrackFingerprintDao -> [updateMemberOffset] ->"
                 << (affected ? "updated" : "no row found")
                 << "trackId:" << trackId;
    }
    return affected;
}

double TrackFingerprintDao::getMemberQualityScore(TrackId trackId) const {
    if (sDebugTrackFingerprintDao) {
        qDebug() << "TrackFingerprintDao -> [getMemberQualityScore] -> entry"
                 << "trackId:" << trackId;
    }

    if (!m_database.isOpen() || !trackId.isValid()) {
        qDebug() << "TrackFingerprintDao -> [getMemberQualityScore] -> "
                    "aborting: database not open or invalid trackId";
        return -1.0;
    }

    QSqlQuery query(m_database);
    query.prepare(QString(
            "SELECT quality_score FROM %1 WHERE track_id=:track_id")
                    .arg(kCmrtMembersTableName));
    query.bindValue(":track_id", trackId.toVariant());

    if (!query.exec()) {
        LOG_FAILED_QUERY(query) << "couldn't fetch quality_score for track" << trackId;
        return -1.0;
    }

    if (!query.next()) {
        if (sDebugTrackFingerprintDao) {
            qDebug() << "TrackFingerprintDao -> [getMemberQualityScore] -> "
                        "no cmrt_members row found for trackId:"
                     << trackId;
        }
        return -1.0;
    }

    // quality_score is nullable -- same sentinel addCmrtMember() already uses
    const QVariant qualityVar = query.value(0);
    const double score = qualityVar.isNull() ? -1.0 : qualityVar.toDouble();

    if (sDebugTrackFingerprintDao) {
        qDebug() << "TrackFingerprintDao -> [getMemberQualityScore] -> found"
                 << "trackId:" << trackId << "score:" << score;
    }
    return score;
}

QList<CmrtMember> TrackFingerprintDao::getCmrtMembersForGroup(int groupId) const {
    if (sDebugTrackFingerprintDao) {
        qDebug() << "TrackFingerprintDao -> [getCmrtMembersForGroup] -> entry"
                 << "groupId:" << groupId;
    }

    if (!m_database.isOpen() || groupId < 0) {
        qDebug() << "TrackFingerprintDao -> [getCmrtMembersForGroup] -> "
                    "aborting: database not open or invalid groupId";
        return {};
    }

    QSqlQuery query(m_database);
    query.prepare(QString(
            "SELECT member_id, track_id, offset_from_canonical, quality_score, "
            "match_score, is_fake_lossless, added_at, user_quality_rating "
            "FROM %1 WHERE group_id=:group_id")
                    .arg(kCmrtMembersTableName));
    query.bindValue(":group_id", groupId);

    if (!query.exec()) {
        LOG_FAILED_QUERY(query) << "couldn't fetch cmrt_members for group" << groupId;
        return {};
    }

    QList<CmrtMember> members;
    QSqlRecord record = query.record();
    while (query.next()) {
        CmrtMember member;
        member.memberId = query.value(record.indexOf("member_id")).toInt();
        member.groupId = groupId;
        member.trackId = TrackId(query.value(record.indexOf("track_id")));
        member.offsetFromCanonical =
                query.value(record.indexOf("offset_from_canonical")).toDouble();

        // quality_score is nullable
        QVariant qualityVar = query.value(record.indexOf("quality_score"));
        member.qualityScore = qualityVar.isNull() ? -1.0 : qualityVar.toDouble();

        // match_score is nullable, same sentinel as quality_score
        QVariant matchScoreVar = query.value(record.indexOf("match_score"));
        member.matchScore = matchScoreVar.isNull() ? -1.0 : matchScoreVar.toDouble();

        member.isFakeLossless = query.value(record.indexOf("is_fake_lossless")).toBool();
        // Schema stores added_at as INTEGER (Unix timestamp)
        member.addedAt = QDateTime::fromSecsSinceEpoch(
                query.value(record.indexOf("added_at")).toLongLong());

        // user_quality_rating is nullable
        QVariant ratingVar = query.value(record.indexOf("user_quality_rating"));
        member.userQualityRating = ratingVar.isNull() ? -1 : ratingVar.toInt();

        members.append(member);
    }

    if (sDebugTrackFingerprintDao) {
        qDebug() << "TrackFingerprintDao -> [getCmrtMembersForGroup] -> found"
                 << members.size() << "members for groupId:" << groupId;
    }
    return members;
}

bool TrackFingerprintDao::deleteCmrtMember(TrackId trackId) const {
    if (sDebugTrackFingerprintDao) {
        qDebug() << "TrackFingerprintDao -> [deleteCmrtMember] -> entry"
                 << "trackId:" << trackId;
    }

    if (!m_database.isOpen() || !trackId.isValid()) {
        qDebug() << "TrackFingerprintDao -> [deleteCmrtMember] -> "
                    "aborting: database not open or invalid trackId";
        return false;
    }

    QSqlQuery query(m_database);
    query.prepare(QString("DELETE FROM %1 WHERE track_id=:track_id")
                    .arg(kCmrtMembersTableName));
    query.bindValue(":track_id", trackId.toVariant());

    if (!query.exec()) {
        LOG_FAILED_QUERY(query)
                << "couldn't delete cmrt_member for track" << trackId;
        return false;
    }

    if (sDebugTrackFingerprintDao) {
        qDebug() << "TrackFingerprintDao -> [deleteCmrtMember] -> done"
                 << "trackId:" << trackId
                 << "rowsAffected:" << query.numRowsAffected();
    }
    return true;
}

bool TrackFingerprintDao::updateCmrtGroupTrackCount(int groupId, int delta) const {
    if (sDebugTrackFingerprintDao) {
        qDebug() << "TrackFingerprintDao -> [updateCmrtGroupTrackCount] -> entry"
                 << "groupId:" << groupId << "delta:" << delta;
    }

    if (!m_database.isOpen() || groupId < 0 || delta == 0) {
        qDebug() << "TrackFingerprintDao -> [updateCmrtGroupTrackCount] -> "
                    "aborting: database not open, invalid groupId, or zero delta";
        return false;
    }

    QSqlQuery query(m_database);
    query.prepare(QString(
            "UPDATE %1 SET "
            "track_count = track_count + :delta, "
            "last_updated = :last_updated "
            "WHERE group_id = :group_id")
                    .arg(kCmrtGroupsTableName));
    query.bindValue(":delta", delta);
    // Schema stores last_updated as INTEGER (Unix timestamp)
    query.bindValue(":last_updated", QDateTime::currentSecsSinceEpoch());
    query.bindValue(":group_id", groupId);

    if (!query.exec()) {
        LOG_FAILED_QUERY(query)
                << "couldn't update track_count for group" << groupId;
        return false;
    }

    const bool affected = query.numRowsAffected() > 0;
    if (sDebugTrackFingerprintDao) {
        qDebug() << "TrackFingerprintDao -> [updateCmrtGroupTrackCount] ->"
                 << (affected ? "updated" : "no row found")
                 << "groupId:" << groupId << "delta:" << delta;
    }
    return affected;
}

QDir TrackFingerprintDao::getFingerprintStoragePath() const {
    QString settingsPath = m_pConfig->getSettingsPath();
    QDir dir(settingsPath.append("/fingerprints/"));
    return dir.absolutePath().append("/");
}

QString TrackFingerprintDao::getChromaFilePath(TrackId trackId) const {
    return getFingerprintStoragePath().absoluteFilePath(
            QStringLiteral("track_%1.chroma").arg(trackId.toString()));
}

QByteArray TrackFingerprintDao::loadChromaFile(TrackId trackId) const {
    if (sDebugTrackFingerprintDao) {
        qDebug() << "TrackFingerprintDao -> [loadChromaFile] -> entry trackId:" << trackId;
    }

    if (!trackId.isValid()) {
        qDebug() << "TrackFingerprintDao -> [loadChromaFile] -> "
                    "aborting: invalid trackId";
        return {};
    }

    const QString path = getChromaFilePath(trackId);
    QFile file(path);
    if (!file.exists()) {
        if (sDebugTrackFingerprintDao) {
            qDebug() << "TrackFingerprintDao -> [loadChromaFile] -> file does not exist:" << path;
        }
        return {};
    }
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "TrackFingerprintDao -> [loadChromaFile] -> could not open file:" << path;
        return {};
    }

    QByteArray data = file.readAll();
    if (sDebugTrackFingerprintDao) {
        qDebug() << "TrackFingerprintDao -> [loadChromaFile] -> done, read bytes:" << data.size();
    }
    return data;
}

bool TrackFingerprintDao::saveChromaFile(TrackId trackId, const QByteArray& data) const {
    if (sDebugTrackFingerprintDao) {
        qDebug() << "TrackFingerprintDao -> [saveChromaFile] -> entry"
                 << "trackId:" << trackId
                 << "bytes:" << data.size();
    }

    if (!trackId.isValid() || data.isEmpty()) {
        qDebug() << "TrackFingerprintDao -> [saveChromaFile] -> "
                    "aborting: invalid trackId or empty data";
        return false;
    }

    const QString path = getChromaFilePath(trackId);
    QFile file(path);

    // Prevents a half-written file if Mixxx crashes mid-write.
    bool success = true;
    if (file.exists()) {
        const QString tempPath = path + QStringLiteral(".tmp");
        QFile tempFile(tempPath);
        if (!tempFile.open(QIODevice::WriteOnly)) {
            success = false;
        } else {
            if (tempFile.write(data) != data.size()) {
                tempFile.remove();
                success = false;
            }
            tempFile.close();
            if (success) {
                if (!file.remove()) {
                    success = false;
                } else if (!tempFile.rename(path)) {
                    success = false;
                }
            }
        }
    } else {
        if (!file.open(QIODevice::WriteOnly)) {
            success = false;
        } else {
            if (file.write(data) != data.size()) {
                success = false;
            }
            file.close();
        }
    }

    if (sDebugTrackFingerprintDao) {
        qDebug() << "TrackFingerprintDao -> [saveChromaFile] -> done, success:" << success;
    }
    return success;
}

bool TrackFingerprintDao::deleteChromaFile(TrackId trackId) const {
    if (sDebugTrackFingerprintDao) {
        qDebug() << "TrackFingerprintDao -> [deleteChromaFile] -> entry trackId:" << trackId;
    }

    if (!trackId.isValid()) {
        qDebug() << "TrackFingerprintDao -> [deleteChromaFile] -> aborting: invalid trackId";
        return false;
    }

    const QString path = getChromaFilePath(trackId);
    QFile file(path);
    if (!file.exists()) {
        if (sDebugTrackFingerprintDao) {
            qDebug() << "TrackFingerprintDao -> [deleteChromaFile] -> file "
                        "does not exist, nothing to delete";
        }
        return true;
    }

    bool result = file.remove();
    if (sDebugTrackFingerprintDao) {
        qDebug() << "TrackFingerprintDao -> [deleteChromaFile] -> file remove result:" << result;
    }
    return result;
}

bool TrackFingerprintDao::enqueueAcoustId(
        TrackId trackId, int priority) const {
    if (sDebugTrackFingerprintDao) {
        qDebug() << "TrackFingerprintDao -> [enqueueAcoustId] -> entry"
                 << "trackId:" << trackId
                 << "priority:" << priority;
    }

    if (!m_database.isOpen() || !trackId.isValid()) {
        qDebug() << "TrackFingerprintDao -> [enqueueAcoustId] -> "
                    "aborting: database not open or invalid trackId";
        return false;
    }

    QSqlQuery query(m_database);
    // INSERT OR IGNORE respects the UNIQUE(track_id) constraint —
    // a track already in the queue is not re-queued.
    query.prepare(QString(
            "INSERT OR IGNORE INTO %1 "
            "(track_id, priority, status, queued_at) "
            "VALUES (:track_id, :priority, :status, :queued_at)")
                    .arg(kAcoustIdQueueTableName));

    query.bindValue(":track_id", trackId.toVariant());
    query.bindValue(":priority", priority);
    query.bindValue(":status", kStatusQueued);
    query.bindValue(":queued_at", QDateTime::currentSecsSinceEpoch());

    if (!query.exec()) {
        LOG_FAILED_QUERY(query)
                << "couldn't enqueue AcoustID job for track" << trackId;
        return false;
    }

    const bool inserted = query.numRowsAffected() > 0;
    if (sDebugTrackFingerprintDao) {
        qDebug() << "TrackFingerprintDao -> [enqueueAcoustId] ->"
                 << (inserted ? "enqueued" : "already in queue")
                 << "trackId:" << trackId;
    }
    return true;
}

bool TrackFingerprintDao::updateQueueStatus(
        int queueId,
        const QString& status,
        const QString& errorMessage) const {
    if (sDebugTrackFingerprintDao) {
        qDebug() << "TrackFingerprintDao -> [updateQueueStatus] -> entry"
                 << "queueId:" << queueId
                 << "status:" << status;
    }

    if (!m_database.isOpen() || queueId < 0) {
        qDebug() << "TrackFingerprintDao -> [updateQueueStatus] -> "
                    "aborting: database not open or invalid queueId";
        return false;
    }

    QSqlQuery query(m_database);
    query.prepare(QString(
            "UPDATE %1 SET "
            "status = :status, "
            "attempts = attempts + 1, "
            "last_attempt = :last_attempt, "
            "error_message = :error_message "
            "WHERE queue_id = :queue_id")
                    .arg(kAcoustIdQueueTableName));

    query.bindValue(":status", status);
    query.bindValue(":last_attempt", QDateTime::currentSecsSinceEpoch());
    // Store NULL when there is no error message.
    if (errorMessage.isEmpty()) {
        query.bindValue(":error_message",
                QVariant(QMetaType(QMetaType::QString)));
    } else {
        query.bindValue(":error_message", errorMessage);
    }
    query.bindValue(":queue_id", queueId);

    if (!query.exec()) {
        LOG_FAILED_QUERY(query)
                << "couldn't update queue status for queueId" << queueId;
        return false;
    }

    const bool affected = query.numRowsAffected() > 0;
    if (sDebugTrackFingerprintDao) {
        qDebug() << "TrackFingerprintDao -> [updateQueueStatus] ->"
                 << (affected ? "updated" : "no row found")
                 << "queueId:" << queueId
                 << "status:" << status;
    }
    return affected;
}

QList<AcoustIdJob> TrackFingerprintDao::getPendingJobs(int limit) const {
    if (sDebugTrackFingerprintDao) {
        qDebug() << "TrackFingerprintDao -> [getPendingJobs] -> entry"
                 << "limit:" << limit;
    }

    if (!m_database.isOpen()) {
        qDebug() << "TrackFingerprintDao -> [getPendingJobs] -> "
                    "aborting: database not open";
        return {};
    }

    QSqlQuery query(m_database);
    // Returns jobs that are either waiting for a first attempt ('queued')
    // or eligible for a retry ('failed' and under max_attempts).
    // priority ASC, queued_at ASC.
    query.prepare(QString(
            "SELECT queue_id, track_id, priority, status, "
            "attempts, max_attempts, last_attempt, error_message, queued_at "
            "FROM %1 "
            "WHERE (status = :queued OR status = :failed) "
            "AND attempts < max_attempts "
            "ORDER BY priority ASC, queued_at ASC "
            "LIMIT :limit")
                    .arg(kAcoustIdQueueTableName));

    query.bindValue(":queued", kStatusQueued);
    query.bindValue(":failed", kStatusFailed);
    query.bindValue(":limit", limit);

    if (!query.exec()) {
        LOG_FAILED_QUERY(query) << "couldn't fetch pending AcoustID jobs";
        return {};
    }

    QList<AcoustIdJob> jobs;
    QSqlRecord record = query.record();
    while (query.next()) {
        AcoustIdJob job;
        job.queueId = query.value(record.indexOf("queue_id")).toInt();
        job.trackId = TrackId(query.value(record.indexOf("track_id")));
        job.priority = query.value(record.indexOf("priority")).toInt();
        job.status = query.value(record.indexOf("status")).toString();
        job.attempts = query.value(record.indexOf("attempts")).toInt();
        job.maxAttempts =
                query.value(record.indexOf("max_attempts")).toInt();

        QVariant lastAttemptVar =
                query.value(record.indexOf("last_attempt"));
        if (!lastAttemptVar.isNull()) {
            job.lastAttempt = QDateTime::fromSecsSinceEpoch(
                    lastAttemptVar.toLongLong());
        }

        job.errorMessage =
                query.value(record.indexOf("error_message")).toString();
        job.queuedAt = QDateTime::fromSecsSinceEpoch(
                query.value(record.indexOf("queued_at")).toLongLong());

        jobs.append(job);
    }

    if (sDebugTrackFingerprintDao) {
        qDebug() << "TrackFingerprintDao -> [getPendingJobs] -> found"
                 << jobs.size() << "pending jobs";
    }
    return jobs;
}

bool TrackFingerprintDao::deleteQueueEntry(TrackId trackId) const {
    if (sDebugTrackFingerprintDao) {
        qDebug() << "TrackFingerprintDao -> [deleteQueueEntry] -> entry"
                 << "trackId:" << trackId;
    }

    if (!m_database.isOpen() || !trackId.isValid()) {
        qDebug() << "TrackFingerprintDao -> [deleteQueueEntry] -> "
                    "aborting: database not open or invalid trackId";
        return false;
    }

    QSqlQuery query(m_database);
    query.prepare(QString("DELETE FROM %1 WHERE track_id = :track_id")
                    .arg(kAcoustIdQueueTableName));
    query.bindValue(":track_id", trackId.toVariant());

    if (!query.exec()) {
        LOG_FAILED_QUERY(query)
                << "couldn't delete queue entry for track" << trackId;
        return false;
    }

    if (sDebugTrackFingerprintDao) {
        qDebug() << "TrackFingerprintDao -> [deleteQueueEntry] -> done"
                 << "trackId:" << trackId
                 << "rowsAffected:" << query.numRowsAffected();
    }
    return true;
}

bool TrackFingerprintDao::cacheAcoustIdResult(
        const AcoustIdCacheEntry& entry) const {
    if (sDebugTrackFingerprintDao) {
        qDebug() << "TrackFingerprintDao -> [cacheAcoustIdResult] -> entry"
                 << "sha256:" << entry.chromaSha256
                 << "acoustidId:" << entry.acoustidId;
    }

    if (!m_database.isOpen() || entry.chromaSha256.isEmpty() ||
            entry.acoustidId.isEmpty()) {
        qDebug() << "TrackFingerprintDao -> [cacheAcoustIdResult] -> "
                    "aborting: database not open or empty sha256/acoustidId";
        return false;
    }

    QSqlQuery query(m_database);
    // Update first — consistent with saveFingerprintMetadata pattern.
    query.prepare(QString(
            "UPDATE %1 SET "
            "acoustid_id=:acoustid_id, "
            "musicbrainz_recording_id=:mb_recording, "
            "musicbrainz_release_id=:mb_release, "
            "musicbrainz_metadata=:mb_metadata, "
            "confidence=:confidence, "
            "lookup_timestamp=:lookup_timestamp, "
            "expires_at=:expires_at "
            "WHERE chroma_sha256=:sha256")
                    .arg(kAcoustIdCacheTableName));

    query.bindValue(":sha256", entry.chromaSha256);
    query.bindValue(":acoustid_id", entry.acoustidId);
    query.bindValue(":mb_recording",
            entry.musicbrainzRecordingId.isEmpty()
                    ? QVariant(QMetaType(QMetaType::QString))
                    : entry.musicbrainzRecordingId);
    query.bindValue(":mb_release",
            entry.musicbrainzReleaseId.isEmpty()
                    ? QVariant(QMetaType(QMetaType::QString))
                    : entry.musicbrainzReleaseId);
    query.bindValue(":mb_metadata",
            entry.musicbrainzMetadata.isEmpty()
                    ? QVariant(QMetaType(QMetaType::QString))
                    : entry.musicbrainzMetadata);
    // confidence: -1.0 sentinel maps to NULL
    if (entry.confidence < 0.0) {
        query.bindValue(":confidence", QVariant(QMetaType(QMetaType::Double)));
    } else {
        query.bindValue(":confidence", entry.confidence);
    }
    query.bindValue(":lookup_timestamp",
            entry.lookupTimestamp.toSecsSinceEpoch());
    // expires_at: isValid() == false maps to NULL (no TTL)
    if (entry.expiresAt.isValid()) {
        query.bindValue(":expires_at", entry.expiresAt.toSecsSinceEpoch());
    } else {
        query.bindValue(":expires_at",
                QVariant(QMetaType(QMetaType::LongLong)));
    }

    if (!query.exec()) {
        LOG_FAILED_QUERY(query)
                << "couldn't update acoustid_cache for sha256"
                << entry.chromaSha256;
        return false;
    }

    if (query.numRowsAffected() == 0) {
        if (sDebugTrackFingerprintDao) {
            qDebug() << "TrackFingerprintDao -> [cacheAcoustIdResult] -> "
                        "no existing row, inserting sha256:"
                     << entry.chromaSha256;
        }

        query.prepare(QString(
                "INSERT INTO %1 "
                "(chroma_sha256, acoustid_id, musicbrainz_recording_id, "
                "musicbrainz_release_id, musicbrainz_metadata, confidence, "
                "lookup_timestamp, expires_at) "
                "VALUES (:sha256, :acoustid_id, :mb_recording, :mb_release, "
                ":mb_metadata, :confidence, :lookup_timestamp, :expires_at)")
                        .arg(kAcoustIdCacheTableName));

        // Re-bind all values
        // QSqlQuery does not carry bindings across prepare() calls.
        query.bindValue(":sha256", entry.chromaSha256);
        query.bindValue(":acoustid_id", entry.acoustidId);
        query.bindValue(":mb_recording",
                entry.musicbrainzRecordingId.isEmpty()
                        ? QVariant(QMetaType(QMetaType::QString))
                        : entry.musicbrainzRecordingId);
        query.bindValue(":mb_release",
                entry.musicbrainzReleaseId.isEmpty()
                        ? QVariant(QMetaType(QMetaType::QString))
                        : entry.musicbrainzReleaseId);
        query.bindValue(":mb_metadata",
                entry.musicbrainzMetadata.isEmpty()
                        ? QVariant(QMetaType(QMetaType::QString))
                        : entry.musicbrainzMetadata);
        if (entry.confidence < 0.0) {
            query.bindValue(":confidence",
                    QVariant(QMetaType(QMetaType::Double)));
        } else {
            query.bindValue(":confidence", entry.confidence);
        }
        query.bindValue(":lookup_timestamp",
                entry.lookupTimestamp.toSecsSinceEpoch());
        if (entry.expiresAt.isValid()) {
            query.bindValue(":expires_at",
                    entry.expiresAt.toSecsSinceEpoch());
        } else {
            query.bindValue(":expires_at",
                    QVariant(QMetaType(QMetaType::LongLong)));
        }

        if (!query.exec()) {
            LOG_FAILED_QUERY(query)
                    << "couldn't insert acoustid_cache for sha256"
                    << entry.chromaSha256;
            return false;
        }
    } else {
        if (sDebugTrackFingerprintDao) {
            qDebug() << "TrackFingerprintDao -> [cacheAcoustIdResult] -> "
                        "updated existing row sha256:"
                     << entry.chromaSha256;
        }
    }
    return true;
}

std::unique_ptr<AcoustIdCacheEntry> TrackFingerprintDao::lookupAcoustIdCache(
        const QString& chromaSha256) const {
    if (sDebugTrackFingerprintDao) {
        qDebug() << "TrackFingerprintDao -> [lookupAcoustIdCache] -> entry"
                 << "sha256:" << chromaSha256;
    }

    if (!m_database.isOpen() || chromaSha256.isEmpty()) {
        qDebug() << "TrackFingerprintDao -> [lookupAcoustIdCache] -> "
                    "aborting: database not open or empty sha256";
        return nullptr;
    }

    QSqlQuery query(m_database);
    // Only return non-expired entries — matches Q5 in Final_Database.md.
    // Binding the current time rather than using strftime() keeps all
    // timestamp handling in C++, consistent with the rest of this file.
    query.prepare(QString(
            "SELECT acoustid_id, musicbrainz_recording_id, "
            "musicbrainz_release_id, musicbrainz_metadata, confidence, "
            "lookup_timestamp, expires_at "
            "FROM %1 "
            "WHERE chroma_sha256=:sha256 "
            "AND (expires_at IS NULL OR expires_at > :now)")
                    .arg(kAcoustIdCacheTableName));
    query.bindValue(":sha256", chromaSha256);
    query.bindValue(":now", QDateTime::currentSecsSinceEpoch());

    if (!query.exec()) {
        LOG_FAILED_QUERY(query)
                << "couldn't look up acoustid_cache for sha256" << chromaSha256;
        return nullptr;
    }

    if (query.next()) {
        auto entry = std::make_unique<AcoustIdCacheEntry>();
        entry->chromaSha256 = chromaSha256;

        QSqlRecord record = query.record();
        entry->acoustidId =
                query.value(record.indexOf("acoustid_id")).toString();
        entry->musicbrainzRecordingId =
                query.value(record.indexOf("musicbrainz_recording_id"))
                        .toString();
        entry->musicbrainzReleaseId =
                query.value(record.indexOf("musicbrainz_release_id"))
                        .toString();
        entry->musicbrainzMetadata =
                query.value(record.indexOf("musicbrainz_metadata")).toString();

        QVariant confidenceVar = query.value(record.indexOf("confidence"));
        entry->confidence = confidenceVar.isNull() ? -1.0
                                                   : confidenceVar.toDouble();

        entry->lookupTimestamp = QDateTime::fromSecsSinceEpoch(
                query.value(record.indexOf("lookup_timestamp")).toLongLong());

        QVariant expiresVar = query.value(record.indexOf("expires_at"));
        if (!expiresVar.isNull()) {
            entry->expiresAt =
                    QDateTime::fromSecsSinceEpoch(expiresVar.toLongLong());
        }

        if (sDebugTrackFingerprintDao) {
            qDebug() << "TrackFingerprintDao -> [lookupAcoustIdCache] -> hit"
                     << "sha256:" << chromaSha256
                     << "acoustidId:" << entry->acoustidId
                     << "confidence:" << entry->confidence;
        }
        return entry;
    }

    if (sDebugTrackFingerprintDao) {
        qDebug() << "TrackFingerprintDao -> [lookupAcoustIdCache] -> "
                    "miss (not found or expired) sha256:"
                 << chromaSha256;
    }
    return nullptr;
}

bool TrackFingerprintDao::deleteExpiredCacheEntries() const {
    if (sDebugTrackFingerprintDao) {
        qDebug() << "TrackFingerprintDao -> [deleteExpiredCacheEntries] -> entry";
    }

    if (!m_database.isOpen()) {
        qDebug() << "TrackFingerprintDao -> [deleteExpiredCacheEntries] -> "
                    "aborting: database not open";
        return false;
    }

    QSqlQuery query(m_database);
    query.prepare(QString(
            "DELETE FROM %1 "
            "WHERE expires_at IS NOT NULL AND expires_at <= :now")
                    .arg(kAcoustIdCacheTableName));
    query.bindValue(":now", QDateTime::currentSecsSinceEpoch());

    if (!query.exec()) {
        LOG_FAILED_QUERY(query) << "couldn't delete expired acoustid_cache entries";
        return false;
    }

    if (sDebugTrackFingerprintDao) {
        qDebug() << "TrackFingerprintDao -> [deleteExpiredCacheEntries] -> done"
                 << "rowsDeleted:" << query.numRowsAffected();
    }
    return true;
}

QList<UnmatchedTrackInfo> TrackFingerprintDao::getUnmatchedTracks() const {
    if (sDebugTrackFingerprintDao) {
        qDebug() << "TrackFingerprintDao -> [getUnmatchedTracks] -> entry";
    }

    if (!m_database.isOpen()) {
        qDebug() << "TrackFingerprintDao -> [getUnmatchedTracks] -> "
                    "aborting: database not open";
        return {};
    }

    // LEFT JOIN acoustid_queue so tracks that were never enqueued still appear
    // in the view — their status and attempts columns will be NULL.
    QSqlQuery query(m_database);
    query.prepare(
            "SELECT l.id, l.title, l.artist, l.duration, "
            "aq.status, aq.attempts, fm.fingerprint_valid "
            "FROM library l "
            "JOIN fingerprint_metadata fm ON l.id = fm.track_id "
            "LEFT JOIN acoustid_queue aq ON l.id = aq.track_id "
            "WHERE l.acoustid_id IS NULL "
            "AND fm.fingerprint_valid = 1 "
            "ORDER BY l.artist, l.title");

    if (!query.exec()) {
        LOG_FAILED_QUERY(query) << "couldn't fetch unmatched tracks";
        return {};
    }

    QList<UnmatchedTrackInfo> results;
    QSqlRecord record = query.record();
    while (query.next()) {
        UnmatchedTrackInfo info;
        info.trackId = TrackId(query.value(record.indexOf("id")));
        info.title = query.value(record.indexOf("title")).toString();
        info.artist = query.value(record.indexOf("artist")).toString();
        info.duration = query.value(record.indexOf("duration")).toDouble();

        QVariant statusVar = query.value(record.indexOf("status"));
        info.queueStatus = statusVar.isNull() ? QString() : statusVar.toString();

        QVariant attemptsVar = query.value(record.indexOf("attempts"));
        info.attempts = attemptsVar.isNull() ? 0 : attemptsVar.toInt();

        info.fingerprintValid =
                query.value(record.indexOf("fingerprint_valid")).toBool();

        results.append(info);
    }

    if (sDebugTrackFingerprintDao) {
        qDebug() << "TrackFingerprintDao -> [getUnmatchedTracks] -> found"
                 << results.size() << "unmatched tracks";
    }
    return results;
}

bool TrackFingerprintDao::reQueueJob(TrackId trackId) const {
    if (sDebugTrackFingerprintDao) {
        qDebug() << "TrackFingerprintDao -> [reQueueJob] -> entry"
                 << "trackId:" << trackId;
    }

    if (!m_database.isOpen() || !trackId.isValid()) {
        qDebug() << "TrackFingerprintDao -> [reQueueJob] -> "
                    "aborting: database not open or invalid trackId";
        return false;
    }

    QSqlQuery query(m_database);
    query.prepare(QString(
            "UPDATE %1 SET "
            "status='queued', attempts=0, "
            "error_message=NULL, last_attempt=NULL "
            "WHERE track_id=:track_id")
                    .arg(kAcoustIdQueueTableName));
    query.bindValue(":track_id", trackId.toVariant());

    if (!query.exec()) {
        LOG_FAILED_QUERY(query) << "couldn't re-queue job for track" << trackId;
        return false;
    }

    const bool affected = query.numRowsAffected() > 0;
    if (sDebugTrackFingerprintDao) {
        qDebug() << "TrackFingerprintDao -> [reQueueJob] ->"
                 << (affected ? "re-queued" : "no row found")
                 << "trackId:" << trackId;
    }
    return affected;
}
