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
    if (column == fieldIndex(LIBRARYTABLE_KEY))    
        return true;
    if(column == fieldIndex(LIBRARYTABLE_BITRATE))
        return true;
    
    return false;        
}