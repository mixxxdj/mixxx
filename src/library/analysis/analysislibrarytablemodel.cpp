#include "library/analysis/analysislibrarytablemodel.h"

#include <QDateTime>

#include "moc_analysislibrarytablemodel.cpp"
#include "util/datetime.h"

namespace {

inline QString recentFilter(int days) {
    // Create a user-formatted query equal to SQL query
    // datetime_added > datetime('now', '-7 days')
    QDateTime dt(QDateTime::currentDateTimeUtc());
    dt = dt.addDays(-days);
    const QString dateStr = QLocale::system().toString(dt.date(), QLocale::ShortFormat);

    // FIXME alternatively, use "added:-days" and add the respective
    // literal parser to DateAddedFilterNode
    return QStringLiteral("added:>%1").arg(dateStr);
}

} // anonymous namespace

AnalysisLibraryTableModel::AnalysisLibraryTableModel(QObject* parent,
        TrackCollectionManager* pTrackCollectionManager)
        : LibraryTableModel(parent, pTrackCollectionManager, "mixxx.db.model.prepare") {
    // Default to showing recent tracks.
    setExtraFilter(recentFilter(7));
}

void AnalysisLibraryTableModel::showRecentSongs(int days) {
    // Search with the recent filter.
    setExtraFilter(recentFilter(days));
    select();
}

void AnalysisLibraryTableModel::showAllSongs() {
    // Clear the recent filter.
    setExtraFilter({});
    select();
}

void AnalysisLibraryTableModel::searchCurrentTrackSet(
        const QString& text, bool useRecentFilter, int days) {
    setExtraFilter(useRecentFilter ? recentFilter(days) : QString{});
    search(text);
}
