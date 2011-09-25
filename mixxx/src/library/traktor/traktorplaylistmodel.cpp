#include <QtCore>
#include <QtGui>
#include <QtSql>
#include "library/trackcollection.h"
#include "library/traktor/traktorplaylistmodel.h"

#include "mixxxutils.cpp"

TraktorPlaylistModel::TraktorPlaylistModel(QObject* parent,
                                       TrackCollection* pTrackCollection)
        : TrackModel(pTrackCollection->getDatabase(),
                     "mixxx.db.model.traktor.playlistmodel"),
          BaseSqlTableModel(parent, pTrackCollection, pTrackCollection->getDatabase()),
          m_pTrackCollection(pTrackCollection),
          m_database(m_pTrackCollection->getDatabase())
{
    connect(this, SIGNAL(doSearch(const QString&)), this, SLOT(slotSearch(const QString&)));
}

TraktorPlaylistModel::~TraktorPlaylistModel() {
}

bool TraktorPlaylistModel::addTrack(const QModelIndex& index, QString location)
{
    return false;
}

TrackPointer TraktorPlaylistModel::getTrack(const QModelIndex& index) const
{
    //qDebug() << "getTraktorTrack";
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

QString TraktorPlaylistModel::getTrackLocation(const QModelIndex& index) const
{
    QString location = index.sibling(index.row(), fieldIndex("location")).data().toString();
    return location;
}

int TraktorPlaylistModel::getTrackId(const QModelIndex& index) const {
    if (!index.isValid()) {
        return -1;
    }
    return index.sibling(index.row(), fieldIndex("id")).data().toInt();
}

const QLinkedList<int> TraktorPlaylistModel::getTrackRows(int trackId) const {
    return BaseSqlTableModel::getTrackRows(trackId);
}

void TraktorPlaylistModel::removeTrack(const QModelIndex& index)
{

}

void TraktorPlaylistModel::removeTracks(const QModelIndexList& indices) {

}

void TraktorPlaylistModel::moveTrack(const QModelIndex& sourceIndex, const QModelIndex& destIndex)
{

}

void TraktorPlaylistModel::search(const QString& searchText) {
    // qDebug() << "TraktorPlaylistModel::search()" << searchText
    //          << QThread::currentThread();
    emit(doSearch(searchText));
}

void TraktorPlaylistModel::slotSearch(const QString& searchText) {
    BaseSqlTableModel::search(searchText);
}

const QString TraktorPlaylistModel::currentSearch() {
    return BaseSqlTableModel::currentSearch();
}

bool TraktorPlaylistModel::isColumnInternal(int column) {
    if (column == fieldIndex("track_id")) {
        return true;
    }
    return false;
}

QMimeData* TraktorPlaylistModel::mimeData(const QModelIndexList &indexes) const {
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

QItemDelegate* TraktorPlaylistModel::delegateForColumn(const int i) {
    return NULL;
}

TrackModel::CapabilitiesFlags TraktorPlaylistModel::getCapabilities() const
{
    return TRACKMODELCAPS_NONE;
}

Qt::ItemFlags TraktorPlaylistModel::flags(const QModelIndex &index) const
{
    return readOnlyFlags(index);
}

void TraktorPlaylistModel::setPlaylist(QString playlist_path) {
    int playlistId = -1;
    QSqlQuery finder_query(m_database);
    finder_query.prepare(
        "SELECT id from traktor_playlists where name='"+playlist_path+"'");

    if (!finder_query.exec()) {
        qDebug() << "SQL Error in TraktorPlaylistModel.cpp: line" << __LINE__
                 << finder_query.lastError();
    }
    while (finder_query.next()) {
        playlistId = finder_query.value(
            finder_query.record().indexOf("id")).toInt();
    }

    QString playlistID = "TraktorPlaylist_" + QString("%1").arg(playlistId);
    // Escape the playlist name
    QSqlDriver* driver = m_pTrackCollection->getDatabase().driver();
    QSqlField playlistNameField("name", QVariant::String);
    playlistNameField.setValue(playlistID);

    QStringList columns;
    columns << "track_id";

    QSqlQuery query(m_database);
    query.prepare("CREATE TEMPORARY VIEW IF NOT EXISTS " +
                  driver->formatValue(playlistNameField) + " AS "
                  "SELECT " + columns.join(",") +
                  " FROM traktor_playlist_tracks "
                  "WHERE playlist_id = " + QString("%1").arg(playlistId));

    if (!query.exec()) {
        qDebug() << "Error creating temporary view for traktor playlists."
                 << "TraktorPlaylistModel --> line: " << __LINE__
                 << query.lastError();
        qDebug() << "Executed Query: " << query.executedQuery();
        return;
    }

    setTable(playlistID, columns[0], columns,
             m_pTrackCollection->getTrackSource("traktor"));
    initHeaderData();
    setSearch("");
}

bool TraktorPlaylistModel::isColumnHiddenByDefault(int column) {
    if (column == fieldIndex(LIBRARYTABLE_KEY) ||
        column == fieldIndex(LIBRARYTABLE_BITRATE)) {
        return true;
    }
    return false;
}
