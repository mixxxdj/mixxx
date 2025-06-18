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
        const char* settingsNamespace)
        : LibraryTableModel(pParent,
                  pTrackCollectionManager,
                  settingsNamespace) {
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
    // Note: don't use 'LIKE' to find matches, that would interpret trailing _
    // as wildcard, but for some reason that would not match _
    // Use '=' which is reported to be more efficient anyway.
    m_directoryFilter = QStringLiteral(
            "%1 IN (SELECT %2.%1 "
            "FROM %2 JOIN %3 "
            "ON %2.%4=%3.id "
            "WHERE %3.%5 = %6)")
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
