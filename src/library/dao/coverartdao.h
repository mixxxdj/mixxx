#ifndef COVERARTDAO_H
#define COVERARTDAO_H

#include <QObject>
#include <QSqlDatabase>

#include "library/coverart.h"
#include "library/dao/dao.h"

class CoverArtDAO : public QObject, public virtual DAO {
    Q_OBJECT
  public:
    CoverArtDAO(QSqlDatabase& database);
    virtual ~CoverArtDAO();
    void setDatabase(QSqlDatabase& database) { m_database = database; }

    void initialize();

    void deleteUnusedCoverArts();
    int getCoverArtId(QString coverLocation);
    QString getCoverArtLocation(int id, bool fromTrackId=false);
    QString getDefaultCoverLocation(int trackId);
    int saveCoverLocation(QString coverLocation);

  private slots:
    void slotCoverArtScan(int trackId);

  private:
    QSqlDatabase& m_database;
    CoverArt* m_pCoverArt;

    TrackPointer getTrackFromDB(int trackId);
    bool updateLibrary(int trackId, int coverId);
};

#endif // COVERARTDAO_H
