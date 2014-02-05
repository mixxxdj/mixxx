#include <QtAlgorithms>
#include <QtDebug>
#include <QTime>

#include "library/ipod/ipodplaylistmodel.h"
#include "mixxxutils.cpp"
#include "library/starrating.h"
#include "library/stardelegate.h"
#include "library/previewbuttondelegate.h"
#include "track/beatfactory.h"
#include "track/beats.h"
#include "playermanager.h"

extern "C" {
#include <glib-object.h> // g_type_init
}

const bool sDebug = false;

IPodPlaylistModel::IPodPlaylistModel(QObject* pParent, TrackCollection* pTrackCollection)
        :  TrackModel(pTrackCollection->getDatabase(),
                "mixxx.db.model.ipod_playlist"),
           QAbstractTableModel(pParent),
           m_iSortColumn(0),
           m_eSortOrder(Qt::AscendingOrder),
           m_currentSearch(""),
           m_pTrackCollection(pTrackCollection),
           m_trackDAO(m_pTrackCollection->getTrackDAO()),
           m_pPlaylist(NULL) {
    initHeaderData();
}

IPodPlaylistModel::~IPodPlaylistModel() {

}

void IPodPlaylistModel::initHeaderData() {
    // Set the column heading labels, rename them for translations and have
    // proper capitalization

    m_headerList.append(qMakePair(QString(tr("#")),          (size_t)0xffff));
    m_headerList.append(qMakePair(QString(tr("Artist")),     offsetof(Itdb_Track, artist)));
    m_headerList.append(qMakePair(QString(tr("Title")),      offsetof(Itdb_Track, title)));
    m_headerList.append(qMakePair(QString(tr("Album")),      offsetof(Itdb_Track, album)));
    m_headerList.append(qMakePair(QString(tr("Year")),       offsetof(Itdb_Track, year)));
    m_headerList.append(qMakePair(QString(tr("Duration")),   offsetof(Itdb_Track, tracklen)));
    m_headerList.append(qMakePair(QString(tr("Rating")),     offsetof(Itdb_Track, rating)));
    m_headerList.append(qMakePair(QString(tr("Genre")),      offsetof(Itdb_Track, genre)));
    m_headerList.append(qMakePair(QString(tr("Type")),       offsetof(Itdb_Track, filetype)));
    m_headerList.append(qMakePair(QString(tr("Track #")),    offsetof(Itdb_Track, track_nr)));
    m_headerList.append(qMakePair(QString(tr("Date Added")), offsetof(Itdb_Track, time_added)));
    m_headerList.append(qMakePair(QString(tr("BPM")),        offsetof(Itdb_Track, BPM)));
    m_headerList.append(qMakePair(QString(tr("Bitrate")),    offsetof(Itdb_Track, bitrate)));
    m_headerList.append(qMakePair(QString(tr("Location")),   offsetof(Itdb_Track, ipod_path)));
    m_headerList.append(qMakePair(QString(tr("Comment")),    offsetof(Itdb_Track, comment)));
}

void IPodPlaylistModel::initDefaultSearchColumns() {
    QStringList searchColumns;
    searchColumns << "artist"
                  << "album"
                  << "location"
                  << "comment"
                  << "title";
    setSearchColumns(searchColumns);
}

void IPodPlaylistModel::setSearchColumns(const QStringList& searchColumns) {
    m_searchColumns = searchColumns;

    // Convert all the search column names to their field indexes because we use
    // them a bunch.
    m_searchColumnIndices.resize(m_searchColumns.size());
    for (int i = 0; i < m_searchColumns.size(); ++i) {
        m_searchColumnIndices[i] = fieldIndex(m_searchColumns[i]);
    }
}

QVariant IPodPlaylistModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole) {
        return QAbstractTableModel::headerData(section, orientation, role);
    }

    if (orientation == Qt::Horizontal &&
            role == Qt::DisplayRole &&
            section < m_headerList.size()) {
        return QVariant(m_headerList.at(section).first);
    }
    return QAbstractTableModel::headerData(section, orientation, role);
}

const QString IPodPlaylistModel::currentSearch() const {
    return m_currentSearch;
}

void IPodPlaylistModel::sort(int column, Qt::SortOrder order) {
    if (sDebug) {
        qDebug() << this << "sort()" << column << order;
    }

    m_iSortColumn = column;
    m_eSortOrder = order;

    emit layoutAboutToBeChanged();
    qSort(m_sortedPlaylist.begin(), m_sortedPlaylist.end(), columnLessThan);
    emit layoutChanged();
}

int IPodPlaylistModel::rowCount(const QModelIndex& parent) const {
    if (m_pPlaylist && !parent.isValid()) {
        return m_sortedPlaylist.size();
    }
    return 0;
}

int IPodPlaylistModel::columnCount(const QModelIndex& parent) const {
    return parent.isValid() ? 0 : m_headerList.size();
}

int IPodPlaylistModel::fieldIndex(const QString& fieldName) const {
    QHash<QString, int>::const_iterator it = m_columnIndex.constFind(fieldName);
    if (it != m_columnIndex.end()) {
        return it.value();
    }
    return -1;
}

QVariant IPodPlaylistModel::data(const QModelIndex& index, int role) const {
    //qDebug() << this << "data()";
    QVariant value = QVariant();

    if (role != Qt::DisplayRole &&
            role != Qt::EditRole &&
            role != Qt::CheckStateRole &&
            role != Qt::ToolTipRole) {
        return value;
    }


    Itdb_Track* pTrack = getPTrackFromModelIndex(index);
    if (!pTrack) {
        return value;
    }

    size_t structOffset = m_headerList.at(index.column()).second;


    if (structOffset == 0xffff) { // "#"
        value = m_sortedPlaylist.at(index.row()).pos;
    } else if (structOffset == offsetof(Itdb_Track, year)) {
        if (pTrack->year) {
            value = QVariant(pTrack->year);
        }
    } else if (structOffset == offsetof(Itdb_Track, tracklen)) {
        if (pTrack->tracklen) {
            value = MixxxUtils::millisecondsToMinutes(pTrack->tracklen, true);
        }
    } else if (structOffset == offsetof(Itdb_Track, rating)) {
        value = qVariantFromValue(StarRating(pTrack->rating));
    } else if (structOffset == offsetof(Itdb_Track, track_nr)) {
        if (pTrack->track_nr) {
            value = QVariant(pTrack->track_nr);
        }
    } else if (structOffset == offsetof(Itdb_Track, BPM)) {
        if (pTrack->BPM) {
            value = QVariant(pTrack->BPM);
        }
    } else if (structOffset == offsetof(Itdb_Track, bitrate)) {
        if (pTrack->bitrate) {
            value = QVariant(pTrack->bitrate);
        }
    } else if (structOffset == offsetof(Itdb_Track, time_added)) {
        if (pTrack->time_added) {
            QDateTime timeAdded;
            timeAdded.setTime_t(pTrack->time_added);
            timeAdded.toString(Qt::ISODate);
            value = QVariant(timeAdded.toString(Qt::ISODate));
        }
    } else {
        // for the gchar* elements
        //qDebug() << *(gchar**)((char*)(pTrack) + structOffset);
        QString ret = QString::fromUtf8(*(gchar**)((char*)(pTrack) + structOffset));

        value = QVariant(ret);
    }

    // Format the value based on whether we are in a tooltip, display, or edit
    // role
    switch (role) {
        case Qt::ToolTipRole:
        case Qt::DisplayRole:
            break;
        case Qt::EditRole:
            break;
        case Qt::CheckStateRole:
            value = QVariant();
            break;
        default:
            break;
    }
    return value;
}

bool IPodPlaylistModel::setData(const QModelIndex& index, const QVariant& value, int role) {
    Q_UNUSED(index);
    Q_UNUSED(value);
    Q_UNUSED(role);
    return false;
}

TrackModel::CapabilitiesFlags IPodPlaylistModel::getCapabilities() const {
    return TRACKMODELCAPS_NONE
            | TRACKMODELCAPS_ADDTOPLAYLIST
            | TRACKMODELCAPS_ADDTOCRATE
            | TRACKMODELCAPS_ADDTOAUTODJ
            | TRACKMODELCAPS_LOADTODECK
            | TRACKMODELCAPS_LOADTOSAMPLER;
}

Qt::ItemFlags IPodPlaylistModel::flags(const QModelIndex &index) const {
    return readWriteFlags(index);
}

Qt::ItemFlags IPodPlaylistModel::readWriteFlags(const QModelIndex &index) const {
    if (!index.isValid()) {
        return Qt::ItemIsEnabled;
    }

    Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);

    // Enable dragging songs from this data model to elsewhere (like the waveform
    // widget to load a track into a Player).
    defaultFlags |= Qt::ItemIsDragEnabled;

    return defaultFlags;
}

Qt::ItemFlags IPodPlaylistModel::readOnlyFlags(const QModelIndex &index) const
{
    Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    //Enable dragging songs from this data model to elsewhere (like the waveform widget to
    //load a track into a Player).
    defaultFlags |= Qt::ItemIsDragEnabled;

    return defaultFlags;
}

void IPodPlaylistModel::trackChanged(int trackId) {
    if (sDebug) {
        qDebug() << this << "trackChanged" << trackId;
    }
}

//static
bool IPodPlaylistModel::columnLessThan(const playlist_member &s1, const playlist_member &s2) {
    bool ret;

    size_t structOffset = s1.pClass->m_headerList.at(s1.pClass->m_iSortColumn).second;

    if (structOffset == 0xffff) { // "#"
        ret = s1.pos < s2.pos;
    } else if (structOffset == offsetof(Itdb_Track, year)) {
        ret = s1.pTrack->year < s2.pTrack->year;
    } else if (structOffset == offsetof(Itdb_Track, tracklen)) {
        ret = s1.pTrack->tracklen < s2.pTrack->tracklen;
    } else if (structOffset == offsetof(Itdb_Track, rating)) {
        ret = s1.pTrack->rating < s2.pTrack->rating;
    } else if (structOffset == offsetof(Itdb_Track, track_nr)) {
        ret = s1.pTrack->track_nr < s2.pTrack->track_nr;
    } else if (structOffset == offsetof(Itdb_Track, BPM)) {
        ret = s1.pTrack->BPM < s2.pTrack->BPM;
    } else if (structOffset == offsetof(Itdb_Track, bitrate)) {
        ret = s1.pTrack->bitrate < s2.pTrack->bitrate;
    } else if (structOffset == offsetof(Itdb_Track, time_added)) {
        ret = s1.pTrack->time_added < s2.pTrack->time_added;
    } else {
        // for the gchar* elements
        QString string1 = QString::fromUtf8(*(gchar**)((char*)(s1.pTrack) + structOffset));
        QString string2 = QString::fromUtf8(*(gchar**)((char*)(s2.pTrack) + structOffset));
        ret = string1.toLower() < string2.toLower();
    }

    if (s1.pClass->m_eSortOrder != Qt::AscendingOrder) {
        return !ret;
    }
    return ret;
}

void IPodPlaylistModel::setPlaylist(Itdb_Playlist* pPlaylist) {
    if (m_pPlaylist) {
        beginRemoveRows(QModelIndex(), 0, m_sortedPlaylist.size()-1);
        m_pPlaylist = NULL;
        m_sortedPlaylist.clear();
        endRemoveRows();
    }

    if (pPlaylist) {
        beginInsertRows(QModelIndex(), 0, pPlaylist->num-1);
        m_pPlaylist = pPlaylist;
        // walk thought linked list and collect playlist position

        GList* track_node;
        uint pl_position = 0;

        QByteArray search = m_currentSearch.toUtf8();

        for (track_node = g_list_first(pPlaylist->members);
             track_node != NULL;
             track_node = g_list_next(track_node))
        {
            struct playlist_member plMember;
            plMember.pClass = this;
            plMember.pTrack = (Itdb_Track*)track_node->data;
            plMember.pos = ++pl_position;

            // Filter
            bool add = false;
            if( itdb_playlist_is_mpl(pPlaylist) && search.size()){
                // Apply filter only for the master playlist "IPOD"
                gchar* haystack = g_strconcat(
                        plMember.pTrack->artist,
                        plMember.pTrack->title,
                        plMember.pTrack->album,
                        plMember.pTrack->comment,
                        plMember.pTrack->genre,
                        NULL);

                if( findInUtf8Case(haystack, search.data())) {
                    add = true;
                }
                g_free(haystack);
            } else {
                add = true;
            }

            if (add) {
                m_sortedPlaylist.append(plMember);
            }

        }
        qSort(m_sortedPlaylist.begin(), m_sortedPlaylist.end(), columnLessThan);

        endInsertRows();
    }
}

//static
bool IPodPlaylistModel::findInUtf8Case(gchar* heystack, gchar* needles) {
    bool ret = true;
    if (heystack == NULL) {
        return false;
    }
    if (needles == NULL) {
        return true;
    }

    gchar* heystack_casefold = g_utf8_casefold(heystack, -1);
    gchar* needles_casefold = g_utf8_casefold(needles, -1);
    gchar** needle = g_strsplit_set(needles_casefold, " ", 0);

    qDebug() << "find" << heystack_casefold;

    // all needles must be found, implicit AND
    for (int i = 0; needle[i]; i++) {
        if (needle[i][0] != 0) {
            if (!g_strrstr(heystack_casefold, needle[i])) {
                ret = false;
                break;
            }
        }
    }
    g_free(heystack_casefold);
    g_free(needles_casefold);
    g_strfreev(needle);
    return ret;
}

TrackPointer IPodPlaylistModel::getTrack(const QModelIndex& index) const {
    Itdb_Track* pTrack = getPTrackFromModelIndex(index);
    if (!pTrack) {
        return TrackPointer();
    }

    QString location = itdb_get_mountpoint(m_pPlaylist->itdb);
    QString ipod_path = pTrack->ipod_path;
    ipod_path.replace(QString(":"), QString("/"));
    location += ipod_path;

    qDebug() << location;

    TrackDAO& track_dao = m_pTrackCollection->getTrackDAO();
    int track_id = track_dao.getTrackId(location);
    if (track_id < 0) {
        // Add Track to library
        track_id = track_dao.addTrack(location, true);
    }

    TrackPointer pTrackP;

    if (track_id < 0) {
        // Add Track to library failed
        // Create own TrackInfoObject
        pTrackP = TrackPointer(new TrackInfoObject(location), &QObject::deleteLater);
    } else {
        pTrackP = track_dao.getTrack(track_id);
    }

    // Overwrite metadata from Ipod
    // Note: This will be written to the mixxx library as well
    // This is OK here because the location ist still pointing to the iPod device
    pTrackP->setArtist(QString::fromUtf8(pTrack->artist));
    pTrackP->setTitle(QString::fromUtf8(pTrack->title));
    pTrackP->setAlbum(QString::fromUtf8(pTrack->album));
    pTrackP->setYear(QString::number(pTrack->year));
    pTrackP->setGenre(QString::fromUtf8(pTrack->genre));
    double bpm = (double)pTrack->BPM;
    pTrackP->setBpm(bpm);
    pTrackP->setComment(QString::fromUtf8(pTrack->comment));

    // If the track has a BPM, then give it a static beatgrid.
    if (bpm) {
        BeatsPointer pBeats = BeatFactory::makeBeatGrid(pTrackP.data(), bpm, 0.0f);
        pTrackP->setBeats(pBeats);
    }
    return pTrackP;
}

// Gets the on-disk location of the track at the given location.
QString IPodPlaylistModel::getTrackLocation(const QModelIndex& index) const {

    Itdb_Track* pTrack = getPTrackFromModelIndex(index);
    if (!pTrack) {
        return QString();
    }

    QString location = itdb_get_mountpoint(m_pPlaylist->itdb);
    QString ipod_path = pTrack->ipod_path;
    ipod_path.replace(QString(":"), QString("/"));
    location += ipod_path;

    return location;
}

// Gets a significant hint of the track at the given QModelIndex
// This is used to restore the selection after WTrackTableView::doSortByColumn
int IPodPlaylistModel::getTrackId(const QModelIndex& index) const {
    // in our case the position in the playlist is as significant hint  
    int row = index.row();
    if (row < m_sortedPlaylist.size()) {
        return m_sortedPlaylist.at(row).pos;
    } else {
        return 0;
    }
}

const QLinkedList<int> IPodPlaylistModel::getTrackRows(int trackId) const {
    // In this case we get the position as trackId, returned from getTrackId above.
    QLinkedList<int> ret;
    for (int i = 0; i < m_sortedPlaylist.size(); ++i) {
        if (m_sortedPlaylist.at(i).pos == trackId ){
            ret.push_back(i);
            break;
        }
    }
    return ret;
}

void IPodPlaylistModel::search(const QString& searchText, const QString& extraFilter) {
    if (sDebug) {
        qDebug() << this << "search" << searchText;
    }

    if (m_currentSearch != searchText || m_currentSearchFilter != extraFilter) {
        m_currentSearch = searchText;
        m_currentSearchFilter = extraFilter;
        if (itdb_playlist_is_mpl(m_pPlaylist)) {
            setPlaylist(m_pPlaylist);
        }
    }
}

const QString IPodPlaylistModel::currentSearch() {
    return m_currentSearch;
}

bool IPodPlaylistModel::isColumnInternal(int column) {
    Q_UNUSED(column);
    return false;
}

QMimeData* IPodPlaylistModel::mimeData(const QModelIndexList &indexes) const {
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
                if (!url.isValid()) {
                    qDebug() << "ERROR invalid url\n";
                } else {
                    urls.append(url);
                }
            }
        }
    }
    mimeData->setUrls(urls);
    return mimeData;
}

// if no header state exists, we may hide some columns so that the user can reactivate them
bool IPodPlaylistModel::isColumnHiddenByDefault(int column) {
    Q_UNUSED(column);
    return false;
}

QAbstractItemDelegate* IPodPlaylistModel::delegateForColumn(const int i, QObject* pParent) {
    size_t structOffset = m_headerList.at(i).second;
    if (structOffset == offsetof(Itdb_Track, rating)) {
        return new StarDelegate(pParent);
    //} else if (PlayerManager::numPreviewDecks() > 0 && (structOffset == offsetof(Itdb_Track, preview)) {
    //    return new PreviewButtonDelegate(pParent, i);
    }
    return NULL;
}

Itdb_Track* IPodPlaylistModel::getPTrackFromModelIndex(const QModelIndex& index) const {
    if (   !index.isValid()
        || m_pPlaylist == NULL
    ) {
        return NULL;
    }

    int row = index.row();
    int column = index.column();

    if (row >= m_sortedPlaylist.size() || column >= m_headerList.size()) {
        // index is outside the valid range
        return NULL;
    }
    return m_sortedPlaylist.at(row).pTrack;
}
