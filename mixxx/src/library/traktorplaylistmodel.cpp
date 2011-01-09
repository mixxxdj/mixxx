#include <QtCore>
#include <QtGui>
#include <QtSql>
#include "library/trackcollection.h"
#include "library/traktorplaylistmodel.h"

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

void TraktorPlaylistModel::slotSearch(const QString& searchText)
{
   if (!m_currentSearch.isNull() && m_currentSearch == searchText)
        return;
    m_currentSearch = searchText;

    //update database so searches reflect updated data
    m_pTrackCollection->getTrackDAO().saveDirtyTracks();
    
    QString filter;
    QSqlField search("search", QVariant::String);
    search.setValue("%" + searchText + "%");
    QString escapedText = database().driver()->formatValue(search);
    filter = "(artist LIKE " + escapedText + " OR " +
                "album LIKE " + escapedText + " OR " +
                "title  LIKE " + escapedText + ")";
    setFilter(filter);
}

const QString TraktorPlaylistModel::currentSearch() {
    return m_currentSearch;
}

bool TraktorPlaylistModel::isColumnInternal(int column) {
    if (column == fieldIndex(LIBRARYTABLE_ID) ||
        column == fieldIndex(LIBRARYTABLE_MIXXXDELETED) ||
        column == fieldIndex(TRACKLOCATIONSTABLE_FSDELETED) ||
        column == fieldIndex("name") ||
        column == fieldIndex("track_id"))
        return true;
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
  
    return NULL;
}

Qt::ItemFlags TraktorPlaylistModel::flags(const QModelIndex &index) const
{
    return readOnlyFlags(index);
}
void TraktorPlaylistModel::setPlaylist(QString playlist_path)
{
    int playlistId = -1;
    QSqlQuery finder_query(m_database);
    finder_query.prepare("SELECT id from traktor_playlists where name='"+playlist_path+"'");
        
    if(finder_query.exec()){
        while (finder_query.next()) {
            playlistId = finder_query.value(finder_query.record().indexOf("id")).toInt();
        }
    }   
    else
        qDebug() << "SQL Error in TraktorPlaylistModel.cpp: line" << __LINE__ << " " << finder_query.lastError(); 
   

    QString playlistID = "TraktorPlaylist_" + QString("%1").arg(playlistId);
    //Escape the playlist name
    QSqlDriver* driver = m_pTrackCollection->getDatabase().driver();
    QSqlField playlistNameField("name", QVariant::String);
    playlistNameField.setValue(playlistID);

    QSqlQuery query(m_database);
    query.prepare("CREATE TEMPORARY VIEW IF NOT EXISTS "+ driver->formatValue(playlistNameField) + " AS "
                  "SELECT " 
                  "traktor_library.id,"
                  "traktor_library.artist," 
                  "traktor_library.title,"
                  "traktor_library.album,"
                  "traktor_library.year,"
                  "traktor_library.genre,"
                  "traktor_library.tracknumber,"
                  "traktor_library.location,"
                  "traktor_library.comment,"
                  "traktor_library.rating,"
                  "traktor_library.duration,"
                  "traktor_library.bitrate,"
                  "traktor_library.bpm,"
                  "traktor_library.key,"
                  "traktor_playlist_tracks.track_id, "
                  "traktor_playlists.name "
                  "FROM traktor_library "
                  "INNER JOIN traktor_playlist_tracks "
                  "ON traktor_playlist_tracks.track_id = traktor_library.id "
                  "INNER JOIN traktor_playlists "
                  "ON traktor_playlist_tracks.playlist_id = traktor_playlists.id "
                  "where traktor_playlists.name='"+playlist_path+"'"
                  );
    
      
    if (!query.exec()) {
       
        qDebug() << "Error creating temporary view for traktor playlists. TraktorPlaylistModel --> line: " << __LINE__ << " " << query.lastError();
        qDebug() << "Executed Query: " <<  query.executedQuery();
        return;
    }
	setTable(playlistID);
    
	//removeColumn(fieldIndex("track_id"));
	//removeColumn(fieldIndex("name"));
	//removeColumn(fieldIndex("id"));

    slotSearch("");

    select(); //Populate the data model.
    initHeaderData();
}
bool TraktorPlaylistModel::isColumnHiddenByDefault(int column) {
    if (column == fieldIndex(LIBRARYTABLE_KEY))    
        return true;
    if(column == fieldIndex(LIBRARYTABLE_BITRATE))
    	return true;
    
    return false;
}
