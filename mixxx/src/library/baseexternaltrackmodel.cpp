#include "library/baseexternaltrackmodel.h"
#include "library/trackcollection.h"
#include "playermanager.h"

BaseExternalTrackModel::BaseExternalTrackModel(QObject* parent,
                                               TrackCollection* pTrackCollection,
                                               QString settingsNamespace,
                                               QString trackTable,
                                               QString trackSource)
        : BaseSqlTableModel(parent, pTrackCollection,
                            pTrackCollection->getDatabase(),
                            settingsNamespace),
          m_pTrackCollection(pTrackCollection),
          m_database(m_pTrackCollection->getDatabase()) {
    connect(this, SIGNAL(doSearch(const QString&)),
            this, SLOT(slotSearch(const QString&)));

    QStringList columns;
    columns << "id";
    // TODO(XXX) preview column, needs a temporary view

    setTable(trackTable, columns[0], columns,
             m_pTrackCollection->getTrackSource(trackSource));
    setDefaultSort(fieldIndex("artist"), Qt::AscendingOrder);
    initHeaderData();
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

    TrackDAO& track_dao = m_pTrackCollection->getTrackDAO();
    int track_id = track_dao.getTrackId(location);
    bool track_already_in_library = track_id >= 0;
    if (track_id < 0) {
        // Add Track to library
        track_id = track_dao.addTrack(location, true);
    }

    TrackPointer pTrack;

    if (track_id < 0) {
        // Add Track to library failed, create a transient TrackInfoObject
        pTrack = TrackPointer(new TrackInfoObject(location), &QObject::deleteLater);
    } else {
        pTrack = track_dao.getTrack(track_id);
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

void BaseExternalTrackModel::search(const QString& searchText) {
    // qDebug() << "BaseExternalTrackModel::search()" << searchText
    //          << QThread::currentThread();
    emit(doSearch(searchText));
}

void BaseExternalTrackModel::slotSearch(const QString& searchText) {
    BaseSqlTableModel::search(searchText);
}

bool BaseExternalTrackModel::isColumnInternal(int column) {
    // Used for preview deck widgets.
    if (column == fieldIndex(LIBRARYTABLE_ID) ||
        (PlayerManager::numPreviewDecks() == 0 && column == fieldIndex("preview"))) {
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
