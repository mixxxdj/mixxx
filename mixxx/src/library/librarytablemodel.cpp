#include <QtCore>
#include <QtGui>
#include <QtSql>

#include "library/trackcollection.h"
#include "library/librarytablemodel.h"

#include "mixxxutils.cpp"

const QString LibraryTableModel::DEFAULT_LIBRARYFILTER = "mixxx_deleted=0";

LibraryTableModel::LibraryTableModel(QObject* parent,
                                     TrackCollection* pTrackCollection)
        : TrackModel(pTrackCollection->getDatabase(),
                     "mixxx.db.model.library"),
          BaseSqlTableModel(parent, pTrackCollection, pTrackCollection->getDatabase()),
          m_trackDao(pTrackCollection->getTrackDAO()) {

    QSqlQuery query(pTrackCollection->getDatabase());
    query.prepare("CREATE TEMPORARY VIEW IF NOT EXISTS library_view AS "
                  "SELECT "
                  "library." + LIBRARYTABLE_ID + "," +
                  "library." + LIBRARYTABLE_ARTIST + "," +
                  "library." + LIBRARYTABLE_TITLE + "," +
                  "library." + LIBRARYTABLE_ALBUM + "," +
                  "library." + LIBRARYTABLE_YEAR + "," +
                  "library." + LIBRARYTABLE_DURATION + "," +
                  "library." + LIBRARYTABLE_GENRE + "," +
                  "library." + LIBRARYTABLE_FILETYPE + "," +
                  "library." + LIBRARYTABLE_TRACKNUMBER + "," +
                  "library." + LIBRARYTABLE_DATETIMEADDED + "," +
                  "library." + LIBRARYTABLE_BPM + "," +
                  "track_locations.location," +
                  "library." + LIBRARYTABLE_COMMENT + "," +
                  "library." + LIBRARYTABLE_MIXXXDELETED + " " +
                  "FROM library " +
                  "INNER JOIN track_locations " +
                  "ON library.location = track_locations.id ");
    if (!query.exec()) {
        qDebug() << query.executedQuery() << query.lastError();
    }

    //Print out any SQL error, if there was one.
    if (query.lastError().isValid()) {
     	qDebug() << __FILE__ << __LINE__ << query.lastError();
    }

    //setTable("library");
    setTable("library_view");

    //Set up a relation which maps our location column (which is a foreign key
    //into the track_locations) table. We tell Qt that our LIBRARYTABLE_LOCATION
    //column maps into the row of the track_locations table that has the id
    //equal to our location col. It then grabs the "location" col from that row
    //and shows it...
    //setRelation(fieldIndex(LIBRARYTABLE_LOCATION), QSqlRelation("track_locations", "id", "location"));

    //Set the column heading labels, rename them for translations and have
    //proper capitalization
    setHeaderData(fieldIndex(LIBRARYTABLE_ARTIST),
                  Qt::Horizontal, tr("Artist"));
    setHeaderData(fieldIndex(LIBRARYTABLE_TITLE),
                  Qt::Horizontal, tr("Title"));
    setHeaderData(fieldIndex(LIBRARYTABLE_ALBUM),
                  Qt::Horizontal, tr("Album"));
    setHeaderData(fieldIndex(LIBRARYTABLE_GENRE),
                  Qt::Horizontal, tr("Genre"));
    setHeaderData(fieldIndex(LIBRARYTABLE_YEAR),
                  Qt::Horizontal, tr("Year"));
    setHeaderData(fieldIndex(LIBRARYTABLE_FILETYPE),
                  Qt::Horizontal, tr("Type"));
    setHeaderData(fieldIndex(LIBRARYTABLE_LOCATION),
                  Qt::Horizontal, tr("Location"));
    setHeaderData(fieldIndex(LIBRARYTABLE_COMMENT),
                  Qt::Horizontal, tr("Comment"));
    setHeaderData(fieldIndex(LIBRARYTABLE_DURATION),
                  Qt::Horizontal, tr("Duration"));
    setHeaderData(fieldIndex(LIBRARYTABLE_BITRATE),
                  Qt::Horizontal, tr("Bitrate"));
    setHeaderData(fieldIndex(LIBRARYTABLE_BPM),
                  Qt::Horizontal, tr("BPM"));
    setHeaderData(fieldIndex(LIBRARYTABLE_TRACKNUMBER),
                  Qt::Horizontal, tr("Track #"));
    setHeaderData(fieldIndex(LIBRARYTABLE_DATETIMEADDED),
                  Qt::Horizontal, tr("Date Added"));

    //Sets up the table filter so that we don't show "deleted" tracks (only show mixxx_deleted=0).
    slotSearch("");

    select(); //Populate the data model.

    connect(this, SIGNAL(doSearch(const QString&)),
            this, SLOT(slotSearch(const QString&)));
}

LibraryTableModel::~LibraryTableModel()
{

}

bool LibraryTableModel::addTrack(const QModelIndex& index, QString location)
{
    //Note: The model index is ignored when adding to the library track collection.
    //      The position in the library is determined by whatever it's being sorted by,
    //      and there's no arbitrary "unsorted" view.
    QFileInfo fileInfo(location);
    if (m_trackDao.trackExistsInDatabase(fileInfo.absoluteFilePath())) {
        return true;
    }

    int trackId = m_trackDao.addTrack(fileInfo);
    select(); //Repopulate the data model.

    if (trackId >= 0)
        return true;
    return false;
}

TrackPointer LibraryTableModel::getTrack(const QModelIndex& index) const
{
	int trackId = index.sibling(index.row(), fieldIndex(LIBRARYTABLE_ID)).data().toInt();
	return m_trackDao.getTrack(trackId);
}

QString LibraryTableModel::getTrackLocation(const QModelIndex& index) const
{
	const int locationColumnIndex = fieldIndex(LIBRARYTABLE_LOCATION);
	QString location = index.sibling(index.row(), locationColumnIndex).data().toString();
	return location;
}

void LibraryTableModel::removeTrack(const QModelIndex& index)
{
	int trackId = index.sibling(index.row(), fieldIndex(LIBRARYTABLE_ID)).data().toInt();
	m_trackDao.removeTrack(trackId);
	select(); //Repopulate the data model.
}

void LibraryTableModel::moveTrack(const QModelIndex& sourceIndex, const QModelIndex& destIndex)
{
    //Does nothing because we don't support reordering tracks in the library,
    //and getCapabilities() reports that.
}

void LibraryTableModel::search(const QString& searchText) {
    // qDebug() << "LibraryTableModel::search()" << searchText
    //          << QThread::currentThread();
    emit(doSearch(searchText));
}

void LibraryTableModel::slotSearch(const QString& searchText) {
    // qDebug() << "slotSearch()" << searchText << QThread::currentThread();
    if (!m_currentSearch.isNull() && m_currentSearch == searchText)
        return;

    m_currentSearch = searchText;

    QString filter;
    if (searchText == "")
        filter = "(" + LibraryTableModel::DEFAULT_LIBRARYFILTER + ")";
    else {
        QSqlField search("search", QVariant::String);
        search.setValue("%" + searchText + "%");
        QString escapedText = database().driver()->formatValue(search);
        filter = "(" + LibraryTableModel::DEFAULT_LIBRARYFILTER + " AND " +
                "(artist LIKE " + escapedText + " OR " +
                "album LIKE " + escapedText + " OR " +
                "title  LIKE " + escapedText + "))";
    }
    setFilter(filter);
}

const QString LibraryTableModel::currentSearch() {
    //qDebug() << "LibraryTableModel::currentSearch(): " << m_currentSearch;
    return m_currentSearch;
}

bool LibraryTableModel::isColumnInternal(int column) {

    if ((column == fieldIndex(LIBRARYTABLE_ID)) ||
        (column == fieldIndex(LIBRARYTABLE_URL)) ||
        (column == fieldIndex(LIBRARYTABLE_CUEPOINT)) ||
        (column == fieldIndex(LIBRARYTABLE_WAVESUMMARYHEX)) ||
        (column == fieldIndex(LIBRARYTABLE_SAMPLERATE)) ||
        (column == fieldIndex(LIBRARYTABLE_MIXXXDELETED)) ||
        (column == fieldIndex(LIBRARYTABLE_HEADERPARSED)) ||
        (column == fieldIndex(LIBRARYTABLE_CHANNELS))) {
        return true;
    }
    return false;
}

QItemDelegate* LibraryTableModel::delegateForColumn(const int i) {
    return NULL;
}

QVariant LibraryTableModel::data(const QModelIndex& item, int role) const {
    if (!item.isValid())
        return QVariant();

    QVariant value;
    if (role == Qt::ToolTipRole)
        value = BaseSqlTableModel::data(item, Qt::DisplayRole);
    else
        value = BaseSqlTableModel::data(item, role);

    if ((role == Qt::DisplayRole || role == Qt::ToolTipRole) &&
        item.column() == fieldIndex(LIBRARYTABLE_DURATION)) {
        if (qVariantCanConvert<int>(value)) {
            value = MixxxUtils::secondsToMinutes(qVariantValue<int>(value));
        }
    }

    return value;
}

QMimeData* LibraryTableModel::mimeData(const QModelIndexList &indexes) const {
    QMimeData *mimeData = new QMimeData();
    QList<QUrl> urls;

    //Ok, so the list of indexes we're given contains separates indexes for
    //each column, so even if only one row is selected, we'll have like 7 indexes.
    //We need to only count each row once:
    QList<int> rows;

    foreach (QModelIndex index, indexes) {
        if (index.isValid()) {
            if (!rows.contains(index.row())) //Only add a URL once per row.
            {
                rows.push_back(index.row());
                QUrl url(getTrackLocation(index));
                if (!url.isValid())
                    qDebug() << "ERROR invalid url\n";
                else {
                    urls.append(url);
                }
            }
        }
    }
    mimeData->setUrls(urls);
    return mimeData;
}

Qt::ItemFlags LibraryTableModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);
    if (!index.isValid())
      return Qt::ItemIsEnabled;

	//Enable dragging songs from this data model to elsewhere (like the waveform
	//widget to load a track into a Player).
    defaultFlags |= Qt::ItemIsDragEnabled;

    /** FIXME: This doesn't seem to work - Albert */
    const int bpmColumnIndex = fieldIndex(LIBRARYTABLE_BPM);
    if (index.column() == bpmColumnIndex)
    {
        return defaultFlags | Qt::ItemIsEditable;
    }

    return defaultFlags;
}

TrackModel::CapabilitiesFlags LibraryTableModel::getCapabilities() const
{
    return TRACKMODELCAPS_RECEIVEDROPS;
}

/*
void LibraryTableModel::notifyBeginInsertRow(int row)
{

    QModelIndex foo = index(0, 0);
    beginInsertRows(foo, row, row);
}

void LibraryTableModel::notifyEndInsertRow(int row)
{
    endInsertRows();
}
*/
