#include <QtCore>
#include <QtGui>
#include <QtSql>

#include "library/trackcollection.h"
#include "library/librarytablemodel.h"
#include "durationdelegate.h"


LibraryTableModel::LibraryTableModel(QObject* parent,
                                     TrackCollection* pTrackCollection)
    : TrackModel(),
      QSqlTableModel(parent, pTrackCollection->getDatabase()) {
	m_pTrackCollection = pTrackCollection;

	setTable("library");
	
	//Hide columns in the tablemodel that we don't want to show.
	removeColumn(fieldIndex(LIBRARYTABLE_ID)); 
	removeColumn(fieldIndex(LIBRARYTABLE_FILENAME));
	removeColumn(fieldIndex(LIBRARYTABLE_URL));
	removeColumn(fieldIndex(LIBRARYTABLE_LENGTHINBYTES));
	removeColumn(fieldIndex(LIBRARYTABLE_CUEPOINT));
	removeColumn(fieldIndex(LIBRARYTABLE_WAVESUMMARYHEX));
	removeColumn(fieldIndex(LIBRARYTABLE_SAMPLERATE));
	removeColumn(fieldIndex(LIBRARYTABLE_CHANNELS));
	removeColumn(fieldIndex(LIBRARYTABLE_TRACKNUMBER));

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

   	select(); //Populate the data model.
}

LibraryTableModel::~LibraryTableModel()
{
	delete m_pTrackCollection;
}

void LibraryTableModel::addTrack(const QModelIndex& index, QString location)
{
	//Note: The model index is ignored when adding to the library track collection.
	//      The position in the library is determined by whatever it's being sorted by,
	//      and there's no arbitrary "unsorted" view.
	m_pTrackCollection->addTrack(location);
	select(); //Repopulate the data model.
}

TrackInfoObject* LibraryTableModel::getTrack(const QModelIndex& index) const
{
	const int locationColumnIndex = fieldIndex(LIBRARYTABLE_LOCATION);
	QString location = index.sibling(index.row(), locationColumnIndex).data().toString();
	return m_pTrackCollection->getTrack(location);
}

QString LibraryTableModel::getTrackLocation(const QModelIndex& index) const
{
	const int locationColumnIndex = fieldIndex(LIBRARYTABLE_LOCATION);
	QString location = index.sibling(index.row(), locationColumnIndex).data().toString();
	return location;
}

void LibraryTableModel::removeTrack(const QModelIndex& index)
{
	const int locationColumnIndex = fieldIndex(LIBRARYTABLE_LOCATION);
	QString location = index.sibling(index.row(), locationColumnIndex).data().toString();
	m_pTrackCollection->removeTrack(location);
	select(); //Repopulate the data model.
}

void LibraryTableModel::search(const QString& searchText)
{
	if (searchText == "")
		setFilter("");
	else
		setFilter("artist LIKE \'%" + searchText + "%\' OR "
						"title  LIKE \'%" + searchText + "%\'");
}

QItemDelegate* LibraryTableModel::delegateForColumn(const int i) {
    if (i == fieldIndex(LIBRARYTABLE_DURATION)) {
        // TODO(rryan) hm. this will never be deleted
        return new DurationDelegate();
    }
    return NULL;
}

QMimeData* LibraryTableModel::mimeData(const QModelIndexList &indexes) const {
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
