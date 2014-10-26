#ifndef COVERARTDAO_H
#define COVERARTDAO_H

#include <QSqlDatabase>

#include "configobject.h"
#include "library/dao/dao.h"

const QString COVERART_TABLE = "cover_art";
const QString COVERARTTABLE_ID = "id";
const QString COVERARTTABLE_LOCATION = "location";
const QString COVERARTTABLE_HASH = "hash";

class CoverArtDAO : public DAO {
  public:
    CoverArtDAO(QSqlDatabase& database);
    virtual ~CoverArtDAO();

    void setDatabase(QSqlDatabase& database) { m_database = database; }
    void initialize();

    void deleteUnusedCoverArts();
    int getCoverArtId(QString coverHash);
    int saveCoverArt(QString coverLocation, QString coverHash);

    // @param covers: <trackId, <coverLoc, hash>>
    // @return <trackId, coverId>
    QSet<QPair<int, int> > saveCoverArt(QHash<int, QPair<QString, QString> > covers);

    struct CoverArtInfo {
        int trackId;
        QString coverLocation;
        QString hash;
        QString album;
        QString trackDirectory;
        QString trackLocation;
    };

    CoverArtInfo getCoverArtInfo(int trackId);

  private:
    QSqlDatabase& m_database;
};

#endif // COVERARTDAO_H
