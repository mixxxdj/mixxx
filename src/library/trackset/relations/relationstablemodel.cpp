#include "library/trackset/relations/relationstablemodel.h"

#include "library/dao/trackschema.h"
#include "library/queryutil.h"
#include "library/trackcollection.h"
#include "library/trackcollectionmanager.h"
#include "mixer/playerinfo.h"
#include "moc_relationstablemodel.cpp"
#include "track/track.h"

namespace {

const QString kModelName = QStringLiteral("relations");

} // anonymous namespace

RelationsTableModel::RelationsTableModel(
        QObject* parent,
        TrackCollectionManager* pTrackCollectionManager)
        : TrackSetTableModel(
                  parent,
                  pTrackCollectionManager,
                  "mixxx.db.model.relations") {
}

void RelationsTableModel::displayTrackTargets(TrackPointer pTrack) {
    if (!pTrack) {
        return;
    }
    m_pTrack = pTrack;
    TrackId trackId = pTrack->getId();
    QString trackTableName = QStringLiteral("relations_track_%1").arg(trackId.toString());

    QStringList columns;
    columns << "library." + LIBRARYTABLE_ID
            << "'' AS " + LIBRARYTABLE_PREVIEW
            // For sorting the cover art column we give LIBRARYTABLE_COVERART
            // the same value as the cover digest.
            << LIBRARYTABLE_COVERART_DIGEST + " AS " + LIBRARYTABLE_COVERART;

    QSqlQuery query(m_database);
    QString queryString = QStringLiteral(
            "CREATE TEMPORARY VIEW IF NOT EXISTS %1 AS "
            "SELECT %2 "
            "FROM library "
            "INNER JOIN relations r "
            "ON (r.source_track_id = %3 AND library.id = r.target_track_id) "
            "OR (r.target_track_id = %3 AND r.bidirectional = 1 AND library.id "
            "= r.source_track_id)")
                                  .arg(trackTableName,
                                          columns.join(','),
                                          trackId.toString());
    query.prepare(queryString);
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
    }

    columns[0] = LIBRARYTABLE_ID;
    columns[1] = LIBRARYTABLE_PREVIEW;
    columns[2] = LIBRARYTABLE_COVERART;
    setTable(trackTableName,
            LIBRARYTABLE_ID,
            columns,
            m_pTrackCollectionManager->internalCollection()->getTrackSource());
    setSearch("");
    setDefaultSort(fieldIndex("artist"), Qt::AscendingOrder);
}

TrackModel::Capabilities RelationsTableModel::getCapabilities() const {
    Capabilities caps =
            Capability::AddToTrackSet |
            Capability::EditMetadata |
            Capability::LoadToDeck |
            Capability::LoadToSampler |
            Capability::LoadToPreviewDeck |
            Capability::ResetPlayed |
            Capability::Hide |
            Capability::Analyze |
            Capability::Properties |
            Capability::Sorting;

    return caps;
}

QString RelationsTableModel::modelKey(bool noSearch) const {
    if (m_pTrack) {
        if (noSearch) {
            return kModelName + QChar(':') + m_pTrack->getId().toString();
        }
        return kModelName + QChar(':') +
                m_pTrack->getId().toString() +
                QChar('#') +
                currentSearch();
    } else {
        if (noSearch) {
            return kModelName;
        }
        return kModelName + QChar('#') +
                currentSearch();
    }
}
