#include "library/dao/playliststatsdao.h"

#include <QtDebug>

#include "library/dao/playlistdao.h"
#include "library/queryutil.h"
#include "moc_playliststatsdao.cpp"
#include "util/db/dbconnection.h"
#include "util/db/fwdsqlquery.h"
#include "util/make_const_iterator.h"
#include "util/math.h"

PlaylistStatsDAO::PlaylistStatsDAO(const QString& countsDurationTableName)
        : m_countsDurationTableName(countsDurationTableName) {
}

void PlaylistStatsDAO::initialize(const QSqlDatabase& database) {
    DAO::initialize(database);
}

void PlaylistStatsDAO::preparePlaylistSummaryTable() {
    QString queryString = QStringLiteral(
            "CREATE TEMPORARY VIEW IF NOT EXISTS %1 "
            "AS SELECT "
            "  Playlists.id AS id, "
            "  Playlists.name AS name, "
            "  LOWER(Playlists.name) AS sort_name, "
            "  COUNT(case library.mixxx_deleted when 0 then 1 else null end) "
            "    AS count, "
            "  SUM(case library.mixxx_deleted "
            "    when 0 then library.duration else 0 end) AS durationSeconds "
            "FROM Playlists "
            "LEFT JOIN PlaylistTracks "
            "  ON PlaylistTracks.playlist_id = Playlists.id "
            "LEFT JOIN library "
            "  ON PlaylistTracks.track_id = library.id "
            "  WHERE Playlists.hidden = %2 "
            "  GROUP BY Playlists.id")
                                  .arg(m_countsDurationTableName,
                                          QString::number(
                                                  PlaylistDAO::PLHT_NOT_HIDDEN));
    queryString.append(
            mixxx::DbConnection::collateLexicographically(
                    " ORDER BY sort_name"));
    QSqlQuery query(m_database);
    if (!query.exec(queryString)) {
        LOG_FAILED_QUERY(query);
    }
}

QList<PlaylistStatsDAO::PlaylistSummary> PlaylistStatsDAO::getPlaylistSummaries() {
    // Setup the sidebar playlist model
    QSqlTableModel playlistTableModel(this, m_database);
    playlistTableModel.setTable(m_countsDurationTableName);
    playlistTableModel.select();
    while (playlistTableModel.canFetchMore()) {
        playlistTableModel.fetchMore();
    }
    QSqlRecord record = playlistTableModel.record();
    int nameColumn = record.indexOf("name");
    int idColumn = record.indexOf("id");
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
        int count =
                playlistTableModel
                        .data(playlistTableModel.index(row, countColumn))
                        .toInt();
        int duration =
                playlistTableModel
                        .data(playlistTableModel.index(row, durationColumn))
                        .toInt();

        playlistSummaries.append(PlaylistSummary(id, name, count, duration));
    }
    return playlistSummaries;
}
