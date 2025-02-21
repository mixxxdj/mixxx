#include "library/baseexternaltrackmodel.h"

#include <QSqlDatabase>
#include <QSqlQuery>

#include "library/dao/trackschema.h"
#include "library/queryutil.h"
#include "library/trackcollectionmanager.h"
#include "mixer/playermanager.h"
#include "moc_baseexternaltrackmodel.cpp"
#include "track/track.h"

BaseExternalTrackModel::BaseExternalTrackModel(QObject* parent,
        TrackCollectionManager* pTrackCollectionManager,
        const char* settingsNamespace,
        const QString& trackTable,
        QSharedPointer<BaseTrackCache> trackSource)
        : BaseSqlTableModel(parent, pTrackCollectionManager, settingsNamespace) {
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
        LOG_FAILED_QUERY(query) << "Error creating temporary view for" << trackTable;
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

    QString location = getTrackLocation(index);

    if (location.isEmpty()) {
        // Track is lost
        return TrackPointer();
    }

    bool track_already_in_library = false;
    TrackPointer pTrack = m_pTrackCollectionManager->getOrAddTrack(
            TrackRef::fromFilePath(location),
            &track_already_in_library);

    if (pTrack) {
        // If this track was not in the Mixxx library it is now added and will be
        // saved with the metadata from external library. If it was already in the
        // Mixxx library then we do not touch it so that we do not over-write the
        // user's metadata.
        if (!track_already_in_library) {
            pTrack->setArtist(artist);
            pTrack->setTitle(title);
            pTrack->setAlbum(album);
            pTrack->setYear(year);
            updateTrackGenre(pTrack.get(), genre);
            pTrack->trySetBpm(bpm);
        }
    } else {
        qWarning() << "Failed to load external track" << location;
    }
    return pTrack;
}

QString BaseExternalTrackModel::resolveLocation(const QString& nativeLocation) const {
    return QDir::fromNativeSeparators(nativeLocation);
}

TrackId BaseExternalTrackModel::getTrackId(const QModelIndex& index) const {
    const auto track = getTrack(index);
    if (track) {
        return track->getId();
    } else {
        return TrackId();
    }
}

QString BaseExternalTrackModel::getTrackLocation(const QModelIndex& index) const {
    QString nativeLocation = index.sibling(index.row(), fieldIndex("location")).data().toString();
    return resolveLocation(nativeLocation);
}

TrackId BaseExternalTrackModel::doGetTrackId(const TrackPointer& pTrack) const {
    if (pTrack) {
        // The external table has foreign Track IDs, so we need to compare
        // by location
        for (int row = 0; row < rowCount(); ++row) {
            QString nativeLocation = index(row, fieldIndex("location")).data().toString();
            QString location = resolveLocation(nativeLocation);
            if (location == pTrack->getLocation()) {
                return TrackId(index(row, 0).data());
            }
        }
    }
    return TrackId();
}

bool BaseExternalTrackModel::isColumnInternal(int column) {
    return column == fieldIndex(LIBRARYTABLE_ID) ||
            (PlayerManager::numPreviewDecks() == 0 &&
                    column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_PREVIEW));
}

Qt::ItemFlags BaseExternalTrackModel::flags(const QModelIndex& index) const {
    return readOnlyFlags(index);
}

TrackModel::Capabilities BaseExternalTrackModel::getCapabilities() const {
    return Capability::AddToTrackSet |
            Capability::AddToAutoDJ |
            Capability::LoadToDeck |
            Capability::LoadToPreviewDeck |
            Capability::LoadToSampler |
            Capability::Sorting;
}
