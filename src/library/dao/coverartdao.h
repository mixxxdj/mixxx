#ifndef COVERARTDAO_H
#define COVERARTDAO_H

#include <QObject>
#include <QSqlDatabase>

#include "library/dao/dao.h"
#include "trackinfoobject.h"

class CoverArtDAO : public QObject, public virtual DAO {
    Q_OBJECT
  public:
    CoverArtDAO(QSqlDatabase& database);
    virtual ~CoverArtDAO();
    void setDatabase(QSqlDatabase& database) { m_database = database; }

    void initialize();

    void saveCoverArt(TrackInfoObject*, ConfigObject<ConfigValue>*);

   private:
    QSqlDatabase& m_database;
};

#endif // COVERARTDAO_H
