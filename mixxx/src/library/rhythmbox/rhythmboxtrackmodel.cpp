#include <QtCore>
#include <QtGui>
#include <QtSql>

#include "library/trackcollection.h"
#include "library/rhythmbox/rhythmboxtrackmodel.h"
#include "track/beatfactory.h"
#include "track/beats.h"

RhythmboxTrackModel::RhythmboxTrackModel(QObject* parent,
                                   TrackCollection* pTrackCollection)
        : BaseSqlTableModel(parent, pTrackCollection,
                            pTrackCollection->getDatabase(),
                            "mixxx.db.model.rhythmbox"),
          m_pTrackCollection(pTrackCollection),
          m_database(m_pTrackCollection->getDatabase()) {
    connect(this, SIGNAL(doSearch(const QString&)), this, SLOT(slotSearch(const QString&)));

    QStringList columns;
    columns << "id";
    setTable("rhythmbox_library", columns[0], columns,
             m_pTrackCollection->getTrackSource("rhythmbox"));
    setDefaultSort(fieldIndex("artist"), Qt::AscendingOrder);
    initHeaderData();
}

RhythmboxTrackModel::~RhythmboxTrackModel() {
}

TrackPointer RhythmboxTrackModel::getTrack(const QModelIndex& index) const {
    QString artist = index.sibling(index.row(), fieldIndex("artist")).data().toString();
    QString title = index.sibling(index.row(), fieldIndex("title")).data().toString();
    QString album = index.sibling(index.row(), fieldIndex("album")).data().toString();
    QString year = index.sibling(index.row(), fieldIndex("year")).data().toString();
    QString genre = index.sibling(index.row(), fieldIndex("genre")).data().toString();
    float bpm = index.sibling(index.row(), fieldIndex("bpm")).data().toString().toFloat();

    QString location = index.sibling(index.row(), fieldIndex("location")).data().toString();

    TrackInfoObject* pTrack = new TrackInfoObject(location);
    pTrack->setArtist(artist);
    pTrack->setTitle(title);
    pTrack->setAlbum(album);
    pTrack->setYear(year);
    pTrack->setGenre(genre);
    pTrack->setBpm(bpm);
    TrackPointer track(pTrack, &QObject::deleteLater);

    // If the track has a BPM, then give it a static beatgrid.
    if (bpm > 0) {
        BeatsPointer pBeats = BeatFactory::makeBeatGrid(track, bpm, 0);
        track->setBeats(pBeats);
    }

    return track;
}

void RhythmboxTrackModel::search(const QString& searchText) {
    // qDebug() << "RhythmboxTrackModel::search()" << searchText
    //          << QThread::currentThread();
    emit(doSearch(searchText));
}

void RhythmboxTrackModel::slotSearch(const QString& searchText) {
    BaseSqlTableModel::search(searchText);
}

bool RhythmboxTrackModel::isColumnInternal(int column) {
    if (column == fieldIndex(LIBRARYTABLE_ID)) {
        return true;
    }
    return false;
}

Qt::ItemFlags RhythmboxTrackModel::flags(const QModelIndex &index) const {
    return readOnlyFlags(index);
}

bool RhythmboxTrackModel::isColumnHiddenByDefault(int column) {
    return false;
}
