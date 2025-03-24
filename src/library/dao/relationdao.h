#pragma once

#include <QObject>

#include "library/dao/dao.h"
#include "track/relation.h"
#include "track/trackid.h"

#define RELATIONS_TABLE "relations"

class Relation;

class RelationDAO : public DAO {
  public:
    ~RelationDAO() override = default;

    QList<Relation*> getRelations(
            TrackId trackId,
            bool trackIsSource = true,
            bool trackIsTarget = true,
            std::optional<bool> bidirectional = std::nullopt) const;

    void saveRelation(Relation* relation);
    void deleteRelation(DbId relationId);
};
