#include <QtCore>
#include <QtGui>
#include <QtSql>
#include "trackcollection.h"
#include "playlisttablemodel.h"


PlaylistTableModel::PlaylistTableModel(QWidget* parent, TrackCollection* pTrackCollection, int playlistId) : TrackModel(), QSqlTableModel(parent, pTrackCollection->getDatabase())
{
	m_pTrackCollection = pTrackCollection;
    //m_iPlaylistId = playlistId;

    setPlaylist(playlistId);

	//Hide columns in the tablemodel that we don't want to show.
	/*
	removeColumn(this->fieldIndex(LIBRARYTABLE_ID));
	removeColumn(this->fieldIndex(LIBRARYTABLE_FILENAME));
	removeColumn(this->fieldIndex(LIBRARYTABLE_URL));
	removeColumn(this->fieldIndex(LIBRARYTABLE_LENGTHINBYTES));
	removeColumn(this->fieldIndex(LIBRARYTABLE_CUEPOINT));
	removeColumn(this->fieldIndex(LIBRARYTABLE_WAVESUMMARYHEX));
	removeColumn(this->fieldIndex(LIBRARYTABLE_SAMPLERATE));
	removeColumn(this->fieldIndex(LIBRARYTABLE_CHANNELS));
	removeColumn(this->fieldIndex(LIBRARYTABLE_TRACKNUMBER));


	//Set the column heading labels, rename them for translations and have proper capitalization
    setHeaderData(this->fieldIndex(LIBRARYTABLE_ARTIST), Qt::Horizontal, tr("Artist"));
    setHeaderData(this->fieldIndex(LIBRARYTABLE_TITLE), Qt::Horizontal, tr("Title"));
    setHeaderData(this->fieldIndex(LIBRARYTABLE_ALBUM), Qt::Horizontal, tr("Album"));
    setHeaderData(this->fieldIndex(LIBRARYTABLE_GENRE), Qt::Horizontal, tr("Genre"));
    setHeaderData(this->fieldIndex(LIBRARYTABLE_YEAR), Qt::Horizontal, tr("Year"));
    setHeaderData(this->fieldIndex(LIBRARYTABLE_LOCATION), Qt::Horizontal, tr("Location"));
    setHeaderData(this->fieldIndex(LIBRARYTABLE_COMMENT), Qt::Horizontal, tr("Comment"));
    setHeaderData(this->fieldIndex(LIBRARYTABLE_DURATION), Qt::Horizontal, tr("Duration"));
    setHeaderData(this->fieldIndex(LIBRARYTABLE_BITRATE), Qt::Horizontal, tr("Bitrate"));
    setHeaderData(this->fieldIndex(LIBRARYTABLE_BPM), Qt::Horizontal, tr("BPM"));
    */


}

PlaylistTableModel::~PlaylistTableModel()
{
}


void PlaylistTableModel::setPlaylist(int playlistId)
{
    qDebug() << "PlaylistTableModel::setPlaylist" << playlistId;
    m_iPlaylistId = playlistId;

    QString playlistTableName = "playlist_" + QString("%1").arg(m_iPlaylistId);

 	QSqlQuery query;
    //query.prepare("DROP VIEW " + playlistTableName);
    //query.exec();

    //Escape the playlist name
    QSqlDriver* driver = m_pTrackCollection->getDatabase().driver();
    QSqlField playlistNameField("name", QVariant::String);
    playlistNameField.setValue(playlistTableName);

    query.prepare("CREATE TEMPORARY VIEW " + driver->formatValue(playlistNameField) + " AS "
                  "SELECT " +
                  PLAYLISTTRACKSTABLE_POSITION + "," +
                  //"playlist_id, " + //DEBUG
                  LIBRARYTABLE_ARTIST + "," +
                  LIBRARYTABLE_TITLE + "," +
                  LIBRARYTABLE_ALBUM + "," +
                  LIBRARYTABLE_YEAR + "," +
                  LIBRARYTABLE_DURATION + "," +
                  LIBRARYTABLE_GENRE + "," +
                  LIBRARYTABLE_TRACKNUMBER + "," +
                  LIBRARYTABLE_BPM + "," +
                  LIBRARYTABLE_LOCATION + "," +
                  LIBRARYTABLE_COMMENT + " "
                  "FROM library "
                  "INNER JOIN PlaylistTracks "
                  "ON library.id=PlaylistTracks.track_id "
                  "WHERE PlaylistTracks.playlist_id=" + QString("%1").arg(playlistId) + " "
                  "ORDER BY PlaylistTracks.position ");
    //query.bindValue(":playlist_name", playlistTableName);
    //query.bindValue(":playlist_id", m_iPlaylistId);
    query.exec();

    //qDebug() << query.executedQuery();

    //Print out any SQL error, if there was one.
    /*
    if (query.lastError().isValid()) {
     	qDebug() << __FILE__ << __LINE__ << query.lastError();
    }*/

    setTable(playlistTableName);

   	select(); //Populate the data model.
}


void PlaylistTableModel::addTrack(const QModelIndex& index, QString location)
{
	//Note: The model index is ignored when adding to the library track collection.
	//      The position in the library is determined by whatever it's being sorted by,
	//      and there's no arbitrary "unsorted" view.
	//m_pTrackCollection->addTrack(location);
	const int positionColumnIndex = this->fieldIndex(PLAYLISTTRACKSTABLE_POSITION);
	int position = index.sibling(index.row(), positionColumnIndex).data().toInt();
	m_pTrackCollection->insertTrackIntoPlaylist(location, m_iPlaylistId, position);
	select(); //Repopulate the data model.
}

TrackInfoObject* PlaylistTableModel::getTrack(const QModelIndex& index) const
{
    //FIXME: use position instead of location for playlist tracks?

	const int locationColumnIndex = this->fieldIndex(LIBRARYTABLE_LOCATION);
	QString location = index.sibling(index.row(), locationColumnIndex).data().toString();
	return m_pTrackCollection->getTrack(location);

}

QString PlaylistTableModel::getTrackLocation(const QModelIndex& index) const
{
    //FIXME: use position instead of location for playlist tracks?
	const int locationColumnIndex = this->fieldIndex(LIBRARYTABLE_LOCATION);
	QString location = index.sibling(index.row(), locationColumnIndex).data().toString();
	return location;
}

void PlaylistTableModel::removeTrack(const QModelIndex& index)
{
	const int positionColumnIndex = this->fieldIndex(PLAYLISTTRACKSTABLE_POSITION);
	int position = index.sibling(index.row(), positionColumnIndex).data().toInt();
	m_pTrackCollection->removeTrackFromPlaylist(m_iPlaylistId, position);
	select(); //Repopulate the data model.
}

void PlaylistTableModel::moveTrack(const QModelIndex& sourceIndex, const QModelIndex& destIndex)
{
    QSqlRecord sourceRecord = this->record(sourceIndex.row());
    this->removeRows(sourceIndex.row(), 1);

    this->insertRecord(destIndex.row(), sourceRecord);

    //TODO: execute a real query to DELETE the sourceIndex.row() row from the PlaylistTracks table.

}

void PlaylistTableModel::search(const QString& searchText)
{
    //FIXME: Need to keep filtering by playlist_id too
    //SQL is "playlist_id = " + QString(m_iPlaylistId)
    m_currentSearch = searchText;
    if (searchText == "")
        this->setFilter("");
    else
        this->setFilter("artist LIKE \'%" + searchText + "%\' OR "
                        "title  LIKE \'%" + searchText + "%\'");
}

const QString PlaylistTableModel::currentSearch() {
    return m_currentSearch;
}

QMimeData* PlaylistTableModel::mimeData(const QModelIndexList &indexes) const {
    QMimeData *mimeData = new QMimeData();
    QList<QUrl> urls;

    foreach (QModelIndex index, indexes) {
        if (index.isValid()) {
            QUrl url(getTrackLocation(index));
            if (!url.isValid())
              qDebug() << "ERROR invalid url\n";
            else
              urls.append(url);
        }
    }
    mimeData->setUrls(urls);
    return mimeData;
}

Qt::ItemFlags PlaylistTableModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);
    if (!index.isValid())
      return Qt::ItemIsEnabled;

	//Enable dragging songs from this data model to elsewhere (like the waveform widget to
	//load a track into a Player).
    defaultFlags |= Qt::ItemIsDragEnabled;

    return defaultFlags;
}

QItemDelegate* PlaylistTableModel::delegateForColumn(int i) {
    return NULL;
}

TrackModel::CapabilitiesFlags PlaylistTableModel::getCapabilities() const
{
    return TRACKMODELCAPS_RECEIVEDROPS | TRACKMODELCAPS_REORDER;
}
