#include "library/dao/playliststatsdao.h"

#include <QtDebug>

#include "library/dao/playlistdao.h"
#include "library/queryutil.h"
#include "moc_playliststatsdao.cpp"
#include "util/db/dbconnection.h"
#include "util/db/fwdsqlquery.h"
#include "util/make_const_iterator.h"
#include "util/math.h"

PlaylistStatsDAO::PlaylistStatsDAO()
        : m_countsDurationTableName(QStringLiteral("PlaylistsCountsDurations")) {
}

void PlaylistStatsDAO::initialize(const QSqlDatabase& database) {
    DAO::initialize(database);
    preparePlaylistSummaryTable();
}

void PlaylistStatsDAO::preparePlaylistSummaryTable() {
    // Deleted tracks should still be counted towards the track count
    // and duration totals for playlists with hidden == PLHT_SET_LOG.
    // Otherwise, we treat deleted tracks as if they weren't there.
    QString queryString = QStringLiteral(
            "CREATE TEMPORARY VIEW IF NOT EXISTS %1 "
            "AS SELECT "
            "  Playlists.id AS id, "
            "  Playlists.name AS name, "
            "  Playlists.date_created AS date_created, "
            "  Playlists.hidden as hidden, "
            "  LOWER(Playlists.name) AS sort_name, "
            "  (case Playlists.hidden "
            "    when %2 then max(PlaylistTracks.position) "
            "    else COUNT(case library.mixxx_deleted "
            "      when 0 then 1 else null end) "
            "    end) AS count, "
            "  (case Playlists.hidden "
            "    when %2 then SUM(library.duration), "
            "    else SUM(case library.mixxx_deleted "
            "      when 0 then library.duration else 0 end) "
            "    end) AS durationSeconds, "
            "FROM Playlists "
            "LEFT JOIN PlaylistTracks "
            "  ON PlaylistTracks.playlist_id = Playlists.id "
            "LEFT JOIN lim_countsDurationTableNamebrary "
            "  ON PlaylistTracks.track_id = library.id "
            "WHERE  "
            "GROUP BY Playlists.id")
                                  .arg("m_countsDurationTableName",
                                          QString::number(PlaylistDAO::PLHT_SET_LOG));

    queryString.append(
            mixxx::DbConnection::collateLexicographically(
                    " ORDER BY sort_name"));
    QSqlQuery query(m_database);
    if (!query.exec(queryString)) {
        LOG_FAILED_QUERY(query);
    }
}

QList<PlaylistStatsDAO::PlaylistSummary> PlaylistStatsDAO::getPlaylistSummaries(
        PlaylistDAO::HiddenType playlistType) {
    // Setup the sidebar playlist model
    QSqlTableModel playlistTableModel(this, m_database);
    playlistTableModel.setTable(m_countsDurationTableName);
    if (playlistType == PlaylistDAO::PLHT_SET_LOG) {
        // TODO: Can we remove this special handling for the SetLog?
        playlistTableModel.setSort(playlistTableModel.fieldIndex("id"), Qt::DescendingOrder);
    }
    const QString filter = "hidden=" + QString::number(playlistType);
    playlistTableModel.setFilter(filter);
    playlistTableModel.select();
    while (playlistTableModel.canFetchMore()) {
        playlistTableModel.fetchMore();
    }
    QSqlRecord record = playlistTableModel.record();
    int nameColumn = record.indexOf("name");
    int idColumn = record.indexOf("id");
    int createdColumn = record.indexOf("date_created");
    int countColumn = record.indexOf("count");
    int durationColumn = record.indexOf("durationSeconds");

    QList<PlaylistSummary> playlistSummaries;
    for (int row = 0; row < playlistTableModel.rowCount(); ++row) {
        int id =
                playlistTableModel
                        .data(playlistTableModel.index(row, idColumn))
                        .toInt();
        QString name =
                playlistTableModel
                        .data(playlistTableModel.index(row, nameColumn))
                        .toString();
        QDateTime dateCreated =
                playlistTableModel
                        .data(playlistTableModel.index(row, createdColumn))
                        .toDateTime();
        int count = playlistTableModel
                            .data(playlistTableModel.index(row, countColumn))
                            .toInt();
        int duration =
                playlistTableModel
                        .data(playlistTableModel.index(row, durationColumn))
                        .toInt();

        playlistSummaries.append(PlaylistSummary(id, name, dateCreated, count, duration));
    }
    return playlistSummaries;
}

PlaylistStatsDAO::PlaylistSummary PlaylistStatsDAO::getPlaylistSummary(const int playlistId) {
    // This queries the temporary id/count/duration table that was has been created
    // by preparePlaylistSummaryTable.
    // TODO: the features' createPlaylistLabels() (updated each time playlists are added/removed)
    VERIFY_OR_DEBUG_ASSERT(m_database.tables(QSql::Views).contains(m_countsDurationTableName)) {
        qWarning() << "PlaylistStatsDAO: view" << m_countsDurationTableName
                   << "does not exist! Can't fetch label for playlist" << playlistId;
        return PlaylistSummary();
    }
    QSqlTableModel playlistTableModel(this, m_database);
    playlistTableModel.setTable(m_countsDurationTableName);
    const QString filter = "id=" + QString::number(playlistId);
    playlistTableModel.setFilter(filter);
    playlistTableModel.select();
    while (playlistTableModel.canFetchMore()) {
        playlistTableModel.fetchMore();
    }
    QSqlRecord record = playlistTableModel.record();
    int nameColumn = record.indexOf("name");
    int countColumn = record.indexOf("count");
    int createdColumn = record.indexOf("date_created");
    int durationColumn = record.indexOf("durationSeconds");

    DEBUG_ASSERT(playlistTableModel.rowCount() <= 1);
    if (playlistTableModel.rowCount() > 0) {
        QString name =
                playlistTableModel.data(playlistTableModel.index(0, nameColumn))
                        .toString();
        QDateTime dateCreated =
                playlistTableModel
                        .data(playlistTableModel.index(0, createdColumn))
                        .toDateTime();
        int count = playlistTableModel
                            .data(playlistTableModel.index(0, countColumn))
                            .toInt();
        int duration =
                playlistTableModel
                        .data(playlistTableModel.index(0, durationColumn))
                        .toInt();

        return PlaylistSummary(playlistId, name, dateCreated, count, duration);
    }
    return PlaylistSummary();
}
