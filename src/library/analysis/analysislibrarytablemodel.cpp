#include "library/analysis/analysislibrarytablemodel.h"

#include <QDateTime>

#include "moc_analysislibrarytablemodel.cpp"
#include "util/datetime.h"

namespace {

inline QString recentFilter(unsigned int days) {
    // Create a user-formatted query equal to SQL query
    // datetime_added > datetime('now', '-<days> days')
    QDateTime dt(QDateTime::currentDateTimeUtc());
    dt = dt.addDays(-static_cast<qint64>(days));
    const QString dateStr = dt.date().toString(Qt::ISODate);

    // FIXME alternatively, use "added:-days" and add the respective
    // literal parser to DateAddedFilterNode
    return QStringLiteral("added:>%1").arg(dateStr);
}

} // anonymous namespace

AnalysisLibraryTableModel::AnalysisLibraryTableModel(
        QObject* parent,
        TrackCollectionManager* pTrackCollectionManager)
        : LibraryTableModel(parent, pTrackCollectionManager, "mixxx.db.model.prepare"),
          m_recentDays(kDefaultRecentDays) {
    // Default to showing recent tracks.
    setExtraFilter(recentFilter(m_recentDays));
}

void AnalysisLibraryTableModel::setRecentDays(unsigned int days) {
    m_recentDays = days;
}

void AnalysisLibraryTableModel::showRecentSongs() {
    // Search with the recent filter using internal days value.
    setExtraFilter(recentFilter(m_recentDays));
    search(currentSearch());
}

void AnalysisLibraryTableModel::showAllSongs() {
    // Clear the recent filter.
    setExtraFilter({});
    search(currentSearch());
}

void AnalysisLibraryTableModel::searchCurrentTrackSet(
        const QString& text,
        bool useRecentFilter) {
    setExtraFilter(useRecentFilter ? recentFilter(m_recentDays) : QString{});
    search(text);
}
