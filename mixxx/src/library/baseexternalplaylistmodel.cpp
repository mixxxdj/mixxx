#include "library/baseexternalplaylistmodel.h"

#include "library/queryutil.h"

BaseExternalPlaylistModel::BaseExternalPlaylistModel(
    QObject* parent, TrackCollection* pTrackCollection,
    QString settingsNamespace, QString playlistsTable,
    QString playlistTracksTable, QString trackSource)
        : BaseSqlTableModel(parent, pTrackCollection,
                            pTrackCollection->getDatabase(),
                            settingsNamespace),
          m_playlistsTable(playlistsTable),
          m_playlistTracksTable(playlistTracksTable),
          m_trackSource(trackSource),
          m_pTrackCollection(pTrackCollection),
          m_database(m_pTrackCollection->getDatabase()) {
    connect(this, SIGNAL(doSearch(const QString&)),
            this, SLOT(slotSearch(const QString&)));
}

BaseExternalPlaylistModel::~BaseExternalPlaylistModel() {
}

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
    int track_id = track_dao.getTrackId(location);
    bool track_already_in_library = track_id >= 0;
    if (track_id < 0) {
        // Add Track to library
        track_id = track_dao.addTrack(location, true);
    }

    TrackPointer pTrack;

    if (track_id < 0) {
        // Add Track to library failed, create a transient TrackInfoObject
        pTrack = TrackPointer(new TrackInfoObject(location), &QObject::deleteLater);
    } else {
        pTrack = track_dao.getTrack(track_id);
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

void BaseExternalPlaylistModel::search(const QString& searchText) {
    // qDebug() << "BaseExternalPlaylistModel::search()" << searchText
    //          << QThread::currentThread();
    emit(doSearch(searchText));
}

void BaseExternalPlaylistModel::slotSearch(const QString& searchText) {
    BaseSqlTableModel::search(searchText);
}

bool BaseExternalPlaylistModel::isColumnInternal(int column) {
    if (column == fieldIndex("track_id")) {
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
    while (finder_query.next()) {
        playlistId = finder_query.value(
            finder_query.record().indexOf("id")).toInt();
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

    setTable(playlistViewTable, columns[0], columns,
             m_pTrackCollection->getTrackSource(m_trackSource));
    setDefaultSort(fieldIndex("position"), Qt::AscendingOrder);
    initHeaderData();
    setSearch("");
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
            | TRACKMODELCAPS_LOADTOSAMPLER;
}
