#include "library/baseexternalplaylistmodel.h"

#include "library/dao/trackschema.h"
#include "library/queryutil.h"
#include "library/trackcollection.h"
#include "library/trackcollectionmanager.h"
#include "mixer/playermanager.h"
#include "moc_baseexternalplaylistmodel.cpp"
#include "track/track.h"
#include "track/track_decl.h"

namespace {

const QString kModelName = "external:";

} // anonymous namespace

BaseExternalPlaylistModel::BaseExternalPlaylistModel(QObject* parent,
        TrackCollectionManager* pTrackCollectionManager,
        const char* settingsNamespace,
        const QString& playlistsTable,
        const QString& playlistTracksTable,
        QSharedPointer<BaseTrackCache> trackSource)
        : BaseSqlTableModel(parent, pTrackCollectionManager, settingsNamespace),
          m_playlistsTable(playlistsTable),
          m_playlistTracksTable(playlistTracksTable),
          m_trackSource(trackSource),
          m_currentPlaylistId(kInvalidPlaylistId) {
}

BaseExternalPlaylistModel::~BaseExternalPlaylistModel() {
}

TrackPointer BaseExternalPlaylistModel::getTrack(const QModelIndex& index) const {
    QString location = getTrackLocation(index);

    if (location.isEmpty()) {
        // Track is lost
        return TrackPointer();
    }

    bool track_already_in_library = false;
    TrackPointer pTrack = m_pTrackCollectionManager->getOrAddTrack(
            TrackRef::fromFilePath(location),
            &track_already_in_library);

    // If this track was not in the Mixxx library it is now added and will be
    // saved with the metadata from iTunes. If it was already in the library
    // then we do not touch it so that we do not over-write the user's metadata.
    if (pTrack && !track_already_in_library) {
        QString artist = getFieldString(index, ColumnCache::COLUMN_LIBRARYTABLE_ARTIST);
        pTrack->setArtist(artist);

        QString title = getFieldString(index, ColumnCache::COLUMN_LIBRARYTABLE_TITLE);
        pTrack->setTitle(title);

        QString album = getFieldString(index, ColumnCache::COLUMN_LIBRARYTABLE_ALBUM);
        pTrack->setAlbum(album);

        QString year = getFieldString(index, ColumnCache::COLUMN_LIBRARYTABLE_YEAR);
        pTrack->setYear(year);

        QString genre = getFieldString(index, ColumnCache::COLUMN_LIBRARYTABLE_GENRE);
        updateTrackGenre(pTrack.get(), genre);

        float bpm = getFieldVariant(index, ColumnCache::COLUMN_LIBRARYTABLE_BPM).toFloat();
        pTrack->trySetBpm(bpm);
    }
    return pTrack;
}

QString BaseExternalPlaylistModel::resolveLocation(const QString& nativeLocation) const {
    return QDir::fromNativeSeparators(nativeLocation);
}

QString BaseExternalPlaylistModel::getTrackLocation(const QModelIndex& index) const {
    QString nativeLocation = index.sibling(index.row(), fieldIndex("location")).data().toString();
    return resolveLocation(nativeLocation);
}

TrackId BaseExternalPlaylistModel::getTrackId(const QModelIndex& index) const {
    const auto track = getTrack(index);
    if (track) {
        return track->getId();
    } else {
        return TrackId();
    }
}

bool BaseExternalPlaylistModel::isColumnInternal(int column) {
    return column == fieldIndex(ColumnCache::COLUMN_PLAYLISTTRACKSTABLE_TRACKID) ||
            (PlayerManager::numPreviewDecks() == 0 &&
                    column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_PREVIEW));
}

Qt::ItemFlags BaseExternalPlaylistModel::flags(const QModelIndex& index) const {
    return readOnlyFlags(index);
}

void BaseExternalPlaylistModel::setPlaylist(const QString& playlist_path) {
    QSqlQuery finder_query(m_database);
    finder_query.prepare(QString("SELECT id from %1 where name=:name").arg(m_playlistsTable));
    finder_query.bindValue(":name", playlist_path);

    if (!finder_query.exec()) {
        LOG_FAILED_QUERY(finder_query) << "Error getting id for playlist:" << playlist_path;
        return;
    }

    // TODO(XXX): Why not last-insert id?
    int playlistId = kInvalidPlaylistId;
    QSqlRecord finder_query_record = finder_query.record();
    while (finder_query.next()) {
        playlistId = finder_query.value(finder_query_record.indexOf("id")).toInt();
    }

    if (playlistId == kInvalidPlaylistId) {
        qWarning() << "ERROR: Could not get the playlist ID for playlist:" << playlist_path;
        return;
    }

    setPlaylistById(playlistId);
}

void BaseExternalPlaylistModel::setPlaylistById(int playlistId) {
    // Store search text
    QString currSearch = currentSearch();
    if (m_currentPlaylistId != kInvalidPlaylistId) {
        if (!currSearch.trimmed().isEmpty()) {
            m_searchTexts.insert(m_currentPlaylistId, currSearch);
        } else {
            m_searchTexts.remove(m_currentPlaylistId);
        }
    }

    const auto playlistIdNumber =
            QString::number(playlistId);
    const auto playlistViewTable =
            QStringLiteral("%1_%2")
                    .arg(
                            m_playlistTracksTable,
                            playlistIdNumber);
    // The ordering of columns is relevant (see below)!
    auto playlistViewColumns = QStringList{
            PLAYLISTTRACKSTABLE_TRACKID,
            PLAYLISTTRACKSTABLE_POSITION,
            QStringLiteral("'' AS ") + LIBRARYTABLE_PREVIEW};
    const auto queryString =
            QStringLiteral(
                    "CREATE TEMPORARY VIEW IF NOT EXISTS %1 AS "
                    "SELECT %2 FROM %3 WHERE playlist_id=%4")
                    .arg(FieldEscaper(m_database)
                                    .escapeString(playlistViewTable),
                            playlistViewColumns.join(","),
                            m_playlistTracksTable,
                            // Using bindValue() for playlist_id would fail: Parameter count mismatch
                            playlistIdNumber);

    QSqlQuery query(m_database);
    query.prepare(queryString);
    if (!query.exec()) {
        LOG_FAILED_QUERY(query) << "Error creating temporary view for playlist.";
        return;
    }

    m_currentPlaylistId = playlistId;
    playlistViewColumns.last() = LIBRARYTABLE_PREVIEW;
    setTable(playlistViewTable, playlistViewColumns.first(), playlistViewColumns, m_trackSource);
    setDefaultSort(fieldIndex(ColumnCache::COLUMN_PLAYLISTTRACKSTABLE_POSITION),
            Qt::AscendingOrder);
    // Restore search text
    setSearch(m_searchTexts.value(m_currentPlaylistId));
}

TrackId BaseExternalPlaylistModel::doGetTrackId(const TrackPointer& pTrack) const {
    if (pTrack) {
        // The external table has foreign Track IDs, so we need to compare
        // by location
        for (int row = 0; row < rowCount(); ++row) {
            QString nativeLocation = getFieldString(index(row, 0),
                    ColumnCache::COLUMN_TRACKLOCATIONSTABLE_LOCATION);
            QString location = QDir::fromNativeSeparators(nativeLocation);
            if (location == pTrack->getLocation()) {
                return TrackId(index(row, 0).data());
            }
        }
    }
    return TrackId();
}

TrackModel::Capabilities BaseExternalPlaylistModel::getCapabilities() const {
    return Capability::AddToTrackSet |
            Capability::AddToAutoDJ |
            Capability::LoadToDeck |
            Capability::LoadToPreviewDeck |
            Capability::LoadToSampler |
            Capability::Sorting;
}

QString BaseExternalPlaylistModel::modelKey(bool noSearch) const {
    if (noSearch) {
        return kModelName +
                QString::number(m_currentPlaylistId);
    }
    return kModelName +
            QString::number(m_currentPlaylistId) +
            QStringLiteral("#") +
            currentSearch();
}
