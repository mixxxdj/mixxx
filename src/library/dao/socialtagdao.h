#ifndef SOCIALTAGDAO_H
#define SOCIALTAGDAO_H

#include <QObject>
#include <QSqlDatabase>

#include "lastfm/lastfmclient.h"

class SocialTagDao: public DAO {
  public:
    SocialTagDao(QSqlDatabase& database);
    ~SocialTagDao();

    void setDatabase(QSqlDatabase& database);
    LastFmClient::TagCounts getTagsForTrack(int trackId);
    bool setTagsForTrack(int trackId, LastFmClient::TagCounts tags);
    bool clearTagsForTrack(int trackId);

  private:
    LastFmClient::TagCounts loadTagsFromQuery(int trackId, QSqlQuery& query);
    static const QString m_sTagTableName;
    static const QString m_sTrackTagTableName;
    QSqlDatabase m_db;
};

#endif // SOCIALTAGDAO_H
