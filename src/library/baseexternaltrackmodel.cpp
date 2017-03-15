#include "library/baseexternaltrackmodel.h"
#include "library/trackcollection.h"
#include "library/queryutil.h"
#include "playermanager.h"

BaseExternalTrackModel::BaseExternalTrackModel(QObject* parent,
                                               TrackCollection* pTrackCollection,
                                               const char* settingsNamespace,
                                               const QString& trackTable,
                                               QSharedPointer<BaseTrackCache> trackSource)
        : BaseSqlTableModel(parent, pTrackCollection, settingsNamespace) {
    QString viewTable = trackTable + "_view";
    QStringList columns;
    columns << "id";
    columns << "'' AS " + LIBRARYTABLE_PREVIEW;

    QSqlQuery query(m_database);
    FieldEscaper f(m_database);
    QString queryString = QString(
        "CREATE TEMPORARY VIEW IF NOT EXISTS %1 AS "
        "SELECT %2 FROM %3")
            .arg(f.escapeString(viewTable),
                 columns.join(","),
                 f.escapeString(trackTable));
    query.prepare(queryString);

    if (!query.exec()) {
        LOG_FAILED_QUERY(query) <<
                "Error creating temporary view for" << trackTable;
        return;
    }

    columns[1] = LIBRARYTABLE_PREVIEW;
    setTable(viewTable, columns[0], columns, trackSource);
    setDefaultSort(fieldIndex("artist"), Qt::AscendingOrder);
}

BaseExternalTrackModel::~BaseExternalTrackModel() {
}

TrackPointer BaseExternalTrackModel::getTrack(const QModelIndex& index) const {
    QString artist = index.sibling(index.row(), fieldIndex("artist")).data().toString();
    QString title = index.sibling(index.row(), fieldIndex("title")).data().toString();
    QString album = index.sibling(index.row(), fieldIndex("album")).data().toString();
    QString year = index.sibling(index.row(), fieldIndex("year")).data().toString();
    QString genre = index.sibling(index.row(), fieldIndex("genre")).data().toString();
    float bpm = index.sibling(index.row(), fieldIndex("bpm")).data().toString().toFloat();

    QString location = index.sibling(index.row(), fieldIndex("location")).data().toString();

    if (location.isEmpty()) {
        // Track is lost
        return TrackPointer();
    }

    bool track_already_in_library = false;
    TrackPointer pTrack = m_pTrackCollection->getTrackDAO()
            .getOrAddTrack(location, true, &track_already_in_library);

    // If this track was not in the Mixxx library it is now added and will be
    // saved with the metadata from iTunes. If it was already in the library
    // then we do not touch it so that we do not over-write the user's metadata.
    if (pTrack && !track_already_in_library) {
        pTrack->setArtist(artist);
        pTrack->setTitle(title);
        pTrack->setAlbum(album);
        pTrack->setYear(year);
        pTrack->setGenre(genre);
        pTrack->setBpm(bpm);
    }
    return pTrack;
}

void BaseExternalTrackModel::trackLoaded(QString group, TrackPointer pTrack) {
    if (group == m_previewDeckGroup) {
        // If there was a previously loaded track, refresh its rows so the
        // preview state will update.
        if (m_previewDeckTrackId.isValid()) {
            const int numColumns = columnCount();
            QLinkedList<int> rows = getTrackRows(m_previewDeckTrackId);
            m_previewDeckTrackId = TrackId(); // invalidate
            foreach (int row, rows) {
                QModelIndex left = index(row, 0);
                QModelIndex right = index(row, numColumns);
                emit(dataChanged(left, right));
            }
        }
        if (pTrack) {
            // The external table has foreign Track IDs, so we need to compare
            // by location
            for (int row = 0; row < rowCount(); ++row) {
                QString location = index(row, fieldIndex("location")).data().toString();
                if (location == pTrack->getLocation()) {
                    m_previewDeckTrackId = TrackId(index(row, 0).data());
                    //qDebug() << "foreign track id" << m_previewDeckTrackId;
                    break;
                }
            }
        }
    }
}

bool BaseExternalTrackModel::isColumnInternal(int column) {
    // Used for preview deck widgets.
    if (column == fieldIndex(LIBRARYTABLE_ID) ||
            (PlayerManager::numPreviewDecks() == 0 &&
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_PREVIEW))) {
        return true;
    }
    return false;
}

Qt::ItemFlags BaseExternalTrackModel::flags(const QModelIndex &index) const {
    return readOnlyFlags(index);
}

TrackModel::CapabilitiesFlags BaseExternalTrackModel::getCapabilities() const {
    // See src/library/trackmodel.h for the list of TRACKMODELCAPS
    return TRACKMODELCAPS_NONE
            | TRACKMODELCAPS_ADDTOPLAYLIST
            | TRACKMODELCAPS_ADDTOCRATE
            | TRACKMODELCAPS_ADDTOAUTODJ
            | TRACKMODELCAPS_LOADTODECK
            | TRACKMODELCAPS_LOADTOPREVIEWDECK
            | TRACKMODELCAPS_LOADTOSAMPLER;
}
