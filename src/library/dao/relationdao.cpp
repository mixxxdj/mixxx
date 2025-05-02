#include "library/dao/relationdao.h"

#include <QString>

#include "library/queryutil.h"
#include "moc_relationdao.cpp"
#include "track/relation.h"
#include "util/db/fwdsqlquery.h"

namespace {

Relation* relationFromRow(const QSqlRecord& row) {
    const auto id = DbId(row.value(row.indexOf("rl_id")));
    TrackPair tracks = {
            TrackId(row.value(row.indexOf("track_a"))),
            TrackId(row.value(row.indexOf("track_b")))};
    std::array<QVariant, 2> qPositions = {
            row.value(row.indexOf("position_a")),
            row.value(row.indexOf("position_b"))};
    PositionPair positions = {
            qPositions[0].isNull()
                    ? std::nullopt
                    : std::optional<mixxx::audio::FramePos>(qPositions[0].toInt()),
            qPositions[1].isNull()
                    ? std::nullopt
                    : std::optional<mixxx::audio::FramePos>(qPositions[1].toInt())};
    QString comment = row.value(row.indexOf("rl_comment")).toString();
    QDateTime dateAdded = row.value(row.indexOf("rl_datetime_added")).toDateTime();

    Relation* relation = new Relation(
            id,
            tracks,
            positions,
            comment,
            dateAdded);
    return relation;
}

} // namespace

Relation* RelationDAO::getRelationById(DbId id) const {
    if (!id.isValid()) {
        return nullptr;
    }
    QSqlQuery query(m_database);
    QString queryText = QString("SELECT * FROM " RELATIONS_TABLE
                                " WHERE rl_id=:id");
    query.prepare(queryText);
    query.bindValue(":id", id.toVariant());
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return nullptr;
    }
    return relationFromRow(query.record());
}

QList<Relation*> RelationDAO::getRelations(TrackId trackId) const {
    QList<Relation*> relations;

    QSqlQuery query(m_database);
    QString queryText = QString("SELECT * FROM " RELATIONS_TABLE
                                " WHERE track_a=:id OR track_b=:id");
    query.prepare(queryText);
    query.bindValue(":id", trackId.toVariant());
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
                                     "track_a=:track_a,"
                                     "track_b=:track_b,"
                                     "position_a=:position_a,"
                                     "position_b=:position_b,"
                                     "rl_comment=:comment,"
                                     "rl_datetime_added=:datetime_added"
                                     " WHERE rl_id=:id"));
        query.bindValue(":id", relation->getId().toVariant());
    } else {
        // New relation
        query.prepare(QStringLiteral("INSERT INTO " RELATIONS_TABLE
                                     " (track_a, track_b, position_a, "
                                     "position_b, rl_comment) VALUES "
                                     "(:track_a, :track_b, "
                                     ":position_a, :position_b, "
                                     ":comment)"));
    }

    // Bind values and execute query
    TrackPair tracks = relation->getTracks();
    query.bindValue(":track_a", tracks[0].toVariant());
    query.bindValue(":track_b", tracks[1].toVariant());

    PositionPair positions = relation->getPositions();
    if (positions[0].has_value()) {
        query.bindValue(":position_a",
                positions[0]->toEngineSamplePos());
    } else {
        query.bindValue(":position_a", QVariant());
    }
    if (positions[1].has_value()) {
        query.bindValue(":position_b",
                positions[1]->toEngineSamplePos());
    } else {
        query.bindValue(":position_b", QVariant());
    }

    query.bindValue(":comment", relation->getComment());
    query.bindValue(":datetime_added", relation->getDateAdded());

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
    query.prepare(QStringLiteral("DELETE FROM " RELATIONS_TABLE " WHERE rl_id=:id"));
    query.bindValue(":id", relationId.toVariant());
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
    }
}
