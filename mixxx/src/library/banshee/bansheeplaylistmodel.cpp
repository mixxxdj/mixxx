#include <QtAlgorithms>
#include <QtDebug>
#include <QTime>


#include "library/banshee/bansheeplaylistmodel.h"
#include "library/banshee/bansheedbconnection.h"
#include "mixxxutils.cpp"
#include "library/starrating.h"
#include "track/beatfactory.h"
#include "track/beats.h"


const bool sDebug = false;

BansheePlaylistModel::BansheePlaylistModel(QObject* pParent, TrackCollection* pTrackCollection, BansheeDbConnection* pConnection)
        :  TrackModel(pTrackCollection->getDatabase(),
                "mixxx.db.model.banshee_playlist"),
           QAbstractTableModel(pParent),
           m_iSortColumn(0),
           m_eSortOrder(Qt::AscendingOrder),
           m_currentSearch(""),
           m_pTrackCollection(pTrackCollection),
           m_trackDAO(m_pTrackCollection->getTrackDAO()),
           m_pConnection(pConnection),
           m_playlistId(-1)
{
    initHeaderData();
}

BansheePlaylistModel::~BansheePlaylistModel() {

}

void BansheePlaylistModel::appendColumnsInfo(
        enum Columns id,
        QString lable,
        bool (*lessThen)(struct BansheeDbConnection::PlaylistEntry &s1, struct BansheeDbConnection::PlaylistEntry &s2),
        bool (*greaterThen)(struct BansheeDbConnection::PlaylistEntry &s1, struct BansheeDbConnection::PlaylistEntry &s2)
        ) {

    struct ColumnsInfo info;

    info.id = id;
    info.lable = lable;
    info.lessThen =lessThen;
    info.greaterThen = greaterThen;

    m_headerList.append(info);
}

void BansheePlaylistModel::initHeaderData() {

    appendColumnsInfo(VIEW_ORDER, tr("#"), &BansheeDbConnection::viewOrderLessThen, &BansheeDbConnection::viewOrderGreaterThen);
    appendColumnsInfo(ARTIST, tr("Artist"), &BansheeDbConnection::artistLessThen, &BansheeDbConnection::artistGreaterThen);
    appendColumnsInfo(TITLE, tr("Title"), &BansheeDbConnection::titleLessThen, &BansheeDbConnection::titleGreaterThen);
    appendColumnsInfo(DURATION, tr("Duration"), &BansheeDbConnection::durationLessThen, &BansheeDbConnection::durationGreaterThen);
    appendColumnsInfo(ALBUM, tr("Album"), &BansheeDbConnection::albumLessThen, &BansheeDbConnection::albumGreaterThen);
    appendColumnsInfo(YEAR, tr("Year"), &BansheeDbConnection::yearLessThen, &BansheeDbConnection::yearGreaterThen);
    appendColumnsInfo(RATING, tr("Rating"), &BansheeDbConnection::ratingLessThen, &BansheeDbConnection::ratingGreaterThen);
    appendColumnsInfo(GENRE, tr("Genre"), &BansheeDbConnection::genreLessThen, &BansheeDbConnection::genreGreaterThen);
    appendColumnsInfo(COMPOSER, tr("Composer"), &BansheeDbConnection::composerLessThen, &BansheeDbConnection::composerGreaterThen);
    appendColumnsInfo(TRACKNUMBER, tr("Track #"), &BansheeDbConnection::tracknumberLessThen, &BansheeDbConnection::tracknumberGreaterThen);
    appendColumnsInfo(DATEADDED, tr("Date Added"), &BansheeDbConnection::dateaddedLessThen, &BansheeDbConnection::dateaddedGreaterThen);
    appendColumnsInfo(BPM, tr("BPM"), &BansheeDbConnection::bpmLessThen, &BansheeDbConnection::bpmGreaterThen);
    appendColumnsInfo(BITRATE, tr("Bitrate"), &BansheeDbConnection::bitrateLessThen, &BansheeDbConnection::bitrateGreaterThen);
    appendColumnsInfo(COMMENT, tr("Comment"), &BansheeDbConnection::commentLessThen, &BansheeDbConnection::commentGreaterThen);
    appendColumnsInfo(PLAYCOUNT, tr("Played"), &BansheeDbConnection::playcountLessThen, &BansheeDbConnection::playcountGreaterThen);
    appendColumnsInfo(URI, tr("Uri"), &BansheeDbConnection::uriLessThen, &BansheeDbConnection::uriGreaterThen);
}

QVariant BansheePlaylistModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole)
        return QAbstractTableModel::headerData(section, orientation, role);

    if (   orientation == Qt::Horizontal
        && role == Qt::DisplayRole
        && section < m_headerList.size()
    ) {
        return QVariant(m_headerList.at(section).lable);
    }
    return QAbstractTableModel::headerData(section, orientation, role);
}


void BansheePlaylistModel::initDefaultSearchColumns() {
}

const QString BansheePlaylistModel::currentSearch() const {
    return m_currentSearch;
}

void BansheePlaylistModel::setSort(int column, Qt::SortOrder order) {
    if (sDebug) {
        qDebug() << this << "setSort()";
    }

    m_iSortColumn = column;
    m_eSortOrder = order;
}

void BansheePlaylistModel::sort(int column, Qt::SortOrder order) {
    if (sDebug) {
        qDebug() << this << "sort()" << column << order;
    }

    m_iSortColumn = column;
    m_eSortOrder = order;

    if (m_iSortColumn >= 0 && m_iSortColumn < m_headerList.size()) {
        emit layoutAboutToBeChanged();
        if (m_eSortOrder != Qt::AscendingOrder) {
            qSort(m_sortedPlaylist.begin(), m_sortedPlaylist.end(), m_headerList.at(m_iSortColumn).greaterThen);
        } else {
            qSort(m_sortedPlaylist.begin(), m_sortedPlaylist.end(), m_headerList.at(m_iSortColumn).lessThen);
        }
        emit layoutChanged();
    }
}

int BansheePlaylistModel::rowCount(const QModelIndex& parent) const {

    if (!parent.isValid()) {
        return m_sortedPlaylist.size();
    }
    return 0;
}

int BansheePlaylistModel::columnCount(const QModelIndex& parent) const {
    return parent.isValid() ? 0 : m_headerList.size();
}

QVariant BansheePlaylistModel::data(const QModelIndex& index, int role) const {
    // qDebug() << this << "data()";

    QVariant value = QVariant();

    if (    role != Qt::DisplayRole
         && role != Qt::EditRole
         && role != Qt::CheckStateRole
         && role != Qt::ToolTipRole
    ) {
        return value;
    }

    int ivalue;
    float fvalue;
    QDateTime timeAdded;

    int row = index.row();
    if (row < m_sortedPlaylist.size()) {
        int duration;
        switch (m_headerList.at(index.column()).id) {
        case VIEW_ORDER:
            value = m_sortedPlaylist.at(row).viewOrder;
            break;
        case ARTIST:
            value = m_sortedPlaylist.at(row).pArtist->name;
            break;
        case TITLE:
            value = m_sortedPlaylist.at(row).pTrack->title;
            break;
        case DURATION:
            duration = m_sortedPlaylist.at(row).pTrack->duration;
            if (duration) {
                value = MixxxUtils::millisecondsToMinutes(duration, true);
            }
            break;
        case URI:
            value = m_sortedPlaylist.at(row).pTrack->uri;
            break;
        case ALBUM:
            value = m_sortedPlaylist.at(row).pAlbum->title;
            break;
        case YEAR:
            ivalue = m_sortedPlaylist.at(row).pTrack->year;
            if (ivalue) {
                value = ivalue;
            }
            break;
        case RATING:
            ivalue = m_sortedPlaylist.at(row).pTrack->rating;
            value = qVariantFromValue(StarRating(ivalue));
            break;
        case GENRE:
            value = m_sortedPlaylist.at(row).pTrack->genre;
            break;
        case TRACKNUMBER:
            ivalue = m_sortedPlaylist.at(row).pTrack->tracknumber;
            if (ivalue) {
                value = ivalue;
            }
            break;
        case DATEADDED:
            ivalue = m_sortedPlaylist.at(row).pTrack->dateadded;
            if (ivalue) {
                timeAdded.setTime_t(ivalue);
                timeAdded.toString(Qt::ISODate);
                value = QVariant(timeAdded.toString(Qt::ISODate));
            }
            break;
        case BPM:
            ivalue = m_sortedPlaylist.at(row).pTrack->bpm;
            if (ivalue) {
                fvalue = ivalue/10.0;
                value = fvalue;
            }
            break;
        case BITRATE:
            ivalue = m_sortedPlaylist.at(row).pTrack->bitrate;
            if (ivalue) {
                value = ivalue;
            }
            break;
        case COMMENT:
            value = m_sortedPlaylist.at(row).pTrack->comment;
            break;
        case PLAYCOUNT:
            value = m_sortedPlaylist.at(row).pTrack->playcount;
            break;
        case COMPOSER:
            value = m_sortedPlaylist.at(row).pTrack->composer;
            break;
        default:
            break; // Not supported
        }
    }


/*
    // This value is the value in its most raw form. It was looked up either
    // from the SQL table or from the cached track layer.
    QVariant value = getBaseValue(index, role);
*/


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
            //}
            break;
        default:
            break;
    }
    return value;
}

bool BansheePlaylistModel::setData(const QModelIndex& index, const QVariant& value, int role) {
    Q_UNUSED(index);
    Q_UNUSED(value);
    Q_UNUSED(role);
    return false;
}

TrackModel::CapabilitiesFlags BansheePlaylistModel::getCapabilities() const {
    return TRACKMODELCAPS_NONE
            | TRACKMODELCAPS_ADDTOPLAYLIST
            | TRACKMODELCAPS_ADDTOCRATE
            | TRACKMODELCAPS_ADDTOAUTODJ
            | TRACKMODELCAPS_LOADTODECK
            | TRACKMODELCAPS_LOADTOSAMPLER;
}

Qt::ItemFlags BansheePlaylistModel::flags(const QModelIndex &index) const {
    return readWriteFlags(index);
}

Qt::ItemFlags BansheePlaylistModel::readWriteFlags(const QModelIndex &index) const {
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);

    //Enable dragging songs from this data model to elsewhere (like the waveform
    //widget to load a track into a Player).
    defaultFlags |= Qt::ItemIsDragEnabled;

    return defaultFlags;
}

Qt::ItemFlags BansheePlaylistModel::readOnlyFlags(const QModelIndex &index) const
{
    Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    //Enable dragging songs from this data model to elsewhere (like the waveform widget to
    //load a track into a Player).
    defaultFlags |= Qt::ItemIsDragEnabled;

    return defaultFlags;
}

void BansheePlaylistModel::trackChanged(int trackId) {
    if (sDebug) {
        qDebug() << this << "trackChanged" << trackId;
    }
}

void BansheePlaylistModel::setPlaylist(int playlistId) {

    if (m_playlistId >= 0) {
        beginRemoveRows(QModelIndex(), 0, m_sortedPlaylist.size()-1);
        m_playlistId = -1;
        m_sortedPlaylist.clear();
        endRemoveRows();
    }

    if (playlistId >= 0) {
        QList<struct BansheeDbConnection::PlaylistEntry> list;

        m_playlistId = playlistId;

        list = m_pConnection->getPlaylistEntries(playlistId);

        beginInsertRows(QModelIndex(), 0, list.size()-1);

        foreach (struct BansheeDbConnection::PlaylistEntry entry, list) {
            bool found = true;
            QStringList search = m_currentSearch.split(" ", QString::SkipEmptyParts);
            foreach (const QString &str, search) {
                if (entry.pArtist->name.contains(str, Qt::CaseInsensitive)) {
                } else if (entry.pTrack->title.contains(str, Qt::CaseInsensitive)) {
                } else if (entry.pAlbum->title.contains(str, Qt::CaseInsensitive)) {
                } else if (entry.pTrack->comment.contains(str, Qt::CaseInsensitive)) {
                } else if (entry.pTrack->genre.contains(str, Qt::CaseInsensitive)) {
                } else {
                    // search String part not found, don't add entry to m_sortedPlaylist
                    found = false;
                    break;
                }
            }

            if (found) {
                m_sortedPlaylist.append(entry);
            }
        }

        endInsertRows();

        sort(m_iSortColumn, m_eSortOrder);
    }
}

TrackPointer BansheePlaylistModel::getTrack(const QModelIndex& index) const {

    QString location;

    location = getTrackLocation(index);

    if (location.isEmpty()) {
        return TrackPointer();
    }

    int row = index.row();

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
        pTrack->setArtist(m_sortedPlaylist.at(row).pArtist->name);
        pTrack->setTitle(m_sortedPlaylist.at(row).pTrack->title);
        pTrack->setDuration(m_sortedPlaylist.at(row).pTrack->duration);
        pTrack->setAlbum(m_sortedPlaylist.at(row).pAlbum->title);
        pTrack->setYear(QString::number(m_sortedPlaylist.at(row).pTrack->year));
        pTrack->setRating(m_sortedPlaylist.at(row).pTrack->rating);
        pTrack->setGenre(m_sortedPlaylist.at(row).pTrack->genre);
        pTrack->setTrackNumber(QString::number(m_sortedPlaylist.at(row).pTrack->tracknumber));
        double bpm = ((double)m_sortedPlaylist.at(row).pTrack->bpm)/10.0;
        pTrack->setBpm(bpm);
        pTrack->setBitrate(m_sortedPlaylist.at(row).pTrack->bitrate);
        pTrack->setComment(m_sortedPlaylist.at(row).pTrack->comment);
        pTrack->setComposer(m_sortedPlaylist.at(row).pTrack->composer);
        // If the track has a BPM, then give it a static beatgrid.
        if (bpm > 0) {
            BeatsPointer pBeats = BeatFactory::makeBeatGrid(pTrack.data(), bpm, 0.0f);
            pTrack->setBeats(pBeats);
        }
    }
    return pTrack;
}

// Gets the on-disk location of the track at the given location.
QString BansheePlaylistModel::getTrackLocation(const QModelIndex& index) const {
    int row = index.row();

    if (row >= m_sortedPlaylist.size()) {
        return QString();
    }

    QUrl url = m_sortedPlaylist.at(row).pTrack->uri;

    QString location;
    location = url.toLocalFile();

    qDebug() << location << " = " << url;

    if (!location.isEmpty()) {
        return location;
    }

    // Try to convert a smb path location = url.toLocalFile();
    QString temp_location = url.toString();

    if (temp_location.startsWith("smb://")) {
        // Hack for samba mounts works only on German GNOME Linux
        // smb://daniel-desktop/volume/Musik/Lastfm/Limp Bizkit/Chocolate Starfish And The Hot Dog Flavored Water/06 - Rollin' (Air Raid Vehicle).mp3"
        // TODO(xxx): use gio instead

        location = QDir::homePath() + "/.gvfs/";
        location += temp_location.section('/', 3, 3);
        location += " auf ";
        location += temp_location.section('/', 2, 2);
        location += "/";
        location += temp_location.section('/', 4);

        return location;
    }

    return QString();
}

// Gets a significant hint of the track at the given QModelIndex
// This is used to restore the selection after WTrackTableView::doSortByColumn
// We user here the view Order because a track might be more than one time in a playlist
int BansheePlaylistModel::getTrackId(const QModelIndex& index) const {
    // in our case the position in the playlist is a significant hint
    int row = index.row();
    if (row < m_sortedPlaylist.size()) {
        return m_sortedPlaylist.at(index.row()).viewOrder;
    }
    else {
        return 0;
    }
}

const QLinkedList<int> BansheePlaylistModel::getTrackRows(int trackId) const {
    // In this case we get the position as trackId, returned from getTrackId above.
    QLinkedList<int> ret;
    for (int i = 0; i < m_sortedPlaylist.size(); ++i) {
        if (m_sortedPlaylist.at(i).viewOrder == trackId ){
            ret.push_back(i);
            break;
        }
    }
    return ret;
}

void BansheePlaylistModel::search(const QString& searchText) {
    if (sDebug)
        qDebug() << this << "search" << searchText;

    if (m_currentSearch != searchText) {
        m_currentSearch = searchText;
        setPlaylist(m_playlistId);
    }
}

const QString BansheePlaylistModel::currentSearch() {
    return m_currentSearch;
}

bool BansheePlaylistModel::isColumnInternal(int column) {
    Q_UNUSED(column);
    return false;
}

    /** if no header state exists, we may hide some columns so that the user can reactivate them **/
bool BansheePlaylistModel::isColumnHiddenByDefault(int column) {
    Q_UNUSED(column);
    return false;
}

void BansheePlaylistModel::removeTrack(const QModelIndex& index) {
    Q_UNUSED(index);
}

void BansheePlaylistModel::removeTracks(const QModelIndexList& indices) {
    Q_UNUSED(indices);
}

bool BansheePlaylistModel::addTrack(const QModelIndex& index, QString location) {
    Q_UNUSED(index);
    Q_UNUSED(location);
    return false;
}

void BansheePlaylistModel::moveTrack(const QModelIndex& sourceIndex, const QModelIndex& destIndex) {
    Q_UNUSED(destIndex);
    Q_UNUSED(sourceIndex);
}

QItemDelegate* BansheePlaylistModel::delegateForColumn(const int i) {
    Q_UNUSED(i);
    return NULL;
}
