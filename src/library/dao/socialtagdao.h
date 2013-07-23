#ifndef SOCIALTAGDAO_H
#define SOCIALTAGDAO_H

#include <QObject>
#include <QSqlDatabase>

#include "library/dao/dao.h"
#include "trackinfoobject.h"
#include "track/tagutils.h"

#include "lastfm/lastfmclient.h"


#define TAG_TABLE "social_tags"
#define TRACK_TAG_TABLE "track_social_tags"

class SocialTagDao : public QObject, public virtual DAO {
    Q_OBJECT
  public:
    SocialTagDao(QSqlDatabase& database);
    ~SocialTagDao();

    void initialize();
    void setDatabase(QSqlDatabase& database);
    TagCounts getTagsForTrack(int trackId);
    void saveTrackTags(int trackId, TrackInfoObject* pTrack);
    bool setTagsForTrack(int trackId, TagCounts tags);
    bool clearTagsForTrack(int trackId);
    bool clearTagsForTracks(QList<int> trackIds);

  private:
    TagCounts loadTagsFromQuery(int trackId, QSqlQuery& query);
    QSqlDatabase m_db;
};

#endif // SOCIALTAGDAO_H
