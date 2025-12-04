#include "library/analysis/analysislibrarytablemodel.h"

#include <QDateTime>

#include "moc_analysislibrarytablemodel.cpp"
#include "util/datetime.h"

namespace {

inline QString recentFilter() {
    // Create a user-formatted query equal to SQL query
    // datetime_added > datetime('now', '-7 days')
    QDateTime dt(QDateTime::currentDateTimeUtc());
    dt = dt.addDays(-7);
    const QString dateStr = QLocale::system().toString(dt.date(), QLocale::ShortFormat);

    // FIXME alternatively, use "added:-days" and add the respective
    // literal parser to DateAddedFilterNode
    return QStringLiteral("added:>%1").arg(dateStr);
}

} // anonymous namespace

AnalysisLibraryTableModel::AnalysisLibraryTableModel(QObject* parent,
                                                   TrackCollectionManager* pTrackCollectionManager)
        : LibraryTableModel(parent, pTrackCollectionManager,
                            "mixxx.db.model.prepare") {
    // Default to showing recent tracks.
    setSearch("", recentFilter());
}

void AnalysisLibraryTableModel::showRecentSongs() {
    // Search with the recent filter.
    search(currentSearch(), recentFilter());
}

void AnalysisLibraryTableModel::showAllSongs() {
    // Clear the recent filter.
    search(currentSearch(), "");
}

void AnalysisLibraryTableModel::searchCurrentTrackSet(const QString& text, bool useRecentFilter) {
    search(text, useRecentFilter ? recentFilter() : "");
}
