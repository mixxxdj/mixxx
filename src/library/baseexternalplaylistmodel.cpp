#include "library/baseexternalplaylistmodel.h"

#include "library/queryutil.h"
#include "playermanager.h"

BaseExternalPlaylistModel::BaseExternalPlaylistModel(
    QObject* parent, TrackCollection* pTrackCollection,
    QString settingsNamespace, QString playlistsTable,
    QString playlistTracksTable, QString trackSource)
        : BaseSqlTableModel(parent, pTrackCollection,
                            settingsNamespace),
          m_playlistsTable(playlistsTable),
          m_playlistTracksTable(playlistTracksTable),
          m_trackSource(trackSource) {
}

BaseExternalPlaylistModel::~BaseExternalPlaylistModel() {
}

void BaseExternalPlaylistModel::setTableModel(int id){
    Q_UNUSED(id);
}

// Must be called from Main thread
TrackPointer BaseExternalPlaylistModel::getTrack(const QModelIndex& index) const {
    QString artist = index.sibling(
        index.row(), fieldIndex("artist")).data().toString();
    QString title = index.sibling(
        index.row(), fieldIndex("title")).data().toString();
    QString album = index.sibling(
        index.row(), fieldIndex("album")).data().toString();
    QString year = index.sibling(
        index.row(), fieldIndex("year")).data().toString();
    QString genre = index.sibling(
        index.row(), fieldIndex("genre")).data().toString();
    float bpm = index.sibling(
        index.row(), fieldIndex("bpm")).data().toString().toFloat();
    QString location = index.sibling(
        index.row(), fieldIndex("location")).data().toString();

    if (location.isEmpty()) {
        // Track is lost
        return TrackPointer();
    }

    TrackDAO& track_dao = m_pTrackCollection->getTrackDAO();
    bool track_already_in_library = false;
    int track_id = -1;
    // tro's lambda idea. This code calls synchronously!
    m_pTrackCollection->callSync(
                [this, &location, &track_dao, &track_already_in_library, &track_id] (void) {
        track_id = track_dao.getTrackId(location);
        track_already_in_library = track_id >= 0;
        if (track_id < 0) {
            // Add Track to library
            track_id = track_dao.addTrack(location, true);
        }
    }, __PRETTY_FUNCTION__);

    TrackPointer pTrack;
    if (track_id < 0) {
        // Add Track to library failed, create a transient TrackInfoObject
        pTrack = TrackPointer(new TrackInfoObject(location), &QObject::deleteLater);
    } else {
        // tro's lambda idea. This code calls synchronously!
        m_pTrackCollection->callSync(
                    [this, &track_dao, &track_id, &pTrack] (void) {
            pTrack = track_dao.getTrack(track_id);
        });
    }

    // If this track was not in the Mixxx library it is now added and will be
    // saved with the metadata from iTunes. If it was already in the library
    // then we do not touch it so that we do not over-write the user's metadata.
    if (!track_already_in_library) {
        pTrack->setArtist(artist);
        pTrack->setTitle(title);
        pTrack->setAlbum(album);
        pTrack->setYear(year);
        pTrack->setGenre(genre);
        pTrack->setBpm(bpm);
    }
    return pTrack;
}

bool BaseExternalPlaylistModel::isColumnInternal(int column) {
    if (column == fieldIndex("track_id") ||
        (PlayerManager::numPreviewDecks() == 0 && column == fieldIndex("preview"))) {
        return true;
    }
    return false;
}

Qt::ItemFlags BaseExternalPlaylistModel::flags(const QModelIndex &index) const {
    return readOnlyFlags(index);
}

// Must be called from Main thread
bool BaseExternalPlaylistModel::setPlaylist(QString playlist_path) {
    bool result = false;
    QString playlistViewTable;
    QStringList columns;
    // tro's lambda idea. This code calls synchronously!
    m_pTrackCollection->callSync(
                [this, &playlist_path, &playlistViewTable, &columns, &result] (void) {
        QSqlQuery finder_query(m_pTrackCollection->getDatabase());
        finder_query.prepare(QString("SELECT id from %1 where name=:name").arg(m_playlistsTable));
        finder_query.bindValue(":name", playlist_path);

        if (!finder_query.exec()) {
            LOG_FAILED_QUERY(finder_query) << "Error getting id for playlist:" << playlist_path;
            result = false;
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
            result = false;
            return;
        }

        playlistViewTable = QString("%1_%2").arg(m_playlistTracksTable,
                                                         QString::number(playlistId));
        columns << "track_id";
        columns << "position";

        QSqlQuery query(m_pTrackCollection->getDatabase());
        FieldEscaper f(m_pTrackCollection->getDatabase());
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
            result = false;
            return;
        }

        result = true;
    }, __PRETTY_FUNCTION__);

    if (result) {
        setTable(playlistViewTable, columns[0], columns,
                 m_pTrackCollection->getTrackSource(m_trackSource));
        setSearch("");
    }
    return result;
}

void BaseExternalPlaylistModel::setPlaylistUI() {
    setDefaultSort(fieldIndex("position"), Qt::AscendingOrder);
    initHeaderData();
}

bool BaseExternalPlaylistModel::isColumnHiddenByDefault(int column) {
    Q_UNUSED(column);
    return false;
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
