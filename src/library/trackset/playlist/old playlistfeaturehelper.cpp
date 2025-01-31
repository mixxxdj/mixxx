#include "library/trackset/playlist/playlistfeaturehelper.h"

#include <QInputDialog>
#include <QLineEdit>

#include "library/trackcollection.h"
#include "library/trackset/playlist/playlist.h"
#include "library/trackset/playlist/playlistsummary.h"
#include "moc_playlistfeaturehelper.cpp"

PlaylistFeatureHelper::PlaylistFeatureHelper(
        TrackCollection* pTrackCollection,
        UserSettingsPointer pConfig)
        : m_pTrackCollection(pTrackCollection),
          m_pConfig(pConfig) {
}

QString PlaylistFeatureHelper::proposeNameForNewPlaylist(
        const QString& initialName) const {
    DEBUG_ASSERT(!initialName.isEmpty());
    QString proposedName;
    int suffixCounter = 0;
    do {
        if (suffixCounter++ > 0) {
            // Append suffix " 2", " 3", ...
            proposedName = QStringLiteral("%1 %2")
                                   .arg(initialName, QString::number(suffixCounter));
        } else {
            proposedName = initialName;
        }
    } while (m_pTrackCollection->playlists().readPlaylistByName(proposedName));
    // Found an unused playlist name
    return proposedName;
}

PlaylistId PlaylistFeatureHelper::createEmptyPlaylist() {
    const QString proposedPlaylistName =
            proposeNameForNewPlaylist(tr("New Playlist"));
    Playlist newPlaylist;
    for (;;) {
        bool ok = false;
        auto newName =
                QInputDialog::getText(
                        nullptr,
                        tr("Create New Playlist"),
                        tr("Enter name for new playlist:"),
                        QLineEdit::Normal,
                        proposedPlaylistName,
                        &ok)
                        .trimmed();
        if (!ok) {
            return PlaylistId();
        }
        if (newName.isEmpty()) {
            QMessageBox::warning(
                    nullptr,
                    tr("Creating Playlist Failed"),
                    tr("A playlist cannot have a blank name."));
            continue;
        }
        if (m_pTrackCollection->playlists().readPlaylistByName(newName)) {
            QMessageBox::warning(
                    nullptr,
                    tr("Creating Playlist Failed"),
                    tr("A playlist by that name already exists."));
            continue;
        }
        newPlaylist.setName(std::move(newName));
        DEBUG_ASSERT(newPlaylist.hasName());
        break;
    }

    PlaylistId newPlaylistId;
    if (m_pTrackCollection->insertPlaylist(newPlaylist, &newPlaylistId)) {
        DEBUG_ASSERT(newPlaylistId.isValid());
        newPlaylist.setId(newPlaylistId);
        qDebug() << "Created new playlist" << newPlaylist;
    } else {
        DEBUG_ASSERT(!newPlaylistId.isValid());
        qWarning() << "Failed to create new playlist"
                   << "->" << newPlaylist.getName();
        QMessageBox::warning(
                nullptr,
                tr("Creating Playlist Failed"),
                tr("An unknown error occurred while creating playlist: ") + newPlaylist.getName());
    }
    return newPlaylistId;
}

PlaylistId PlaylistFeatureHelper::duplicatePlaylist(const Playlist& oldPlaylist) {
    const QString proposedPlaylistName =
            proposeNameForNewPlaylist(
                    QStringLiteral("%1 %2")
                            .arg(oldPlaylist.getName(), tr("copy", "//:")));
    Playlist newPlaylist;
    for (;;) {
        bool ok = false;
        auto newName =
                QInputDialog::getText(
                        nullptr,
                        tr("Duplicate Playlist"),
                        tr("Enter name for new playlist:"),
                        QLineEdit::Normal,
                        proposedPlaylistName,
                        &ok)
                        .trimmed();
        if (!ok) {
            return PlaylistId();
        }
        if (newName.isEmpty()) {
            QMessageBox::warning(
                    nullptr,
                    tr("Duplicating Playlist Failed"),
                    tr("A playlist cannot have a blank name."));
            continue;
        }
        if (m_pTrackCollection->playlists().readPlaylistByName(newName)) {
            QMessageBox::warning(
                    nullptr,
                    tr("Duplicating Playlist Failed"),
                    tr("A playlist by that name already exists."));
            continue;
        }
        newPlaylist.setName(std::move(newName));
        DEBUG_ASSERT(newPlaylist.hasName());
        break;
    }

    PlaylistId newPlaylistId;
    if (m_pTrackCollection->insertPlaylist(newPlaylist, &newPlaylistId)) {
        DEBUG_ASSERT(newPlaylistId.isValid());
        newPlaylist.setId(newPlaylistId);
        qDebug() << "Created new playlist" << newPlaylist;
        QList<TrackId> trackIds;
        trackIds.reserve(
                m_pTrackCollection->playlists().countPlaylistTracks(oldPlaylist.getId()));
        {
            PlaylistTrackSelectResult playlistTracks(
                    m_pTrackCollection->playlists().selectPlaylistTracksSorted(
                            oldPlaylist.getId()));
            while (playlistTracks.next()) {
                trackIds.append(playlistTracks.trackId());
            }
        }
        if (m_pTrackCollection->addPlaylistTracks(newPlaylistId, trackIds)) {
            qDebug() << "Duplicated playlist"
                     << oldPlaylist << "->" << newPlaylist;
        } else {
            qWarning() << "Failed to copy tracks from"
                       << oldPlaylist << "into" << newPlaylist;
        }
    } else {
        qWarning() << "Failed to duplicate playlist"
                   << oldPlaylist << "->" << newPlaylist.getName();
        QMessageBox::warning(
                nullptr,
                tr("Duplicating Playlist Failed"),
                tr("An unknown error occurred while creating playlist: ") + newPlaylist.getName());
    }
    return newPlaylistId;
}
