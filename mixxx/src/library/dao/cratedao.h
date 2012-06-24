// cratedao.h
// Created 10/22/2009 by RJ Ryan (rryan@mit.edu)

#ifndef CRATEDAO_H
#define CRATEDAO_H

#include <QObject>
#include <QSqlDatabase>

#include "library/dao/dao.h"
#include "util.h"

#define CRATE_TABLE "crates"
#define CRATE_TRACKS_TABLE "crate_tracks"

const QString CRATETRACKSTABLE_TRACKID = "track_id";
const QString CRATETRACKSTABLE_CRATEID = "crate_id";

class CrateDAO : public QObject, public virtual DAO {
    Q_OBJECT
  public:
    CrateDAO(QSqlDatabase& database);
    virtual ~CrateDAO();

    void setDatabase(QSqlDatabase& database) { m_database = database; };

    // Initialize this DAO, create the tables it relies on, etc.
    void initialize();

    unsigned int crateCount();
    int createCrate(const QString& name);
    bool deleteCrate(int crateId);
    bool renameCrate(int crateId, const QString& newName);
    bool setCrateLocked(int crateId, bool locked);
    bool isCrateLocked(int crateId);
    int getCrateIdByName(const QString& name);
    int getCrateId(int position);
    QString crateName(int crateId);
    unsigned int crateSize(int crateId);
    bool addTrackToCrate(int trackId, int crateId);
    // This method takes a list of track ids to be added to crate and returns
    // the number of successful insertions.
    int addTracksToCrate(QList<int> trackIdList, int crateId);
    bool removeTrackFromCrate(int trackId, int crateId);
    // remove tracks from all crates
    void removeTracksFromCrates(QList<int> ids);

  signals:
    void added(int crateId);
    void deleted(int crateId);
    void changed(int crateId);
    void trackAdded(int crateId, int trackId);
    void trackRemoved(int crateId, int trackId);
    void renamed(int crateId);
    void lockChanged(int crateId);

  private:
    QSqlDatabase& m_database;
    DISALLOW_COPY_AND_ASSIGN(CrateDAO);
};

#endif /* CRATEDAO_H */
