#pragma once

#include <QDateTime>
#include <QList>
#include <QObject>

#include "library/dao/dao.h"
#include "library/dao/playlistdao.h"
#include "track/trackid.h"
#include "util/class.h"

class PlaylistStatsDAO : public QObject, public virtual DAO {
    Q_OBJECT
  public:
    PlaylistStatsDAO();
    ~PlaylistStatsDAO() override = default;

    void initialize(const QSqlDatabase& database) override;

    struct PlaylistSummary {
        PlaylistSummary()
                : hasValue(false),
                  name(),
                  dateCreated(),
                  count(0),
                  duration(0) {
        }
        PlaylistSummary(const int playlistId,
                const QString& name,
                const QDateTime& dateCreated,
                int count,
                int duration)
                : hasValue(true),
                  playlistId(playlistId),
                  name(name),
                  dateCreated(dateCreated),
                  count(count),
                  duration(duration) {
        }

        bool isEmpty() const {
            return !hasValue;
        }

        bool hasValue;
        int playlistId;
        QString name;
        QDateTime dateCreated;
        int count;
        int duration;
    };

    /// Prepares the table view used by getPlaylistSummary.
    void preparePlaylistSummaryTable();

    /// Returns the name, track count and total duration
    /// of all playlists of the given type.
    QList<PlaylistSummary> getPlaylistSummaries(PlaylistDAO::HiddenType playlistType);

    /// Returns the name, track count and total duration of the given playlist.
    PlaylistSummary getPlaylistSummary(const int playlistId);

  private:
    QString m_countsDurationTableName;
    DISALLOW_COPY_AND_ASSIGN(PlaylistStatsDAO);
};
