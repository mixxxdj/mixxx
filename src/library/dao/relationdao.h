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

    RelationPointer getRelationById(DbId id) const;
    QList<RelationPointer> getRelations(TrackId trackId) const;

    void saveRelation(RelationPointer relation);
    void deleteRelation(DbId relationId);
};
