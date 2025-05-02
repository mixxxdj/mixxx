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

    Relation* getRelationById(DbId id) const;
    QList<Relation*> getRelations(TrackId trackId) const;

    void saveRelation(Relation* relation);
    void deleteRelation(DbId relationId);
};
