#include "library/browse/browselibrarytablemodel.h"

#include "library/dao/trackschema.h"
#include "library/queryutil.h"
#include "library/trackcollection.h"
#include "library/trackcollectionmanager.h"
#include "moc_browselibrarytablemodel.cpp"
#include "recording/recordingmanager.h"

BrowseLibraryTableModel::BrowseLibraryTableModel(
        QObject* pParent,
        TrackCollectionManager* pTrackCollectionManager,
        RecordingManager* pRecordingManager,
        const char* settingsNamespace)
        : LibraryTableModel(pParent,
                  pTrackCollectionManager,
                  settingsNamespace),
          m_pRecordingManager(pRecordingManager) {
}

void BrowseLibraryTableModel::setPath(QString path) {
    while (path.endsWith('/')) {
        path.chop(1);
    }
    FieldEscaper escaper(m_pTrackCollectionManager->internalCollection()->database());
    // Note: don't use leading or trailing '%' for the LIKE pattern. We only want
    // tracks from exactly that path, not from it's children or any other path
    // that coincidentally contains 'path'.
    const QString escapedDirPath = escaper.escapeString(path);
    m_directoryFilter = QStringLiteral(
            "%1 IN (SELECT %2.%1 "
            "FROM %2 JOIN %3 "
            "ON %2.%4=%3.id "
            "WHERE %3.%5 LIKE %6)")
                                .arg(LIBRARYTABLE_ID,
                                        LIBRARY_TABLE,
                                        TRACKLOCATIONS_TABLE,
                                        LIBRARYTABLE_LOCATION,
                                        TRACKLOCATIONSTABLE_DIRECTORY,
                                        escapedDirPath);
}

void BrowseLibraryTableModel::search(const QString& searchText, const QString& /* extraFilter */) {
    LibraryTableModel::search(searchText, m_directoryFilter);
}

TrackPointer BrowseLibraryTableModel::getTrackByRef(const TrackRef& trackRef) const {
    if (m_pRecordingManager->getRecordingLocation() == trackRef.getLocation()) {
        QMessageBox::critical(nullptr,
                tr("Mixxx Library"),
                tr("Could not load the following file because it is in use by "
                   "Mixxx or another application.") +
                        "\n" + trackRef.getLocation());
        return TrackPointer();
    }
    // TODO Comment from BrowseTableModel::getTrackByRef
    // NOTE(uklotzde, 2015-12-08): Accessing tracks from the browse view
    // will implicitly add them to the library. Is this really what we
    // want here??
    // NOTE(rryan, 2015-12-27): This was intentional at the time since
    // some people use Browse instead of the library and we want to let
    // them edit the tracks in a way that persists across sessions
    // and we didn't want to edit the files on disk by default
    // unless the user opts in to that.
    return m_pTrackCollectionManager->getOrAddTrack(trackRef);
}
