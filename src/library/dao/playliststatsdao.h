#pragma once

#include <QList>
#include <QObject>

#include "library/dao/dao.h"
#include "track/trackid.h"
#include "util/class.h"

class PlaylistStatsDAO : public QObject, public virtual DAO {
    Q_OBJECT
  public:
    PlaylistStatsDAO(const QString& countsDurationTableName);
    ~PlaylistStatsDAO() override = default;

    void initialize(const QSqlDatabase& database) override;

    struct PlaylistSummary {
        PlaylistSummary()
                : hasValue(false),
                  name(),
                  count(0),
                  duration(0) {
        }
        PlaylistSummary(const int playlistId,
                const QString& name,
                int count,
                int duration)
                : hasValue(true),
                  playlistId(playlistId),
                  name(name),
                  count(count),
                  duration(duration) {
        }

        bool isEmpty() const {
            return !hasValue;
        }

        bool hasValue;
        int playlistId;
        QString name;
        int count;
        int duration;
    };

    /// Prepares the table view used by getPlaylistSummary.
    void preparePlaylistSummaryTable();

    /// Returns the name, track count and total duration of all playlists.
    QList<PlaylistSummary> getPlaylistSummaries();

    /// Returns the name, track count and total duration of the given playlist.
    PlaylistSummary getPlaylistSummary(const int playlistId);

  private:
    QString m_countsDurationTableName;
    DISALLOW_COPY_AND_ASSIGN(PlaylistStatsDAO);
};
