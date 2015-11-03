#include "library/baseexternaltrackmodel.h"
#include "library/trackcollection.h"
#include "playermanager.h"

BaseExternalTrackModel::BaseExternalTrackModel(QObject* parent,
                                               TrackCollection* pTrackCollection,
                                               const char* settingsNamespace,
                                               const QString& trackTable,
                                               QSharedPointer<BaseTrackCache> trackSource)
        : BaseSqlTableModel(parent, pTrackCollection, settingsNamespace),
          m_trackTable(trackTable) {
    //TODO(MK): Moved that out of init to constructor. Is this ok?
    QStringList columns;
    columns << "id";
    // TODO(XXX) preview column, needs a temporary view
    setTable(m_trackTable, columns[0], columns, trackSource);
    setDefaultSort(fieldIndex("artist"), Qt::AscendingOrder);
}

void BaseExternalTrackModel::init() {
}

BaseExternalTrackModel::~BaseExternalTrackModel() {
}

// must be called from Main thread
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
    int track_id = -1;
    // tro's lambda idea. This code calls synchronously!
    m_pTrackCollection->callSync(
            [this, &location, &track_already_in_library, &track_id] (TrackCollectionPrivate* pTrackCollectionPrivate) {
        TrackDAO& track_dao = pTrackCollectionPrivate->getTrackDAO();
        track_id = track_dao.getTrackId(location);
        track_already_in_library = track_id >= 0;
        if (track_id < 0) {
            // Add Track to library
            track_id = track_dao.addTrack(location, true);
        }
    }, __PRETTY_FUNCTION__);

    TrackPointer pTrack;
    if (track_id < 0) {
        // Add Track to library failed, create a transient TrackInfoObject
        pTrack = TrackPointer(new TrackInfoObject(location), &QObject::deleteLater);
    } else {
        // tro's lambda idea. This code calls synchronously!
        m_pTrackCollection->callSync(
                [this, &track_id, &pTrack] (TrackCollectionPrivate* pTrackCollectionPrivate) {
            TrackDAO& track_dao = pTrackCollectionPrivate->getTrackDAO();
            pTrack = track_dao.getTrack(track_id);
        });
    }

    // If this track was not in the Mixxx library it is now added and will be
    // saved with the metadata from iTunes. If it was already in the library
    // then we do not touch it so that we do not over-write the user's metadata.
    if (!track_already_in_library) {
        pTrack->setArtist(artist);
        pTrack->setTitle(title);
        pTrack->setAlbum(album);
        pTrack->setYear(year);
        pTrack->setGenre(genre);
        pTrack->setBpm(bpm);
    }
    return pTrack;
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

bool BaseExternalTrackModel::isColumnHiddenByDefault(int column) {
    Q_UNUSED(column);
    return false;
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
