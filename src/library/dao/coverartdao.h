#ifndef COVERARTDAO_H
#define COVERARTDAO_H

#include <QObject>
#include <QSqlDatabase>

#include "configobject.h"
#include "library/dao/dao.h"

const QString COVERART_TABLE = "cover_art";
const QString COVERARTTABLE_ID = "id";
const QString COVERARTTABLE_LOCATION = "location";

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
    int saveCoverLocation(QString coverLocation);

    struct coverArtInfo {
        int trackId;
        QString currentCoverLocation;
        QString album;
        QString trackFilename;
        QString trackDirectory;
        QString trackLocation;
    };

    coverArtInfo getCoverArtInfo(int trackId);

  private:
    QSqlDatabase& m_database;
    ConfigObject<ConfigValue>* m_pConfig;
};

#endif // COVERARTDAO_H
