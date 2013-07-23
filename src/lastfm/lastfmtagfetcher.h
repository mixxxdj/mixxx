#ifndef LASTFMTAGFETCHER_H
#define LASTFMTAGFETCHER_H

#include <QObject>
#include "trackinfoobject.h"
#include "lastfm/lastfmclient.h"
#include "track/tagutils.h"

class LastFmTagFetcher : public QObject {
  Q_OBJECT

  // High-level interface to LastFmClient.
  public:
    LastFmTagFetcher(QObject *parent = 0);
    void startFetch(const TrackPointer track);

  public slots:
    void cancel();
//    void fetchProgress(QString);

  private slots:
    void tagsFetched(int index, const TagCounts& tags);

  private:
    LastFmClient m_LastFmClient;
    QList<TrackPointer> m_tracks;
};

#endif // LASTFMTAGFETCHER_H
