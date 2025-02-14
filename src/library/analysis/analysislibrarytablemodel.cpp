#include "library/analysis/analysislibrarytablemodel.h"

#include "moc_analysislibrarytablemodel.cpp"

namespace {

const QString RECENT_FILTER = "datetime_added > datetime('now', '-7 days')";

} // anonymous namespace

AnalysisLibraryTableModel::AnalysisLibraryTableModel(QObject* parent,
                                                   TrackCollectionManager* pTrackCollectionManager)
        : LibraryTableModel(parent, pTrackCollectionManager,
                            "mixxx.db.model.prepare") {
    // Default to showing recent tracks.
    setSearch("", RECENT_FILTER);
}

void AnalysisLibraryTableModel::showRecentSongs() {
    // Search with the recent filter.
    search(currentSearch(), RECENT_FILTER);
}

void AnalysisLibraryTableModel::showAllSongs() {
    // Clear the recent filter.
    search(currentSearch(), "");
}

void AnalysisLibraryTableModel::searchCurrentTrackSet(const QString& text, bool useRecentFilter) {
    search(text, useRecentFilter ? RECENT_FILTER : "");
}
