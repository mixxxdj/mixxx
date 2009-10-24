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
    //sourceRecord.setValue("position", destIndex.row());
    //this->removeRows(sourceIndex.row(), 1);


    //this->insertRecord(destIndex.row(), sourceRecord);

    //TODO: execute a real query to DELETE the sourceIndex.row() row from the PlaylistTracks table.
    //int newPosition = destIndex.row();
    //int oldPosition = sourceIndex.row();
    //const int positionColumnIndex = this->fieldIndex(PLAYLISTTRACKSTABLE_POSITION);
    //int newPosition = index.sibling(destIndex.row(), positionColumnIndex).data().toInt();
    //int oldPosition = index.sibling(sourceIndex.row(), positionColumnIndex).data().toInt();
    int newPosition = this->record(destIndex.row()).value(PLAYLISTTRACKSTABLE_POSITION).toInt();
    int oldPosition = this->record(sourceIndex.row()).value(PLAYLISTTRACKSTABLE_POSITION).toInt();

    qDebug() << "old pos" << oldPosition << "new pos" << newPosition;

    //Invalid for the position to be 0 or less. (Happens when the tracked is dropped out of bounds.)
    if (newPosition < 0)
        return;

    //Start the transaction
    QSqlDatabase::database().transaction();

    //Find out the highest position existing in the playlist so we know what
    //position this track should have.
    QSqlQuery query;

    //Insert the song into the PlaylistTracks table

    /** ALGORITHM for code below
      Case 1: destination < source (newPos < oldPos)
        1) Set position = -1 where pos=source -- Gives that track a dummy index to keep stuff simple.
        2) Decrement position where pos > source
        3) increment position where pos > dest
        4) Set position = dest where pos=-1 -- Move track from dummy pos to final destination.

      Case 2: destination > source (newPos > oldPos)
        1) Set position=-1 where pos=source -- Give track a dummy index again.
        2) Decrement position where pos > source AND pos <= dest
        3) Set postion=dest where pos=-1 -- Move that track from dummy pos to final destination
    */

    QString queryString;
    if (newPosition < oldPosition) {
        queryString =
            QString("UPDATE PlaylistTracks SET position=-1 "
                    "WHERE position=%1 AND "
                    "playlist_id=%2").arg(oldPosition).arg(m_iPlaylistId);
        query.exec(queryString);
        //qDebug() << queryString;

        queryString = QString("UPDATE PlaylistTracks SET position=position-1 "
                            "WHERE position > %1 AND "
                            "playlist_id=%2").arg(oldPosition).arg(m_iPlaylistId);
        query.exec(queryString);

        queryString = QString("UPDATE PlaylistTracks SET position=position+1 "
                            "WHERE position >= %1 AND " //position < %2 AND "
                            "playlist_id=%3").arg(newPosition).arg(m_iPlaylistId);
        query.exec(queryString);

        queryString = QString("UPDATE PlaylistTracks SET position=%1 "
                            "WHERE position=-1 AND "
                            "playlist_id=%2").arg(newPosition).arg(m_iPlaylistId);
        query.exec(queryString);
    }
    else if (newPosition > oldPosition)
    {
        queryString = QString("UPDATE PlaylistTracks SET position=-1 "
                              "WHERE position = %1 AND "
                              "playlist_id=%2").arg(oldPosition).arg(m_iPlaylistId);
        //qDebug() << queryString;
        query.exec(queryString);

        queryString = QString("UPDATE PlaylistTracks SET position=position-1 "
                              "WHERE position > %1 AND position <= %2 AND "
                              "playlist_id=%3").arg(oldPosition).arg(newPosition).arg(m_iPlaylistId);
        query.exec(queryString);

        queryString = QString("UPDATE PlaylistTracks SET position=%1 "
                              "WHERE position=-1 AND "
                              "playlist_id=%2").arg(newPosition).arg(m_iPlaylistId);
        query.exec(queryString);
    }

    QSqlDatabase::database().commit();

    //Print out any SQL error, if there was one.
    if (query.lastError().isValid()) {
     	qDebug() << query.lastError();
    }

    select();
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

    //Ok, so the list of indexes we're given contains separates indexes for
    //each column, so even if only one row is selected, we'll have like 7 indexes.
    //We need to only count each row once:
    QList<int> rows;

    foreach (QModelIndex index, indexes) {
        if (index.isValid()) {
            if (!rows.contains(index.row())) {
                rows.push_back(index.row());
                QUrl url(getTrackLocation(index));
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

QVariant PlaylistTableModel::data(const QModelIndex& item, int role) const {
    if (!item.isValid())
        return QVariant();

    QVariant value = QSqlTableModel::data(item, role);

    if (role == Qt::DisplayRole &&
        item.column() == fieldIndex(LIBRARYTABLE_DURATION)) {
        if (qVariantCanConvert<int>(value)) {
            // TODO(XXX) Pull this out into a MixxxUtil or something.

            //Let's reformat this song length into a human readable MM:SS format.
            int totalSeconds = qVariantValue<int>(value);
            int seconds = totalSeconds % 60;
            int mins = totalSeconds / 60;
            //int hours = mins / 60; //Not going to worry about this for now. :)

            //Construct a nicely formatted duration string now.
            value = QString("%1:%2").arg(mins).arg(seconds, 2, 10, QChar('0'));
        }
    }
    return value;
}

TrackModel::CapabilitiesFlags PlaylistTableModel::getCapabilities() const
{
    return TRACKMODELCAPS_RECEIVEDROPS | TRACKMODELCAPS_REORDER;
}
