#include "library/dao/relationdao.h"

#include <QString>

#include "library/queryutil.h"
#include "moc_relationdao.cpp"
#include "track/relation.h"
#include "track/track.h"
#include "util/db/fwdsqlquery.h"

namespace {

Relation* relationFromRow(const QSqlRecord& row) {
    const auto id = DbId(row.value(row.indexOf("id")));
    TrackId sourceTrackId = TrackId(row.value(row.indexOf("source_track_id")));
    TrackId targetTrackId = TrackId(row.value(row.indexOf("target_track_id")));
    QVariant qSourcePosition = row.value(row.indexOf("source_position"));
    std::optional<mixxx::audio::FramePos> sourcePosition = qSourcePosition.isNull()
            ? std::nullopt
            : std::optional<mixxx::audio::FramePos>(qSourcePosition.toInt());
    QVariant qTargetPosition = row.value(row.indexOf("target_position"));
    std::optional<mixxx::audio::FramePos> targetPosition = qTargetPosition.isNull()
            ? std::nullopt
            : std::optional<mixxx::audio::FramePos>(qTargetPosition.toInt());
    const bool bidirectional = row.value(row.indexOf("bidirectional")).toBool();
    QString comment = row.value(row.indexOf("comment")).toString();
    QString tag = row.value(row.indexOf("tag")).toString();
    QDateTime dateAdded = row.value(row.indexOf("datetime_added")).toDateTime();

    Relation* relation = new Relation(
            id,
            sourceTrackId,
            targetTrackId,
            sourcePosition,
            targetPosition,
            bidirectional,
            comment,
            tag,
            dateAdded);
    return relation;
}

} // namespace

QList<Relation*> RelationDAO::getRelations(TrackId trackId,
        bool trackIsSource,
        bool trackIsTarget,
        std::optional<bool> bidirectional) const {
    QList<Relation*> relations;

    QSqlQuery query(m_database);
    QString queryText = QString("SELECT * FROM " RELATIONS_TABLE " WHERE 1=1");
    if (trackIsSource) {
        queryText.append(QString(" AND source_track_id=:id"));
    }
    if (trackIsTarget) {
        queryText.append(QString(" AND target_track_id=:id"));
    }
    if (bidirectional.has_value()) {
        queryText.append(QString(" AND bidirectional=:bi"));
    }
    const int iBidirectional = bidirectional.value_or(0);

    query.prepare(queryText);
    query.bindValue(":id", trackId.toVariant());
    query.bindValue(":bi", iBidirectional);
    if (query.exec()) {
        while (query.next()) {
            relations.append(relationFromRow(query.record()));
        }
    }
    return relations;
}

void RelationDAO::saveRelation(Relation* relation) {
    QSqlQuery query(m_database);
    if (relation->getId().isValid()) {
        // Update relation
        query.prepare(QStringLiteral("UPDATE " RELATIONS_TABLE " SET "
                                     "source_track_id=:source_track_id,"
                                     "target_track_id=:target_track_id,"
                                     "source_position=:source_position,"
                                     "target_position=:target_position,"
                                     "bidirectional=:bidirectional,"
                                     "comment=:comment,"
                                     "tag=:tag,"
                                     "datetime_added=:datetime_added"
                                     " WHERE id=:id"));
        query.bindValue(":id", relation->getId().toVariant());
    } else {
        // New relation
        query.prepare(QStringLiteral("INSERT INFO " RELATIONS_TABLE
                                     " (source_track_id, target_track_id, source_position, "
                                     "target_position, bidirectional, comment, tag) VALUES "
                                     "(:source_track_id, :target_track_id, "
                                     ":source_position, :target_position, :bidirectional, "
                                     ":comment, :tag)"));
    }

    // Bind values and execute query
    query.bindValue(":source_track_id", relation->getSourceTrackId().toVariant());
    query.bindValue(":target_track_id", relation->getTargetTrackId().toVariant());
    if (relation->getSourcePosition().has_value()) {
        query.bindValue(":source_position",
                relation->getSourcePosition()->toEngineSamplePosMaybeInvalid());
    } else {
        query.bindValue(":source_position", QVariant());
    }
    if (relation->getTargetPosition().has_value()) {
        query.bindValue(":target_position",
                relation->getTargetPosition()->toEngineSamplePosMaybeInvalid());
    } else {
        query.bindValue(":target_position", QVariant());
    }
    query.bindValue(":bidirectional", relation->getBidirectional());
    query.bindValue(":comment", relation->getcomment());
    query.bindValue(":tag", relation->getTag());
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return;
    }

    if (!relation->getId().isValid()) {
        // Set ID if new relation was added
        const auto newId = DbId(query.lastInsertId());
        DEBUG_ASSERT(newId.isValid());
        relation->setId(newId);
    }
    DEBUG_ASSERT(relation->getId().isValid());
}

void RelationDAO::deleteRelation(DbId relationId) {
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral("DELETE FROM " RELATIONS_TABLE " WHERE id=:id"));
    query.bindValue(":id", relationId.toVariant());
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
    }
}
