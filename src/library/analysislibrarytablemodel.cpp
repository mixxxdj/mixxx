#include <QObject>

#include "analysislibrarytablemodel.h"
#include "library/trackcollection.h"

const QString RECENT_FILTER = "datetime_added > datetime('now', '-7 days')";

AnalysisLibraryTableModel::AnalysisLibraryTableModel(QObject* parent,
                                                   TrackCollection* pTrackCollection)
        : LibraryTableModel(parent, pTrackCollection,
                            "mixxx.db.model.prepare") {
    m_bShowRecentSongs = true;
    // tro's lambda idea
//    m_pTrackCollection->callAsync(
//                [this] (void) {
        setSearch("", RECENT_FILTER);
//    });
}


AnalysisLibraryTableModel::~AnalysisLibraryTableModel() {
}


void AnalysisLibraryTableModel::showRecentSongs() {
    m_bShowRecentSongs = true;
    search(currentSearch());
}

void AnalysisLibraryTableModel::showAllSongs() {
    m_bShowRecentSongs = false;
    search(currentSearch());
}
