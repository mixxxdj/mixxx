#include "library/baseexternalplaylistmodel.h"

#include "library/queryutil.h"
#include "playermanager.h"

BaseExternalPlaylistModel::BaseExternalPlaylistModel(QObject* parent,
                                                     TrackCollection* pTrackCollection,
                                                     const char* settingsNamespace,
                                                     const QString& playlistsTable,
                                                     const QString& playlistTracksTable,
                                                     QSharedPointer<BaseTrackCache> trackSource)
        : BaseSqlTableModel(parent, pTrackCollection,
                            settingsNamespace),
          m_playlistsTable(playlistsTable),
          m_playlistTracksTable(playlistTracksTable),
          m_trackSource(trackSource) {
}

BaseExternalPlaylistModel::~BaseExternalPlaylistModel() {
}

TrackPointer BaseExternalPlaylistModel::getTrack(const QModelIndex& index) const {
    QString location = index.sibling(
            index.row(), fieldIndex("location")).data().toString();

    if (location.isEmpty()) {
        // Track is lost
        return TrackPointer();
    }

    bool track_already_in_library = false;
    TrackPointer pTrack = m_pTrackCollection->getTrackDAO()
            .getOrAddTrack(location, true, &track_already_in_library);

    // If this track was not in the Mixxx library it is now added and will be
    // saved with the metadata from iTunes. If it was already in the library
    // then we do not touch it so that we do not over-write the user's metadata.
    if (pTrack && !track_already_in_library) {
        QString artist = index.sibling(
                index.row(), fieldIndex("artist")).data().toString();
        pTrack->setArtist(artist);

        QString title = index.sibling(
                index.row(), fieldIndex("title")).data().toString();
        pTrack->setTitle(title);

        QString album = index.sibling(
                index.row(), fieldIndex("album")).data().toString();
        pTrack->setAlbum(album);

        QString year = index.sibling(
                index.row(), fieldIndex("year")).data().toString();
        pTrack->setYear(year);

        QString genre = index.sibling(
                index.row(), fieldIndex("genre")).data().toString();
        pTrack->setGenre(genre);

        float bpm = index.sibling(
                index.row(), fieldIndex("bpm")).data().toString().toFloat();
        pTrack->setBpm(bpm);
    }
    return pTrack;
}

bool BaseExternalPlaylistModel::isColumnInternal(int column) {
    if (column == fieldIndex(ColumnCache::COLUMN_PLAYLISTTRACKSTABLE_TRACKID) ||
            (PlayerManager::numPreviewDecks() == 0 &&
             column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_PREVIEW))) {
        return true;
    }
    return false;
}

Qt::ItemFlags BaseExternalPlaylistModel::flags(const QModelIndex &index) const {
    return readOnlyFlags(index);
}

void BaseExternalPlaylistModel::setPlaylist(QString playlist_path) {
    QSqlQuery finder_query(m_database);
    finder_query.prepare(QString("SELECT id from %1 where name=:name").arg(m_playlistsTable));
    finder_query.bindValue(":name", playlist_path);

    if (!finder_query.exec()) {
        LOG_FAILED_QUERY(finder_query) << "Error getting id for playlist:" << playlist_path;
        return;
    }

    // TODO(XXX): Why not last-insert id?
    int playlistId = -1;
    QSqlRecord finder_query_record = finder_query.record();
    while (finder_query.next()) {
        playlistId = finder_query.value(
                finder_query_record.indexOf("id")).toInt();
    }

    if (playlistId == -1) {
        qDebug() << "ERROR: Could not get the playlist ID for playlist:" << playlist_path;
        return;
    }

    QString playlistViewTable = QString("%1_%2").arg(m_playlistTracksTable,
                                                     QString::number(playlistId));

    QStringList columns;
    columns << "track_id";
    columns << "position";

    QSqlQuery query(m_database);
    FieldEscaper f(m_database);
    QString queryString = QString(
        "CREATE TEMPORARY VIEW IF NOT EXISTS %1 AS "
        "SELECT %2 FROM %3 WHERE playlist_id = %4")
            .arg(f.escapeString(playlistViewTable),
                 columns.join(","),
                 m_playlistTracksTable,
                 QString::number(playlistId));
    query.prepare(queryString);

    if (!query.exec()) {
        LOG_FAILED_QUERY(query) << "Error creating temporary view for playlist.";
        return;
    }

    setTable(playlistViewTable, columns[0], columns, m_trackSource);
    setDefaultSort(fieldIndex(ColumnCache::COLUMN_PLAYLISTTRACKSTABLE_POSITION),
                   Qt::AscendingOrder);
    setSearch("");
}

TrackModel::CapabilitiesFlags BaseExternalPlaylistModel::getCapabilities() const {
    // See src/library/trackmodel.h for the list of TRACKMODELCAPS
    return TRACKMODELCAPS_NONE
            | TRACKMODELCAPS_ADDTOPLAYLIST
            | TRACKMODELCAPS_ADDTOCRATE
            | TRACKMODELCAPS_ADDTOAUTODJ
            | TRACKMODELCAPS_LOADTODECK
            | TRACKMODELCAPS_LOADTOPREVIEWDECK
            | TRACKMODELCAPS_LOADTOSAMPLER;
}
