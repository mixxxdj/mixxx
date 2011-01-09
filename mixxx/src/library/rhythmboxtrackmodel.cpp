#include <QtCore>
#include <QtGui>
#include <QtSql>
#include "library/trackcollection.h"
#include "library/rhythmboxtrackmodel.h"

#include "mixxxutils.cpp"

RhythmboxTrackModel::RhythmboxTrackModel(QObject* parent,
                                   TrackCollection* pTrackCollection)
        : TrackModel(pTrackCollection->getDatabase(),
                     "mixxx.db.model.rhythmbox"),
          BaseSqlTableModel(parent, pTrackCollection, pTrackCollection->getDatabase()),
          m_pTrackCollection(pTrackCollection),
          m_database(m_pTrackCollection->getDatabase()) {
    connect(this, SIGNAL(doSearch(const QString&)), this, SLOT(slotSearch(const QString&)));
    setTable("rhythmbox_library");
    initHeaderData();
}

RhythmboxTrackModel::~RhythmboxTrackModel() {
}

bool RhythmboxTrackModel::addTrack(const QModelIndex& index, QString location) {
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

    TrackInfoObject* pTrack = new TrackInfoObject(location);
    pTrack->setArtist(artist);
    pTrack->setTitle(title);
    pTrack->setAlbum(album);
    pTrack->setYear(year);
    pTrack->setGenre(genre);
    pTrack->setBpm(bpm);

    return TrackPointer(pTrack, &QObject::deleteLater);
}

QString RhythmboxTrackModel::getTrackLocation(const QModelIndex& index) const {
    QString location = index.sibling(index.row(), fieldIndex("location")).data().toString();
    return location;
}

void RhythmboxTrackModel::removeTrack(const QModelIndex& index) {

}

void RhythmboxTrackModel::removeTracks(const QModelIndexList& indices) {

}

void RhythmboxTrackModel::moveTrack(const QModelIndex& sourceIndex, const QModelIndex& destIndex) {

}

void RhythmboxTrackModel::search(const QString& searchText) {
    // qDebug() << "RhythmboxTrackModel::search()" << searchText
    //          << QThread::currentThread();
    emit(doSearch(searchText));
}

void RhythmboxTrackModel::slotSearch(const QString& searchText) {
    if (!m_currentSearch.isNull() && m_currentSearch == searchText)
        return;
    m_currentSearch = searchText;

    QString filter;
    QSqlField search("search", QVariant::String);
    search.setValue("%" + searchText + "%");
    QString escapedText = database().driver()->formatValue(search);
    filter = "(artist LIKE " + escapedText + " OR " +
            "album LIKE " + escapedText + " OR " +
            "title  LIKE " + escapedText + ")";
    setFilter(filter);

}

const QString RhythmboxTrackModel::currentSearch() {
    return m_currentSearch;
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
    return NULL;
}

TrackModel::CapabilitiesFlags RhythmboxTrackModel::getCapabilities() const {
    return NULL;
}

Qt::ItemFlags RhythmboxTrackModel::flags(const QModelIndex &index) const {
    return readOnlyFlags(index);
}

bool RhythmboxTrackModel::isColumnHiddenByDefault(int column) {
    return false;
}
