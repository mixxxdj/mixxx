#ifndef COVERARTDAO_H
#define COVERARTDAO_H

#include <QObject>
#include <QSqlDatabase>

#include "library/dao/dao.h"
#include "trackinfoobject.h"

class CoverArtDAO : public QObject, public virtual DAO {
    Q_OBJECT
  public:
    CoverArtDAO(QSqlDatabase& database, ConfigObject<ConfigValue> *pConfig);
    virtual ~CoverArtDAO();
    void setDatabase(QSqlDatabase& database) { m_database = database; }

    void initialize();

    void saveCoverArt(TrackInfoObject*);
    int getCoverArtID(QString location);
    QString getCoverArtLocation(int id);

   private:
    ConfigObject<ConfigValue>* m_pConfig;
    QSqlDatabase& m_database;

    QString getStoragePath() const;
    QString searchCoverArtFile(TrackInfoObject* pTrack);
};

#endif // COVERARTDAO_H
