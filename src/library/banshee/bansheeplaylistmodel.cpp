#include <QtAlgorithms>
#include <QtDebug>
#include <QTime>

#include "library/banshee/bansheeplaylistmodel.h"
#include "library/banshee/bansheedbconnection.h"
#include "library/queryutil.h"
#include "mixxxutils.cpp"
#include "library/starrating.h"
#include "library/previewbuttondelegate.h"
#include "track/beatfactory.h"
#include "track/beats.h"
#include "playermanager.h"

#define BANSHEE_TABLE "banshee"
#define CLM_VIEW_ORDER "position"
#define CLM_ARTIST "artist"
#define CLM_TITLE "title"
#define CLM_DURATION "duration"
#define CLM_URI "location"
#define CLM_ALBUM "album"
#define CLM_ALBUM_ARTIST "album_artist"
#define CLM_YEAR "year"
#define CLM_RATING "rating"
#define CLM_GENRE "genre"
#define CLM_GROUPING "grouping"
#define CLM_TRACKNUMBER "tracknumber"
#define CLM_DATEADDED "datetime_added"
#define CLM_BPM "bpm"
#define CLM_BITRATE "bitrate"
#define CLM_COMMENT "comment"
#define CLM_PLAYCOUNT "timesplayed"
#define CLM_COMPOSER "composer"
#define CLM_PREVIEW "preview"


const bool sDebug = false;

BansheePlaylistModel::BansheePlaylistModel(QObject* pParent, TrackCollection* pTrackCollection, BansheeDbConnection* pConnection)
        : BaseSqlTableModel(pParent, pTrackCollection, "mixxx.db.model.banshee_playlist"),
         // m_playlistDao(m_pTrackCollection->getPlaylistDAO()),
         //   m_iPlaylistId(-1),
         //                   m_showAll(showAll)
          m_iSortColumn(0),
          m_eSortOrder(Qt::AscendingOrder),
          m_currentSearch(""),
          m_pTrackCollection(pTrackCollection),
          m_trackDAO(m_pTrackCollection->getTrackDAO()),
          m_pConnection(pConnection),
          m_playlistId(-1) {
}

BansheePlaylistModel::~BansheePlaylistModel() {
}

void BansheePlaylistModel::setTableModel(int playlistId) {
    //qDebug() << "BansheePlaylistModel::setTableModel" << playlistId;
    if (m_playlistId == playlistId) {
        qDebug() << "Already focused on playlist " << playlistId;
        return;
    }

    if (m_playlistId >= 0) {
        // Clear old playlist
        beginRemoveRows(QModelIndex(), 0, m_sortedPlaylist.size() - 1);
        m_playlistId = -1;
        QSqlQuery query(m_pTrackCollection->getDatabase());
        QString strQuery("DELETE FROM " BANSHEE_TABLE);
        if (!query.exec(strQuery)) {
            LOG_FAILED_QUERY(query);
        }
        endRemoveRows();
    }

    if (playlistId >= 0) {
        // setup new playlist
        m_playlistId = playlistId;

        QSqlQuery query(m_pTrackCollection->getDatabase());
        QString strQuery("CREATE TEMP TABLE IF NOT EXISTS " BANSHEE_TABLE
            " (" CLM_VIEW_ORDER " INTEGER, "
                 CLM_ARTIST " TEXT, "
                 CLM_TITLE " TEXT, "
                 CLM_DURATION " INTEGER, "
                 CLM_URI " TEXT, "
                 CLM_ALBUM " TEXT, "
                 CLM_ALBUM_ARTIST " TEXT, "
                 CLM_YEAR " INTEGER, "
                 CLM_RATING " INTEGER, "
                 CLM_GENRE " TEXT, "
                 CLM_GROUPING " TEXT, "
                 CLM_TRACKNUMBER " INTEGER, "
                 CLM_DATEADDED " INTEGER, "
                 CLM_BPM " INTEGER, "
                 CLM_BITRATE " INTEGER, "
                 CLM_COMMENT " TEXT, "
                 CLM_PLAYCOUNT" INTEGER, "
                 CLM_COMPOSER " TEXT, "
                 CLM_PREVIEW " TEXT)");
        if (!query.exec(strQuery)) {
            LOG_FAILED_QUERY(query);
        }

        query.prepare("INSERT INTO " BANSHEE_TABLE
                " (" CLM_VIEW_ORDER ", "
                     CLM_ARTIST ", "
                     CLM_TITLE ", "
                     CLM_DURATION ", "
                     CLM_URI ", "
                     CLM_ALBUM ", "
                     CLM_ALBUM_ARTIST ", "
                     CLM_YEAR ", "
                     CLM_RATING ", "
                     CLM_GENRE ", "
                     CLM_GROUPING ", "
                     CLM_TRACKNUMBER ", "
                     CLM_DATEADDED ", "
                     CLM_BPM ", "
                     CLM_BITRATE ", "
                     CLM_COMMENT ", "
                     CLM_PLAYCOUNT ", "
                     CLM_COMPOSER ") "
                     "VALUES (:"
                     CLM_VIEW_ORDER ", :"
                     CLM_ARTIST ", :"
                     CLM_TITLE ", :"
                     CLM_DURATION ", :"
                     CLM_URI ", :"
                     CLM_ALBUM ", :"
                     CLM_ALBUM_ARTIST ", :"
                     CLM_YEAR ", :"
                     CLM_RATING ", :"
                     CLM_GENRE ", :"
                     CLM_GROUPING ", :"
                     CLM_TRACKNUMBER ", :"
                     CLM_DATEADDED ", :"
                     CLM_BPM ", :"
                     CLM_BITRATE ", :"
                     CLM_COMMENT ", :"
                     CLM_PLAYCOUNT ", :"
                     CLM_COMPOSER ") ");


        QList<struct BansheeDbConnection::PlaylistEntry> list =
                m_pConnection->getPlaylistEntries(playlistId);

        beginInsertRows(QModelIndex(), 0, list.size() - 1);

        foreach (struct BansheeDbConnection::PlaylistEntry entry, list){
            query.bindValue(":" CLM_VIEW_ORDER, entry.viewOrder + 1);
            query.bindValue(":" CLM_ARTIST, entry.pArtist->name);
            query.bindValue(":" CLM_TITLE, entry.pTrack->title);
            query.bindValue(":" CLM_DURATION, entry.pTrack->duration);
            query.bindValue(":" CLM_URI, entry.pTrack->uri);
            query.bindValue(":" CLM_ALBUM, entry.pAlbum->title);
            query.bindValue(":" CLM_ALBUM_ARTIST, entry.pAlbumArtist->name);
            query.bindValue(":" CLM_YEAR, entry.pTrack->year);
            query.bindValue(":" CLM_RATING, entry.pTrack->rating);
            query.bindValue(":" CLM_GENRE, entry.pTrack->genre);
            query.bindValue(":" CLM_GROUPING, entry.pTrack->grouping);
            query.bindValue(":" CLM_TRACKNUMBER, entry.pTrack->tracknumber);
            query.bindValue(":" CLM_DATEADDED, entry.pTrack->dateadded);
            query.bindValue(":" CLM_BPM, entry.pTrack->bpm);
            query.bindValue(":" CLM_BITRATE, entry.pTrack->bitrate);
            query.bindValue(":" CLM_COMMENT, entry.pTrack->comment);
            query.bindValue(":" CLM_PLAYCOUNT, entry.pTrack->playcount);
            query.bindValue(":" CLM_COMPOSER, entry.pTrack->composer);

            if (!query.exec()) {
                LOG_FAILED_QUERY(query);
            }
            // qDebug() << "-----" << entry.pTrack->title << query.executedQuery();
        }

        endInsertRows();
    }

    QStringList columns;
    columns << CLM_VIEW_ORDER // 0
         << CLM_ARTIST
         << CLM_TITLE
         << CLM_DURATION
         << CLM_URI
         << CLM_ALBUM
         << CLM_ALBUM_ARTIST
         << CLM_YEAR
         << CLM_RATING
         << CLM_GENRE
         << CLM_GROUPING
         << CLM_TRACKNUMBER
         << CLM_DATEADDED
         << CLM_BPM
         << CLM_BITRATE
         << CLM_COMMENT
         << CLM_PLAYCOUNT
         << CLM_COMPOSER
         << CLM_PREVIEW;

    setTable(BANSHEE_TABLE, columns[0], columns, QSharedPointer<BaseTrackCache>());
    setSearch("");
    setDefaultSort(fieldIndex(PLAYLISTTRACKSTABLE_POSITION), Qt::AscendingOrder);
    setSort(defaultSortColumn(), defaultSortOrder());
}

void BansheePlaylistModel::initDefaultSearchColumns() {
}

const QString BansheePlaylistModel::currentSearch() const {
    return m_currentSearch;
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
    if (!index.isValid()) {
        return Qt::ItemIsEnabled;
    }

    Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);

    // Enable dragging songs from this data model to elsewhere (like the waveform
    // widget to load a track into a Player).
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
    // saved with the metadata from Banshee. If it was already in the library
    // then we do not touch it so that we do not over-write the user's metadata.
    if (!track_already_in_library) {
        pTrack->setArtist(m_sortedPlaylist.at(row).pArtist->name);
        pTrack->setTitle(m_sortedPlaylist.at(row).pTrack->title);
        pTrack->setDuration(m_sortedPlaylist.at(row).pTrack->duration);
        pTrack->setAlbum(m_sortedPlaylist.at(row).pAlbum->title);
        pTrack->setAlbumArtist(m_sortedPlaylist.at(row).pAlbumArtist->name);
        pTrack->setYear(QString::number(m_sortedPlaylist.at(row).pTrack->year));
        pTrack->setGenre(m_sortedPlaylist.at(row).pTrack->genre);
        pTrack->setGrouping(m_sortedPlaylist.at(row).pTrack->grouping);
        pTrack->setRating(m_sortedPlaylist.at(row).pTrack->rating);
        pTrack->setTrackNumber(QString::number(m_sortedPlaylist.at(row).pTrack->tracknumber));
        double bpm = ((double)m_sortedPlaylist.at(row).pTrack->bpm);
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
    if (!index.isValid()) {
        return "";
    }
    QUrl url = index.sibling(
            index.row(), fieldIndex("location")).data().toString();

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

/*
// Gets a significant hint of the track at the given QModelIndex
// This is used to restore the selection after WTrackTableView::doSortByColumn
// We user here the view Order because a track might be more than one time in a playlist
int BansheePlaylistModel::getTrackId(const QModelIndex& index) const {
    // in our case the position in the playlist is a significant hint
    int row = index.row();
    if (row < m_sortedPlaylist.size()) {
        return m_sortedPlaylist.at(index.row()).viewOrder;
    } else {
        return 0;
    }
}
*/

/*
const QLinkedList<int> BansheePlaylistModel::getTrackRows(int trackId) const {
    // In this case we get the position as trackId, returned from getTrackId above.
    QLinkedList<int> ret;
    for (int i = 0; i < m_sortedPlaylist.size(); ++i) {
        if (m_sortedPlaylist.at(i).viewOrder == trackId) {
            ret.push_back(i);
            break;
        }
    }
    return ret;
}
*/
/*
void BansheePlaylistModel::search(const QString& searchText, const QString& extraFilter) {
    if (sDebug) {
        qDebug() << this << "search" << searchText;
    }

    if (m_currentSearch != searchText || m_currentSearchFilter != extraFilter) {
        m_currentSearch = searchText;
        m_currentSearchFilter = extraFilter;
        setPlaylist(m_playlistId);
    }
}
*/

const QString BansheePlaylistModel::currentSearch() {
    return m_currentSearch;
}

bool BansheePlaylistModel::isColumnInternal(int column) {
    Q_UNUSED(column);
    return false;
}

// if no header state exists, we may hide some columns so that the user can reactivate them
bool BansheePlaylistModel::isColumnHiddenByDefault(int column) {
    Q_UNUSED(column);
    return false;
}
