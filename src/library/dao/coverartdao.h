#ifndef COVERARTDAO_H
#define COVERARTDAO_H

#include <QObject>
#include <QSqlDatabase>

#include "library/coverart.h"
#include "library/dao/dao.h"
#include "trackinfoobject.h"

class CoverArtDAO : public QObject, public virtual DAO {
    Q_OBJECT
  public:
    CoverArtDAO(QSqlDatabase& database);
    virtual ~CoverArtDAO();
    void setDatabase(QSqlDatabase& database) { m_database = database; }

    void initialize();

    bool deleteCoverArt(const int coverId);
    bool deleteUnusedCoverArts();
    int getCoverArtId(QString coverLocation);
    QString getCoverArtLocation(int id);
    int saveCoverLocation(QString coverLocation);

  private slots:
    void slotCoverArtScan(TrackPointer pTrack);

  private:
    QSqlDatabase& m_database;
    CoverArt* m_pCoverArt;
};

#endif // COVERARTDAO_H
