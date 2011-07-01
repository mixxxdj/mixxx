#include <QtCore>
#include <QtGui>
#include <QtSql>
#include "library/trackcollection.h"
#include "library/rhythmbox/rhythmboxplaylistmodel.h"

#include "mixxxutils.cpp"

RhythmboxPlaylistModel::RhythmboxPlaylistModel(QObject* parent,
                                         TrackCollection* pTrackCollection)
        : TrackModel(pTrackCollection->getDatabase(),
                     "mixxx.db.model.rhythmbox_playlist"),
          BaseSqlTableModel(parent, pTrackCollection, pTrackCollection->getDatabase()),
          m_pTrackCollection(pTrackCollection),
          m_database(m_pTrackCollection->getDatabase())
{
    connect(this, SIGNAL(doSearch(const QString&)), this, SLOT(slotSearch(const QString&)));
    setCaching(false);
}

RhythmboxPlaylistModel::~RhythmboxPlaylistModel() {
}

bool RhythmboxPlaylistModel::addTrack(const QModelIndex& index, QString location) {
    Q_UNUSED(index);
    Q_UNUSED(location);
	return false;
}

TrackPointer RhythmboxPlaylistModel::getTrack(const QModelIndex& index) const {
    QString artist = index.sibling(index.row(), fieldIndex("artist")).data().toString();
    QString title = index.sibling(index.row(), fieldIndex("title")).data().toString();
    QString album = index.sibling(index.row(), fieldIndex("album")).data().toString();
    QString year = index.sibling(index.row(), fieldIndex("year")).data().toString();
    QString genre = index.sibling(index.row(), fieldIndex("genre")).data().toString();
    float bpm = index.sibling(index.row(), fieldIndex("bpm")).data().toString().toFloat();

    QString location = index.sibling(index.row(), fieldIndex("location")).data().toString();

    if (location.isEmpty()) {
    	// Track is lost
    	return TrackPointer();
    }

    TrackDAO& track_dao = m_pTrackCollection->getTrackDAO();
    int track_id = track_dao.getTrackId(location);
    if (track_id < 0) {
    	// Add Track to library
    	track_id = track_dao.addTrack(location);
    }

    TrackPointer pTrack;

    if (track_id < 0) {
    	// Add Track to library failed
    	// Create own TrackInfoObject
    	pTrack = TrackPointer(new TrackInfoObject(location), &QObject::deleteLater);
    }
    else {
    	pTrack = track_dao.getTrack(track_id);
    }

    // Overwrite Metadata from Rythmbox library
    pTrack->setArtist(artist);
    pTrack->setTitle(title);
    pTrack->setAlbum(album);
    pTrack->setYear(year);
    pTrack->setGenre(genre);
    pTrack->setBpm(bpm);

    return pTrack;
}

QString RhythmboxPlaylistModel::getTrackLocation(const QModelIndex& index) const {
    QString location = index.sibling(index.row(), fieldIndex("location")).data().toString();
    return location;
}

int RhythmboxPlaylistModel::getTrackId(const QModelIndex& index) const {
    if (!index.isValid()) {
        return -1;
    }
    return index.sibling(index.row(), fieldIndex("id")).data().toInt();
}

const QLinkedList<int> RhythmboxPlaylistModel::getTrackRows(int trackId) const {
    return BaseSqlTableModel::getTrackRows(trackId);
}

void RhythmboxPlaylistModel::removeTrack(const QModelIndex& index) {
    Q_UNUSED(index);
}

void RhythmboxPlaylistModel::removeTracks(const QModelIndexList& indices) {
    Q_UNUSED(indices);
}

void RhythmboxPlaylistModel::moveTrack(const QModelIndex& sourceIndex, const QModelIndex& destIndex) {
    Q_UNUSED(sourceIndex);
    Q_UNUSED(destIndex);
}

void RhythmboxPlaylistModel::search(const QString& searchText) {
    // qDebug() << "RhythmboxPlaylistModel::search()" << searchText
    //          << QThread::currentThread();
    emit(doSearch(searchText));
}

void RhythmboxPlaylistModel::slotSearch(const QString& searchText) {
    BaseSqlTableModel::search(searchText);
}

const QString RhythmboxPlaylistModel::currentSearch() {
    return BaseSqlTableModel::currentSearch();
}

bool RhythmboxPlaylistModel::isColumnInternal(int column) {
    if (column == fieldIndex(LIBRARYTABLE_ID) ||
        column == fieldIndex(LIBRARYTABLE_MIXXXDELETED) ||
        column == fieldIndex(TRACKLOCATIONSTABLE_FSDELETED) ||
        column == fieldIndex("name") ||
        column == fieldIndex("track_id"))
        return true;
    return false;
}

QMimeData* RhythmboxPlaylistModel::mimeData(const QModelIndexList &indexes) const {
    QMimeData *mimeData = new QMimeData();
    QList<QUrl> urls;

    //Ok, so the list of indexes we're given contains separates indexes for
    //each column, so even if only one row is selected, we'll have like 7 indexes.
    //We need to only count each row once:
    QList<int> rows;

    foreach (QModelIndex index, indexes) {
        if (index.isValid()) {
            if (!rows.contains(index.row())) {
                rows.push_back(index.row());
                QUrl url = QUrl::fromLocalFile(getTrackLocation(index));
                if (!url.isValid())
                    qDebug() << "ERROR invalid url\n";
                else
                    urls.append(url);
            }
        }
    }
    mimeData->setUrls(urls);
    return mimeData;
}

QItemDelegate* RhythmboxPlaylistModel::delegateForColumn(const int i) {
    Q_UNUSED(i);
	return NULL;
}

TrackModel::CapabilitiesFlags RhythmboxPlaylistModel::getCapabilities() const {
    return TRACKMODELCAPS_NONE |
      	   TRACKMODELCAPS_ADDTOAUTODJ;
}

Qt::ItemFlags RhythmboxPlaylistModel::flags(const QModelIndex &index) const {
    return readOnlyFlags(index);
}

void RhythmboxPlaylistModel::setPlaylist(QString playlist_path) {
    int playlistId = -1;
    QSqlQuery finder_query(m_database);
    finder_query.prepare("SELECT id from rhythmbox_playlists where name='"+playlist_path+"'");

    if(finder_query.exec()){
        while (finder_query.next()) {
            playlistId = finder_query.value(finder_query.record().indexOf("id")).toInt();
        }
    }
    else
        qDebug() << "SQL Error in RhythmboxPlaylistModel.cpp: line" << __LINE__ << " " << finder_query.lastError();


    QString playlistID = "Rhythmboxplaylist_" + QString("%1").arg(playlistId);
    //Escape the playlist name
    QSqlDriver* driver = m_pTrackCollection->getDatabase().driver();
    QSqlField playlistNameField("name", QVariant::String);
    playlistNameField.setValue(playlistID);

    QStringList columns;
    columns << "rhythmbox_library.id"
            << "rhythmbox_library.artist"
            << "rhythmbox_library.title"
            << "rhythmbox_library.album"
            << "rhythmbox_library.year"
            << "rhythmbox_library.genre"
            << "rhythmbox_library.tracknumber"
            << "rhythmbox_library.location"
            << "rhythmbox_library.comment"
            << "rhythmbox_library.rating"
            << "rhythmbox_library.duration"
            << "rhythmbox_library.bitrate"
            << "rhythmbox_library.bpm"
            << "rhythmbox_playlist_tracks.track_id"
            << "rhythmbox_playlists.name";

    QSqlQuery query(m_database);
    query.prepare("CREATE TEMPORARY VIEW IF NOT EXISTS "+ driver->formatValue(playlistNameField) + " AS "
                  "SELECT "
                  + columns.join(",") +
                  " FROM rhythmbox_library "
                  "INNER JOIN rhythmbox_playlist_tracks "
                  "ON rhythmbox_playlist_tracks.track_id = rhythmbox_library.id "
                  "INNER JOIN rhythmbox_playlists "
                  "ON rhythmbox_playlist_tracks.playlist_id = rhythmbox_playlists.id "
                  "where rhythmbox_playlists.name='"+playlist_path+"'"
                  );


    if (!query.exec()) {

        qDebug() << "Error creating temporary view for rhythmbox playlists. RhythmboxPlaylistModel --> line: " << __LINE__ << " " << query.lastError();
        qDebug() << "Executed Query: " <<  query.executedQuery();
        return;
    }

    // Strip out library. and track_locations.
    for (int i = 0; i < columns.size(); ++i) {
        columns[i] = columns[i].replace("rhythmbox_library.", "")
                .replace("rhythmbox_playlist_tracks.", "").replace("rhythmbox_playlists.", "");
    }

    setTable(playlistID, columns, "id");
    //removeColumn(fieldIndex("track_id"));
    //removeColumn(fieldIndex("name"));
    //removeColumn(fieldIndex("id"));
    initHeaderData();
    initDefaultSearchColumns();
    slotSearch("");
    select(); //Populate the data model.
}

bool RhythmboxPlaylistModel::isColumnHiddenByDefault(int column) {
    Q_UNUSED(column);
	return false;
}
