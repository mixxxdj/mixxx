#ifndef SOCIALTAGDAO_H
#define SOCIALTAGDAO_H

#include <QObject>
#include <QSqlDatabase>

#include "lastfm/lastfmclient.h"

#define TAG_TABLE "social_tags"
#define TRACK_TAG_TABLE "track_social_tags"

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
    QSqlDatabase m_db;
};

#endif // SOCIALTAGDAO_H
