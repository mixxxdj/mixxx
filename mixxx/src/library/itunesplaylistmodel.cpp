#include <QtCore>
#include <QtGui>
#include <QtSql>
#include "library/trackcollection.h"
#include "library/itunesplaylistmodel.h"

#include "mixxxutils.cpp"

ITunesPlaylistModel::ITunesPlaylistModel(QObject* parent,
                                         TrackCollection* pTrackCollection)
        : TrackModel(pTrackCollection->getDatabase(),
                     "mixxx.db.model.itunes_playlist"),
          BaseSqlTableModel(parent, pTrackCollection, pTrackCollection->getDatabase()),
          m_pTrackCollection(pTrackCollection),
          m_database(m_pTrackCollection->getDatabase())
{
    connect(this, SIGNAL(doSearch(const QString&)), this, SLOT(slotSearch(const QString&)));
}

ITunesPlaylistModel::~ITunesPlaylistModel() {
}

bool ITunesPlaylistModel::addTrack(const QModelIndex& index, QString location)
{

    return false;
}

int ITunesPlaylistModel::getTrackId(const QModelIndex& index) const
{
    return -1;
}

const QLinkedList<int> ITunesPlaylistModel::getTrackRows(int trackId) const
{
    return QLinkedList<int>();
}

TrackPointer ITunesPlaylistModel::getTrack(const QModelIndex& index) const
{
    QString artist = index.sibling(index.row(), fieldIndex("artist")).data().toString();
    QString title = index.sibling(index.row(), fieldIndex("title")).data().toString();
    QString album = index.sibling(index.row(), fieldIndex("album")).data().toString();
    QString year = index.sibling(index.row(), fieldIndex("year")).data().toString();
    QString genre = index.sibling(index.row(), fieldIndex("genre")).data().toString();
    float bpm = index.sibling(index.row(), fieldIndex("bpm")).data().toString().toFloat();

    QString location = index.sibling(index.row(), fieldIndex("location")).data().toString();

    TrackInfoObject* pTrack = new TrackInfoObject(location);
    pTrack->setArtist(artist);
    pTrack->setTitle(title);
    pTrack->setAlbum(album);
    pTrack->setYear(year);
    pTrack->setGenre(genre);
    pTrack->setBpm(bpm);

    return TrackPointer(pTrack, &QObject::deleteLater);
}

QString ITunesPlaylistModel::getTrackLocation(const QModelIndex& index) const {
    QString location = index.sibling(index.row(), fieldIndex("location")).data().toString();
    return location;
}

void ITunesPlaylistModel::removeTrack(const QModelIndex& index) {

}

void ITunesPlaylistModel::removeTracks(const QModelIndexList& indices) {

}

void ITunesPlaylistModel::moveTrack(const QModelIndex& sourceIndex, const QModelIndex& destIndex) {

}

void ITunesPlaylistModel::search(const QString& searchText) {
    // qDebug() << "ITunesPlaylistModel::search()" << searchText
    //          << QThread::currentThread();
    emit(doSearch(searchText));
}

void ITunesPlaylistModel::slotSearch(const QString& searchText) {
    BaseSqlTableModel::search(searchText);
}

const QString ITunesPlaylistModel::currentSearch() {
    return BaseSqlTableModel::currentSearch();
}

bool ITunesPlaylistModel::isColumnInternal(int column) {
    if (column == fieldIndex(LIBRARYTABLE_ID) ||
        column == fieldIndex(LIBRARYTABLE_MIXXXDELETED) ||
        column == fieldIndex(TRACKLOCATIONSTABLE_FSDELETED) ||
        column == fieldIndex("name") ||
        column == fieldIndex("track_id"))
        return true;
    return false;
}

QMimeData* ITunesPlaylistModel::mimeData(const QModelIndexList &indexes) const {
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


QItemDelegate* ITunesPlaylistModel::delegateForColumn(const int i) {
    return NULL;
}

TrackModel::CapabilitiesFlags ITunesPlaylistModel::getCapabilities() const {
    return NULL;
}

Qt::ItemFlags ITunesPlaylistModel::flags(const QModelIndex &index) const {
    return readOnlyFlags(index);
}

void ITunesPlaylistModel::setPlaylist(QString playlist_path) {
    int playlistId = -1;
    QSqlQuery finder_query(m_database);
    finder_query.prepare("SELECT id from itunes_playlists where name='"+playlist_path+"'");

    if(finder_query.exec()){
        while (finder_query.next()) {
            playlistId = finder_query.value(finder_query.record().indexOf("id")).toInt();
        }
    }
    else {
        qDebug() << "SQL Error in ITunesPlaylistModel.cpp: line" << __LINE__ << " " << finder_query.lastError();
    }


    QString playlistID = "ITunesPlaylist_" + QString("%1").arg(playlistId);
    //Escape the playlist name
    QSqlDriver* driver = m_pTrackCollection->getDatabase().driver();
    QSqlField playlistNameField("name", QVariant::String);
    playlistNameField.setValue(playlistID);

    QStringList columns;
    columns << "itunes_library.id";

    QSqlQuery query(m_database);
    query.prepare("CREATE TEMPORARY VIEW IF NOT EXISTS " +
                  driver->formatValue(playlistNameField) + " AS "
                  "SELECT "
                  + columns.join(",") +
                  " FROM itunes_library "
                  "INNER JOIN itunes_playlist_tracks "
                  "ON itunes_playlist_tracks.track_id = itunes_library.id "
                  "INNER JOIN itunes_playlists "
                  "ON itunes_playlist_tracks.playlist_id = itunes_playlists.id "
                  "where itunes_playlists.name='"+playlist_path+"'"
                  );


    if (!query.exec()) {

        qDebug() << "Error creating temporary view for itunes playlists. ITunesPlaylistModel --> line: " << __LINE__ << " " << query.lastError();
        qDebug() << "Executed Query: " <<  query.executedQuery();
        return;
    }

    QStringList tableColumns;
    tableColumns << "id";
    setTable(playlistID, "id", tableColumns,
             m_pTrackCollection->getTrackSource("itunes"));

    //removeColumn(fieldIndex("track_id"));
    //removeColumn(fieldIndex("name"));
    //removeColumn(fieldIndex("id"));

    initHeaderData();
    slotSearch("");
    select(); //Populate the data model.
}

bool ITunesPlaylistModel::isColumnHiddenByDefault(int column) {
    return false;
}
