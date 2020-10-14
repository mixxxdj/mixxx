#include "library/clementine/clementineplaylistmodel.h"

#include <QtDebug>

#include "library/clementine/clementinedbconnection.h"
#include "library/queryutil.h"
#include "library/starrating.h"
#include "library/trackcollectionmanager.h"
#include "mixer/playermanager.h"
#include "track/beatfactory.h"
#include "track/beats.h"

#define Clementine_TABLE "Clementine"
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

ClementinePlaylistModel::ClementinePlaylistModel(QObject* pParent, TrackCollectionManager* pTrackCollectionManager, ClementineDbConnection* pConnection)
        : BaseSqlTableModel(pParent, pTrackCollectionManager, "mixxx.db.model.Clementine_playlist"),
          m_pConnection(pConnection),
          m_playlistId(-1) {
}

ClementinePlaylistModel::~ClementinePlaylistModel() {
}

void ClementinePlaylistModel::setTableModel(int playlistId) {
    qDebug() << "ClementinePlaylistModel::setTableModel" << playlistId;
    if (m_playlistId == playlistId) {
        qDebug() << "Already focused on playlist " << playlistId;
        return;
    }

    if (m_playlistId >= 0) {
        // Clear old playlist
        m_playlistId = -1;
        QSqlQuery query(m_database);
        QString strQuery("DELETE FROM " Clementine_TABLE);
        if (!query.exec(strQuery)) {
            LOG_FAILED_QUERY(query);
        }
    }

    if (playlistId >= 0) {
        // setup new playlist
        m_playlistId = playlistId;

        QSqlQuery query(m_database);
        QString strQuery("CREATE TEMP TABLE IF NOT EXISTS " Clementine_TABLE
                         " (" CLM_VIEW_ORDER " INTEGER, " CLM_ARTIST " TEXT, " CLM_TITLE " TEXT, " CLM_DURATION " INTEGER, " CLM_URI " TEXT, " CLM_ALBUM " TEXT, " CLM_ALBUM_ARTIST " TEXT, " CLM_YEAR " INTEGER, " CLM_RATING " INTEGER, " CLM_GENRE " TEXT, " CLM_GROUPING " TEXT, " CLM_TRACKNUMBER " INTEGER, "
                //CLM_DATEADDED " INTEGER, "
                CLM_BPM " INTEGER, " CLM_BITRATE " INTEGER, " CLM_COMMENT " TEXT, " CLM_PLAYCOUNT " INTEGER, " CLM_COMPOSER " TEXT, " CLM_PREVIEW " TEXT)");
        if (!query.exec(strQuery)) {
            LOG_FAILED_QUERY(query);
        }

        query.prepare("INSERT INTO " Clementine_TABLE
                      " (" CLM_VIEW_ORDER ", " CLM_ARTIST ", " CLM_TITLE ", " CLM_DURATION ", " CLM_URI ", " CLM_ALBUM ", " CLM_ALBUM_ARTIST ", " CLM_YEAR ", " CLM_RATING ", " CLM_GENRE ", " CLM_GROUPING ", " CLM_TRACKNUMBER ", "
                //CLM_DATEADDED ", "
                CLM_BPM ", " CLM_BITRATE ", " CLM_COMMENT ", " CLM_PLAYCOUNT ", " CLM_COMPOSER
                      ") "
                      "VALUES (:" CLM_VIEW_ORDER ", :" CLM_ARTIST ", :" CLM_TITLE ", :" CLM_DURATION ", :" CLM_URI ", :" CLM_ALBUM ", :" CLM_ALBUM_ARTIST ", :" CLM_YEAR ", :" CLM_RATING ", :" CLM_GENRE ", :" CLM_GROUPING ", :" CLM_TRACKNUMBER ", :"
                //CLM_DATEADDED ", :"
                CLM_BPM ", :" CLM_BITRATE ", :" CLM_COMMENT ", :" CLM_PLAYCOUNT ", :" CLM_COMPOSER ") ");

        QList<struct ClementineDbConnection::PlaylistEntry> list =
                m_pConnection->getPlaylistEntries(playlistId);

        if (!list.isEmpty()) {
            beginInsertRows(QModelIndex(), 0, list.size() - 1);
            int i = 1;
            foreach (struct ClementineDbConnection::PlaylistEntry entry, list) {
                query.bindValue(":" CLM_VIEW_ORDER, i++);
                query.bindValue(":" CLM_ARTIST, entry.artist);
                query.bindValue(":" CLM_TITLE, entry.title);
                query.bindValue(":" CLM_DURATION, entry.duration);
                query.bindValue(":" CLM_URI, entry.uri);
                query.bindValue(":" CLM_ALBUM, entry.album);
                query.bindValue(":" CLM_ALBUM_ARTIST, entry.albumartist);
                query.bindValue(":" CLM_YEAR, entry.year);
                query.bindValue(":" CLM_RATING, entry.rating);
                query.bindValue(":" CLM_GENRE, entry.genre);
                query.bindValue(":" CLM_GROUPING, entry.grouping);
                query.bindValue(":" CLM_TRACKNUMBER, entry.tracknumber);

                query.bindValue(":" CLM_BPM, entry.bpm);
                query.bindValue(":" CLM_BITRATE, entry.bitrate);
                query.bindValue(":" CLM_COMMENT, entry.comment);
                query.bindValue(":" CLM_PLAYCOUNT, entry.playcount);
                query.bindValue(":" CLM_COMPOSER, entry.composer);

                if (!query.exec()) {
                    LOG_FAILED_QUERY(query);
                }
                // qDebug() << "-----" << entry.pTrack->title << query.executedQuery();
            }

            endInsertRows();
        }
    }

    QStringList tableColumns;
    tableColumns << CLM_VIEW_ORDER // 0
                 << CLM_PREVIEW;

    QStringList trackSourceColumns;
    trackSourceColumns << CLM_VIEW_ORDER // 0
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
                       //<< CLM_DATEADDED
                       << CLM_BPM
                       << CLM_BITRATE
                       << CLM_COMMENT
                       << CLM_PLAYCOUNT
                       << CLM_COMPOSER;

    QSharedPointer<BaseTrackCache> trackSource(
            new BaseTrackCache(m_pTrackCollectionManager->internalCollection(),
                    Clementine_TABLE,
                    CLM_VIEW_ORDER,
                    trackSourceColumns,
                    false));

    setTable(Clementine_TABLE, CLM_VIEW_ORDER, tableColumns, trackSource);
    setSearch("");
    setDefaultSort(fieldIndex(PLAYLISTTRACKSTABLE_POSITION), Qt::AscendingOrder);
    setSort(defaultSortColumn(), defaultSortOrder());
}

TrackModel::Capabilities ClementinePlaylistModel::getCapabilities() const {
    return Capability::AddToTrackSet |
            Capability::AddToAutoDJ |
            Capability::LoadToDeck |
            Capability::LoadToSampler;
}

Qt::ItemFlags ClementinePlaylistModel::flags(const QModelIndex& index) const {
    return readWriteFlags(index);
}

TrackId ClementinePlaylistModel::doGetTrackId(const TrackPointer& pTrack) const {
    if (pTrack) {
        for (int row = 0; row < rowCount(); ++row) {
            const QUrl rowUrl(getFieldString(index(row, 0), CLM_URI));
            if (TrackFile::fromUrl(rowUrl) == pTrack->getFileInfo()) {
                return TrackId(getFieldVariant(index(row, 0), CLM_VIEW_ORDER));
            }
        }
    }
    return TrackId();
}

QVariant ClementinePlaylistModel::getFieldVariant(const QModelIndex& index,
        const QString& fieldName) const {
    return index.sibling(index.row(), fieldIndex(fieldName)).data();
}

QString ClementinePlaylistModel::getFieldString(const QModelIndex& index,
        const QString& fieldName) const {
    return getFieldVariant(index, fieldName).toString();
}

TrackPointer ClementinePlaylistModel::getTrack(const QModelIndex& index) const {
    QString location = getTrackLocation(index);

    if (location.isEmpty()) {
        // Track is lost
        return TrackPointer();
    }

    bool track_already_in_library = false;
    TrackPointer pTrack = m_pTrackCollectionManager->getOrAddTrack(
            TrackRef::fromFileInfo(location),
            &track_already_in_library);

    // If this track was not in the Mixxx library it is now added and will be
    // saved with the metadata from Clementine. If it was already in the library
    // then we do not touch it so that we do not over-write the user's metadata.
    if (pTrack && !track_already_in_library) {
        pTrack->setArtist(getFieldString(index, CLM_ARTIST));
        pTrack->setTitle(getFieldString(index, CLM_TITLE));
        pTrack->setDuration(getFieldString(index, CLM_DURATION).toDouble());
        pTrack->setAlbum(getFieldString(index, CLM_ALBUM));
        pTrack->setAlbumArtist(getFieldString(index, CLM_ALBUM_ARTIST));
        pTrack->setYear(getFieldString(index, CLM_YEAR));
        pTrack->setGenre(getFieldString(index, CLM_GENRE));
        pTrack->setGrouping(getFieldString(index, CLM_GROUPING));
        pTrack->setRating(getFieldString(index, CLM_RATING).toInt());
        pTrack->setTrackNumber(getFieldString(index, CLM_TRACKNUMBER));
        double bpm = getFieldString(index, CLM_BPM).toDouble();
        bpm = pTrack->setBpm(bpm);
        pTrack->setBitrate(getFieldString(index, CLM_BITRATE).toInt());
        pTrack->setComment(getFieldString(index, CLM_COMMENT));
        pTrack->setComposer(getFieldString(index, CLM_COMPOSER));
        // If the track has a BPM, then give it a static beatgrid.
        if (bpm > 0) {
            mixxx::BeatsPointer pBeats = BeatFactory::makeBeatGrid(*pTrack, bpm, 0.0);
            pTrack->setBeats(pBeats);
        }
    }
    return pTrack;
}

TrackId ClementinePlaylistModel::getTrackId(const QModelIndex& index) const {
    const auto track = getTrack(index);
    if (track) {
        return track->getId();
    } else {
        return TrackId();
    }
}

// Gets the on-disk location of the track at the given location.
QString ClementinePlaylistModel::getTrackLocation(const QModelIndex& index) const {
    if (!index.isValid()) {
        return "";
    }
    QUrl url(getFieldString(index, CLM_URI));

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

bool ClementinePlaylistModel::isColumnInternal(int column) {
    return (column == fieldIndex(ColumnCache::COLUMN_PLAYLISTTRACKSTABLE_TRACKID) ||
            (PlayerManager::numPreviewDecks() == 0 &&
                    column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_PREVIEW)));
}
