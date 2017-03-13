// cratedao.h
// Created 10/22/2009 by RJ Ryan (rryan@mit.edu)

#ifndef CRATEDAO_H
#define CRATEDAO_H

#include <QObject>
#include <QMap>
#include <QMultiHash>
#include <QSqlDatabase>
#include <QSet>

#include "library/dao/dao.h"
#include "util.h"

#define CRATE_TABLE "crates"
#define CRATE_TRACKS_TABLE "crate_tracks"

const QString CRATETABLE_ID = "id";
const QString CRATETABLE_NAME = "name";
const QString CRATETABLE_COUNT = "count";
const QString CRATETABLE_SHOW = "show";
const QString CRATETABLE_LOCKED = "locked";
const QString CRATETABLE_AUTODJ_SOURCE = "autodj_source";

const QString CRATETRACKSTABLE_TRACKID = "track_id";
const QString CRATETRACKSTABLE_CRATEID = "crate_id";

class CrateDAO : public QObject, public virtual DAO {
    Q_OBJECT
  public:
    CrateDAO(QSqlDatabase& database);
    virtual ~CrateDAO();

    void setDatabase(QSqlDatabase& database) { m_database = database; }

    // Initialize this DAO, create the tables it relies on, etc.
    void initialize();

    unsigned int crateCount();
    int createCrate(const QString& name);
    bool deleteCrate(const int crateId);
    bool renameCrate(const int crateId, const QString& newName);
    bool setCrateLocked(const int crateId, const bool locked);
    bool isCrateLocked(const int crateId);
    #ifdef __AUTODJCRATES__
    bool setCrateInAutoDj(int crateId, bool a_bIn);
    bool isCrateInAutoDj(int crateId);
    QList<int> getCrateTracks(int crateId);
    void getAutoDjCrates(bool trackSource, QMap<QString,int>* pCrateMap);
    #endif // __AUTODJCRATES__
    int getCrateIdByName(const QString& name);
    int getCrateId(const int position);
    QString crateName(const int crateId);
    unsigned int crateSize(const int crateId);
    bool addTrackToCrate(const int trackId, const int crateId);
    QList<int> getTrackIds(int crateId);
    // This method takes a list of track ids to be added to crate and returns
    // the number of successful insertions.
    int addTracksToCrate(const int crateId, QList<int>* trackIdList);
    void copyCrateTracks(const int sourceCrateId, const int tragetCrateId);
    bool removeTrackFromCrate(const int trackId, const int crateId);
    bool removeTracksFromCrate(const QList<int>& ids, const int crateId);
    // remove tracks from all crates
    void removeTracksFromCrates(const QList<int>& ids);
    bool isTrackInCrate(const int trackId, const int crateId);
    void getCratesTrackIsIn(const int trackId, QSet<int>* crateSet) const;

  signals:
    void added(int crateId);
    void renamed(int crateId, QString a_strName);
    void deleted(int crateId);
    void changed(int crateId);
    void trackAdded(int crateId, int trackId);
    void trackRemoved(int crateId, int trackId);
    void lockChanged(int crateId);
    void autoDjChanged(int a_iCrateId, bool a_bIn);

  private:
    void populateCrateMembershipCache();

    QSqlDatabase& m_database;
    QMultiHash<int,int> m_cratesTrackIsIn;
    DISALLOW_COPY_AND_ASSIGN(CrateDAO);
};

#endif /* CRATEDAO_H */
