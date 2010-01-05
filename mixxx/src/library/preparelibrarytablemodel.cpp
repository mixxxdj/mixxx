#include <QObject>

#include "preparelibrarytablemodel.h"
#include "library/trackcollection.h"

const QString RECENT_FILTER = "datetime_added > datetime('now', '-7 days')";

PrepareLibraryTableModel::PrepareLibraryTableModel(QObject* parent,
                                                   TrackCollection* pTrackCollection)
        : TrackModel(pTrackCollection->getDatabase(),
                     "mixxx.db.model.prepare"),
          LibraryTableModel(parent, pTrackCollection) {

    m_bShowRecentSongs = true;
    slotSearch("");
    select();

    //XXX: Fetch the entire result set to allow the database to unlock. --
    //Albert Nov 29/09
    while (canFetchMore())
        fetchMore();

    connect(this, SIGNAL(doSearch(const QString&)),
            this, SLOT(slotSearch(const QString&)));
}


PrepareLibraryTableModel::~PrepareLibraryTableModel()
{

}

bool PrepareLibraryTableModel::isColumnInternal(int column) {
    bool result = false;

    if ((column == fieldIndex(LIBRARYTABLE_DATETIMEADDED))) {
        result = false;
    }
    else
        result = LibraryTableModel::isColumnInternal(column);

    return result;
}

void PrepareLibraryTableModel::search(const QString& searchText) {
    qDebug() << "PrepareLibraryTableModel::search()" << searchText
             << QThread::currentThread();
    emit(doSearch(searchText));
}

void PrepareLibraryTableModel::slotSearch(const QString& searchText)
{
    m_currentSearch = searchText;
    QString baseFilter;
    if (m_bShowRecentSongs)
        baseFilter = DEFAULT_LIBRARYFILTER + " AND " + RECENT_FILTER;
    else
        baseFilter = DEFAULT_LIBRARYFILTER;

    QString filter;
    if (searchText == "")
        filter = baseFilter;
    else {
        QSqlField search("search", QVariant::String);
        search.setValue("%" + searchText + "%");
        QString escapedText = database().driver()->formatValue(search);
        filter = "(" + baseFilter + " AND " +
                "(artist LIKE " + escapedText + " OR "
                "title  LIKE " + escapedText + "))";
    }
    setFilter(filter);

    // setFilter() calls select() implicitly, so we have to fetchMore to prevent
    // locking the database.

    //XXX: Fetch the entire result set to allow the database to unlock. --
    //Albert Nov 29/09
    while (canFetchMore())
        fetchMore();
}

void PrepareLibraryTableModel::showRecentSongs()
{
   m_bShowRecentSongs = true;
   search(m_currentSearch);
   select();

   //XXX: Fetch the entire result set to allow the database to unlock. --
   //Albert Nov 29/09
   while (canFetchMore())
       fetchMore();
}

void PrepareLibraryTableModel::showAllSongs()
{
    m_bShowRecentSongs = false;
    search(m_currentSearch);
    select();

    //XXX: Fetch the entire result set to allow the database to unlock. --
    //Albert Nov 29/09
    while (canFetchMore())
        fetchMore();
}


void PrepareLibraryTableModel::updateTracks(QModelIndexList& indices)
{
    QModelIndex current;
    foreach(current, indices)
    {
        emit(dataChanged(current, current));
    }
}
