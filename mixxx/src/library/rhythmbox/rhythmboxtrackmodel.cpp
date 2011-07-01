#include <QtCore>
#include <QtGui>
#include <QtSql>
#include "library/trackcollection.h"
#include "library/rhythmbox/rhythmboxtrackmodel.h"

#include "mixxxutils.cpp"

RhythmboxTrackModel::RhythmboxTrackModel(QObject* parent,
                                   TrackCollection* pTrackCollection)
        : TrackModel(pTrackCollection->getDatabase(),
                     "mixxx.db.model.rhythmbox"),
          BaseSqlTableModel(parent, pTrackCollection, pTrackCollection->getDatabase()),
          m_pTrackCollection(pTrackCollection),
          m_database(m_pTrackCollection->getDatabase()) {
    connect(this, SIGNAL(doSearch(const QString&)), this, SLOT(slotSearch(const QString&)));

    QStringList columns;
    columns << "id"
            << "artist"
            << "album"
            << "genre"
            << "location"
            << "comment"
            << "duration"
            << "bitrate"
            << "bpm"
            << "rating";
    setTable("rhythmbox_library", columns, "id");
    initHeaderData();
    initDefaultSearchColumns();
    setCaching(false);
}

RhythmboxTrackModel::~RhythmboxTrackModel() {
}

bool RhythmboxTrackModel::addTrack(const QModelIndex& index, QString location) {
    Q_UNUSED(index);
    Q_UNUSED(location)
	return false;
}

TrackPointer RhythmboxTrackModel::getTrack(const QModelIndex& index) const {
    QString artist = index.sibling(index.row(), fieldIndex("artist")).data().toString();
    QString title = index.sibling(index.row(), fieldIndex("title")).data().toString();
    QString album = index.sibling(index.row(), fieldIndex("album")).data().toString();
    QString year = index.sibling(index.row(), fieldIndex("year")).data().toString();
    QString genre = index.sibling(index.row(), fieldIndex("genre")).data().toString();
    float bpm = index.sibling(index.row(), fieldIndex("bpm")).data().toString().toFloat();

    QString location = index.sibling(index.row(), fieldIndex("location")).data().toString();

    if( location.isEmpty()) {
    	// Track is lost
    	return TrackPointer();
    }

    TrackDAO& track_dao = m_pTrackCollection->getTrackDAO();
    int track_id = track_dao.getTrackId(location);
    if (track_id < 0) {
    	// Add Track to library
    	track_id = track_dao.addTrack(location);
    }

    TrackPointer pTrack;

    if (track_id < 0) {
    	// Add Track to library failed
    	// Create own TrackInfoObject
    	pTrack = TrackPointer(new TrackInfoObject(location), &QObject::deleteLater);
    }
    else {
    	pTrack = track_dao.getTrack(track_id);
    }

    // Overwrite Metadata from Rythmbox library
    pTrack->setArtist(artist);
    pTrack->setTitle(title);
    pTrack->setAlbum(album);
    pTrack->setYear(year);
    pTrack->setGenre(genre);
    pTrack->setBpm(bpm);

    return pTrack;
}

QString RhythmboxTrackModel::getTrackLocation(const QModelIndex& index) const {
    QString location = index.sibling(index.row(), fieldIndex("location")).data().toString();
    return location;
}

int RhythmboxTrackModel::getTrackId(const QModelIndex& index) const {
    if (!index.isValid()) {
        return -1;
    }
    return index.sibling(index.row(), fieldIndex("id")).data().toInt();
}

const QLinkedList<int> RhythmboxTrackModel::getTrackRows(int trackId) const {
    return BaseSqlTableModel::getTrackRows(trackId);
}


void RhythmboxTrackModel::removeTrack(const QModelIndex& index) {
	Q_UNUSED(index);
}

void RhythmboxTrackModel::removeTracks(const QModelIndexList& indices) {
	Q_UNUSED(indices);
}

void RhythmboxTrackModel::moveTrack(const QModelIndex& sourceIndex, const QModelIndex& destIndex) {
	Q_UNUSED(sourceIndex);
	Q_UNUSED(destIndex);
}

void RhythmboxTrackModel::search(const QString& searchText) {
    // qDebug() << "RhythmboxTrackModel::search()" << searchText
    //          << QThread::currentThread();
    emit(doSearch(searchText));
}

void RhythmboxTrackModel::slotSearch(const QString& searchText) {
    BaseSqlTableModel::search(searchText);
}

const QString RhythmboxTrackModel::currentSearch() {
    return BaseSqlTableModel::currentSearch();
}

bool RhythmboxTrackModel::isColumnInternal(int column) {
    if (column == fieldIndex(LIBRARYTABLE_ID) ||
        column == fieldIndex(LIBRARYTABLE_MIXXXDELETED) ||
        column == fieldIndex(TRACKLOCATIONSTABLE_FSDELETED))
        return true;
    return false;
}

QMimeData* RhythmboxTrackModel::mimeData(const QModelIndexList &indexes) const {
    QMimeData *mimeData = new QMimeData();
    QList<QUrl> urls;

    //Ok, so the list of indexes we're given contains separates indexes for
    //each column, so even if only one row is selected, we'll have like 7 indexes.
    //We need to only count each row once:
    QList<int> rows;

    foreach (QModelIndex index, indexes) {
        if (index.isValid()) {
            if (!rows.contains(index.row())) {
                rows.push_back(index.row());
                QUrl url = QUrl::fromLocalFile(getTrackLocation(index));
                if (!url.isValid())
                    qDebug() << "ERROR invalid url\n";
                else
                    urls.append(url);
            }
        }
    }
    mimeData->setUrls(urls);
    return mimeData;
}


QItemDelegate* RhythmboxTrackModel::delegateForColumn(const int i) {
	Q_UNUSED(i);
    return NULL;
}

TrackModel::CapabilitiesFlags RhythmboxTrackModel::getCapabilities() const {
    return TRACKMODELCAPS_NONE |
     	   TRACKMODELCAPS_ADDTOAUTODJ;
}

Qt::ItemFlags RhythmboxTrackModel::flags(const QModelIndex &index) const {
    return readOnlyFlags(index);
}

bool RhythmboxTrackModel::isColumnHiddenByDefault(int column) {
    Q_UNUSED(column);
	return false;
}
