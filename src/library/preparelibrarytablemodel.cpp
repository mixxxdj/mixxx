#include <QObject>

#include "preparelibrarytablemodel.h"
#include "library/trackcollection.h"

const QString RECENT_FILTER = "datetime_added > datetime('now', '-7 days')";

PrepareLibraryTableModel::PrepareLibraryTableModel(QObject* parent,
                                                   TrackCollection* pTrackCollection)
        : LibraryTableModel(parent, pTrackCollection,
                            "mixxx.db.model.prepare") {
    m_bShowRecentSongs = true;
    setSearch("", RECENT_FILTER);

}


PrepareLibraryTableModel::~PrepareLibraryTableModel() {
}


void PrepareLibraryTableModel::showRecentSongs() {
   m_bShowRecentSongs = true;
   search(currentSearch());
}

void PrepareLibraryTableModel::showAllSongs() {
    m_bShowRecentSongs = false;
    search(currentSearch());
}
