#include <QtCore>
#include <QtGui>
#include <QtSql>

#include "library/trackcollection.h"
#include "library/rhythmbox/rhythmboxplaylistmodel.h"
#include "track/beatfactory.h"
#include "track/beats.h"

RhythmboxPlaylistModel::RhythmboxPlaylistModel(QObject* parent,
                                         TrackCollection* pTrackCollection)
        : BaseSqlTableModel(parent, pTrackCollection,
                            pTrackCollection->getDatabase(),
                            "mixxx.db.model.rhythmbox_playlist"),
          m_pTrackCollection(pTrackCollection),
          m_database(m_pTrackCollection->getDatabase()) {
    connect(this, SIGNAL(doSearch(const QString&)),
            this, SLOT(slotSearch(const QString&)));
}

RhythmboxPlaylistModel::~RhythmboxPlaylistModel() {
}

TrackPointer RhythmboxPlaylistModel::getTrack(const QModelIndex& index) const {
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

void RhythmboxPlaylistModel::search(const QString& searchText) {
    // qDebug() << "RhythmboxPlaylistModel::search()" << searchText
    //          << QThread::currentThread();
    emit(doSearch(searchText));
}

void RhythmboxPlaylistModel::slotSearch(const QString& searchText) {
    BaseSqlTableModel::search(searchText);
}

bool RhythmboxPlaylistModel::isColumnInternal(int column) {
    if (column == fieldIndex("track_id")) {
        return true;
    }
    return false;
}

Qt::ItemFlags RhythmboxPlaylistModel::flags(const QModelIndex &index) const {
    return readOnlyFlags(index);
}

void RhythmboxPlaylistModel::setPlaylist(QString playlist_path) {
    int playlistId = -1;
    QSqlQuery finder_query(m_database);
    finder_query.prepare("SELECT id from rhythmbox_playlists where name=:name");
    finder_query.bindValue(":name", playlist_path);

    if (!finder_query.exec()) {
        qDebug() << "SQL Error in RhythmboxPlaylistModel.cpp: line"
                 << __LINE__ << " " << finder_query.lastError();
        return;
    }

    while (finder_query.next()) {
        playlistId = finder_query.value(
            finder_query.record().indexOf("id")).toInt();
    }

    QString playlistID = "Rhythmboxplaylist_" + QString("%1").arg(playlistId);
    // Escape the playlist name
    QSqlDriver* driver = m_pTrackCollection->getDatabase().driver();
    QSqlField playlistNameField("name", QVariant::String);
    playlistNameField.setValue(playlistID);

    QStringList columns;
    columns << "track_id";
    columns << "position";

    QSqlQuery query(m_database);
    QString queryString = QString(
        "CREATE TEMPORARY VIEW IF NOT EXISTS %1 AS "
        "SELECT %2 FROM %3 WHERE playlist_id = %4")
            .arg(driver->formatValue(playlistNameField),
                 columns.join(","),
                 "rhythmbox_playlist_tracks",
                 QString::number(playlistId));
    query.prepare(queryString);

    if (!query.exec()) {
        qDebug() << "Error creating temporary view for rhythmbox playlists. "
                 << "RhythmboxPlaylistModel --> line: "
                 << __LINE__ << " " << query.lastError();
        qDebug() << "Executed Query: " <<  query.executedQuery();
        return;
    }

    setTable(playlistID, columns[0], columns,
             m_pTrackCollection->getTrackSource("rhythmbox"));
    setDefaultSort(fieldIndex("position"), Qt::AscendingOrder);
    initHeaderData();
    setSearch("");
}

bool RhythmboxPlaylistModel::isColumnHiddenByDefault(int column) {
    Q_UNUSED(column);
    return false;
}


TrackModel::CapabilitiesFlags RhythmboxPlaylistModel::getCapabilities() const {
    // See src/library/trackmodel.h for the list of TRACKMODELCAPS
    return TRACKMODELCAPS_NONE
            | TRACKMODELCAPS_ADDTOPLAYLIST
            | TRACKMODELCAPS_ADDTOCRATE
            | TRACKMODELCAPS_ADDTOAUTODJ
            | TRACKMODELCAPS_LOADTODECK
            | TRACKMODELCAPS_LOADTOSAMPLER;
}
