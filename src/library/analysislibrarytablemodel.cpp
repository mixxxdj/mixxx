#include <QObject>

#include "analysislibrarytablemodel.h"
#include "library/trackcollection.h"

const QString RECENT_FILTER = "datetime_added > datetime('now', '-7 days')";

AnalysisLibraryTableModel::AnalysisLibraryTableModel(QObject* parent,
                                                   TrackCollection* pTrackCollection)
        : LibraryTableModel(parent, pTrackCollection,
                            "mixxx.db.model.prepare") {
    // Default to showing recent tracks.
    setSearch("", RECENT_FILTER);
}


AnalysisLibraryTableModel::~AnalysisLibraryTableModel() {
}


void AnalysisLibraryTableModel::showRecentSongs() {
    // Search with the recent filter.
    search(currentSearch(), RECENT_FILTER);
}

void AnalysisLibraryTableModel::showAllSongs() {
    // Clear the recent filter.
    search(currentSearch(), "");
}
