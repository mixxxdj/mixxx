#include "library/browse/browselibrarytablemodel.h"

#include "moc_browselibrarytablemodel.cpp"

namespace {

const QString kTableName = QStringLiteral("browse_library_view");
// Track filter: missing tracks
const QString kTrackFilter = QStringLiteral("(mixxx_deleted=0 AND fs_deleted=0)");

} // anonymous namespace

BrowseLibraryTableModel::BrowseLibraryTableModel(
        QObject* pParent,
        TrackCollectionManager* pTrackCollectionManager,
        const char* settingsNamespace)
        : LibraryTableModel(pParent, pTrackCollectionManager, settingsNamespace) {
}

void BrowseLibraryTableModel::setPath(QString path) {
    while (path.endsWith('/')) {
        path.chop(1);
    }
    // Note: depending on operator we get strict or recursive search:
    // dir:=path -> strict
    // dir:path  -> recursive
    // Note(ronso0) For now I'm using recursive mode since that is what almost
    // all my b2b DJs expected (coming from Serato or VDJ).
    // FIXME Default to strict mode and add strict/recursive toggle?
    setExtraFilter(QStringLiteral("dir:\"%1\"").arg(path));
}

TrackModel::Capabilities BrowseLibraryTableModel::getCapabilities() const {
    return Capability::AddToTrackSet |
            Capability::AddToAutoDJ |
            Capability::EditMetadata |
            Capability::LoadToDeck |
            Capability::LoadToSampler |
            Capability::LoadToPreviewDeck |
            Capability::ResetPlayed |
            Capability::RemoveFromDisk |
            Capability::Analyze |
            Capability::Properties |
            Capability::Sorting;
}
