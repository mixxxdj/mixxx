#include <QObject>
#include "preparelibrarytablemodel.h"

const QString RECENT_FILTER = "datetime_added > datetime('now', '-7 days')";

PrepareLibraryTableModel::PrepareLibraryTableModel(QObject* parent, TrackCollection* pTrackCollection) : LibraryTableModel(parent, pTrackCollection)
{
    m_bShowRecentSongs = true;
    search("");
    select();
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
    m_currentSearch = searchText;
    QString baseFilter;
    if (m_bShowRecentSongs)
        baseFilter = DEFAULT_LIBRARYFILTER + " AND " + RECENT_FILTER;
    else
        baseFilter = DEFAULT_LIBRARYFILTER;

    if (searchText == "")
        setFilter(baseFilter);
    else
        setFilter(baseFilter + " " +   
                  "artist LIKE \'%" + searchText + "%\' OR "
                  "title  LIKE \'%" + searchText + "%\'");
}

void PrepareLibraryTableModel::showRecentSongs()
{
   m_bShowRecentSongs = true; 
   search("");
   select();
}

void PrepareLibraryTableModel::showAllSongs()
{
    m_bShowRecentSongs = false;
    search("");
    select();
}


void PrepareLibraryTableModel::updateTracks(QModelIndexList& indices)
{
    QModelIndex current;
    foreach(current, indices)
    {
        emit(dataChanged(current, current));
    }
}
