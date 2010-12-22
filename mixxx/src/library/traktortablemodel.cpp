#include <QtCore>
#include <QtGui>
#include <QtSql>
#include "library/trackcollection.h"
#include "library/traktortablemodel.h"

#include "mixxxutils.cpp"

TraktorTableModel::TraktorTableModel(QObject* parent,
                                       TrackCollection* pTrackCollection)
        : TrackModel(pTrackCollection->getDatabase(),
                     "mixxx.db.model.traktor_tablemodel"),
          BaseSqlTableModel(parent, pTrackCollection, pTrackCollection->getDatabase()),
          m_pTrackCollection(pTrackCollection),
		  m_database(m_pTrackCollection->getDatabase())
          
{
    connect(this, SIGNAL(doSearch(const QString&)), this, SLOT(slotSearch(const QString&)));
	setTable("traktor_library");
    initHeaderData();
}

TraktorTableModel::~TraktorTableModel() {
}

bool TraktorTableModel::addTrack(const QModelIndex& index, QString location)
{

    return false;
}

TrackPointer TraktorTableModel::getTrack(const QModelIndex& index) const
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

QString TraktorTableModel::getTrackLocation(const QModelIndex& index) const
{
    QString location = index.sibling(index.row(), fieldIndex("location")).data().toString();
    return location;
}

void TraktorTableModel::removeTrack(const QModelIndex& index)
{
    
}

void TraktorTableModel::removeTracks(const QModelIndexList& indices) {

}
void TraktorTableModel::moveTrack(const QModelIndex& sourceIndex, const QModelIndex& destIndex)
{
   
}

void TraktorTableModel::search(const QString& searchText) {
    // qDebug() << "TraktorTableModel::search()" << searchText
    //          << QThread::currentThread();
    emit(doSearch(searchText));
}

void TraktorTableModel::slotSearch(const QString& searchText)
{
   if (!m_currentSearch.isNull() && m_currentSearch == searchText)
        return;
    m_currentSearch = searchText;

    QString filter;
    QSqlField search("search", QVariant::String);
    search.setValue("%" + searchText + "%");
    QString escapedText = database().driver()->formatValue(search);
    filter = "(artist LIKE " + escapedText + " OR " +
                "album LIKE " + escapedText + " OR " +
                "title  LIKE " + escapedText + ")";
    setFilter(filter);
  
}

const QString TraktorTableModel::currentSearch() {
    return m_currentSearch;
}

bool TraktorTableModel::isColumnInternal(int column) {
    if (column == fieldIndex(LIBRARYTABLE_ID) ||
        column == fieldIndex(LIBRARYTABLE_MIXXXDELETED) ||
        column == fieldIndex(TRACKLOCATIONSTABLE_FSDELETED))
        return true;
    return false;
}

QMimeData* TraktorTableModel::mimeData(const QModelIndexList &indexes) const {
    
    return NULL;
}


QItemDelegate* TraktorTableModel::delegateForColumn(const int i) {
    return NULL;
}

TrackModel::CapabilitiesFlags TraktorTableModel::getCapabilities() const
{
  
    return NULL;
}

Qt::ItemFlags TraktorTableModel::flags(const QModelIndex &index) const
{
    return readOnlyFlags(index);
}
bool TraktorTableModel::isColumnHiddenByDefault(int column) {
    return false;
}