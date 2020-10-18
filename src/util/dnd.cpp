#include "util/dnd.h"

#include <QRegExp>

#include "control/controlobject.h"
#include "library/parserm3u.h"
#include "library/parserpls.h"
#include "mixer/playermanager.h"
#include "sources/soundsourceproxy.h"
#include "track/track.h"
#include "util/sandbox.h"

namespace {

QDrag* dragUrls(
        const QList<QUrl>& trackUrls,
        QWidget* pDragSource,
        QString sourceIdentifier) {
    if (trackUrls.isEmpty()) {
        return NULL;
    }

    QMimeData* mimeData = new QMimeData();
    mimeData->setUrls(trackUrls);
    mimeData->setText(sourceIdentifier);

    QDrag* drag = new QDrag(pDragSource);
    drag->setMimeData(mimeData);
    drag->setPixmap(QPixmap(":/images/library/ic_library_drag_and_drop.svg"));
    drag->exec(Qt::CopyAction);

    return drag;
}

bool addFileToList(
        TrackFile trackFile,
        QList<TrackFile>* trackFiles) {
    // Since the user just dropped these files into Mixxx we have permission
    // to touch the file. Create a security token to keep this permission
    // across reboots.
    Sandbox::createSecurityToken(trackFile.asFileInfo());

    if (!trackFile.checkFileExists()) {
        return false;
    }

    // Filter out invalid URLs (eg. files that aren't supported audio
    // filetypes, etc.)
    if (!SoundSourceProxy::isFileSupported(trackFile.asFileInfo())) {
        return false;
    }

    trackFiles->append(std::move(trackFile));
    return true;
}

QList<TrackFile> dropEventFiles(
        const QMimeData& mimeData,
        const QString& sourceIdentifier,
        bool firstOnly,
        bool acceptPlaylists) {
    qDebug() << "dropEventFiles()" << mimeData.hasUrls() << mimeData.urls();
    qDebug() << "mimeData.hasText()" << mimeData.hasText() << mimeData.text();

    if (!mimeData.hasUrls() ||
            (mimeData.hasText() && mimeData.text() == sourceIdentifier)) {
        return QList<TrackFile>();
    }

    return DragAndDropHelper::supportedTracksFromUrls(
            mimeData.urls(),
            firstOnly,
            acceptPlaylists);
}

// Allow loading to a player if the player isn't playing
// or the settings allow interrupting a playing player.
bool allowLoadToPlayer(
        const QString& group,
        UserSettingsPointer pConfig) {
    // Always allow loads to preview decks
    if (PlayerManager::isPreviewDeckGroup(group)) {
        return true;
    }

    // Allow if deck is not playing
    if (ControlObject::get(ConfigKey(group, "play")) <= 0.0) {
        return true;
    }

    return pConfig->getValueString(
            ConfigKey("[Controls]",
            "AllowTrackLoadToPlayingDeck")).toInt();
}

} // anonymous namespace

//static
QList<TrackFile> DragAndDropHelper::supportedTracksFromUrls(
        const QList<QUrl>& urls,
        bool firstOnly,
        bool acceptPlaylists) {
    QList<TrackFile> trackFiles;
    for (const QUrl& url : urls) {

        // XXX: Possible WTF alert - Previously we thought we needed
        // toString() here but what you actually want in any case when
        // converting a QUrl to a file system path is
        // QUrl::toLocalFile(). This is the second time we have flip-flopped
        // on this, but I think toLocalFile() should work in any
        // case. toString() absolutely does not work when you pass the
        // result to a (this comment was never finished by the original
        // author).
        QString file(url.toLocalFile());

        // If the file is on a network share, try just converting the URL to
        // a string...
        if (file.isEmpty()) {
            file = url.toString();
        }

        if (file.isEmpty()) {
            continue;
        }

        if (acceptPlaylists && (file.endsWith(".m3u") || file.endsWith(".m3u8"))) {
            QScopedPointer<ParserM3u> playlist_parser(new ParserM3u());
            QList<QString> track_list = playlist_parser->parse(file);
            foreach (const QString& playlistFile, track_list) {
                addFileToList(TrackFile(playlistFile), &trackFiles);
            }
        } else if (acceptPlaylists && url.toString().endsWith(".pls")) {
            QScopedPointer<ParserPls> playlist_parser(new ParserPls());
            QList<QString> track_list = playlist_parser->parse(file);
            foreach (const QString& playlistFile, track_list) {
                addFileToList(TrackFile(playlistFile), &trackFiles);
            }
        } else {
            addFileToList(TrackFile::fromUrl(url), &trackFiles);
        }

        if (firstOnly && !trackFiles.isEmpty()) {
            break;
        }
    }

    return trackFiles;
}

//static
bool DragAndDropHelper::allowDeckCloneAttempt(
        const QDropEvent& event,
        const QString& group) {
    // only allow clones to decks
    if (!PlayerManager::isDeckGroup(group, nullptr)) {
        return false;
    }

    // forbid clone if shift is pressed
    if (event.keyboardModifiers().testFlag(Qt::ShiftModifier)) {
        return false;
    }

    if (!event.mimeData()->hasText() ||
            // prevent cloning to ourself
            event.mimeData()->text() == group ||
            // only allow clone from decks
            !PlayerManager::isDeckGroup(event.mimeData()->text(), nullptr)) {
        return false;
    }

    return true;
}

//static
bool DragAndDropHelper::dragEnterAccept(
        const QMimeData& mimeData,
        const QString& sourceIdentifier,
        bool firstOnly,
        bool acceptPlaylists) {
    // TODO(XXX): This operation blocks the UI when many
    // files are selected!
    QList<TrackFile> files = dropEventFiles(mimeData, sourceIdentifier, firstOnly, acceptPlaylists);
    return !files.isEmpty();
}

//static
QDrag* DragAndDropHelper::dragTrack(
        TrackPointer pTrack,
        QWidget* pDragSource,
        QString sourceIdentifier) {
    QList<QUrl> trackUrls;
    trackUrls.append(pTrack->getFileInfo().toUrl());
    return dragUrls(trackUrls, pDragSource, sourceIdentifier);
}

//static
QDrag* DragAndDropHelper::dragTrackLocations(
        const QList<QString>& locations,
        QWidget* pDragSource,
        QString sourceIdentifier) {
    QList<QUrl> trackUrls;
    foreach (QString location, locations) {
        trackUrls.append(TrackFile(location).toUrl());
    }
    return dragUrls(trackUrls, pDragSource, sourceIdentifier);
}

//static
void DragAndDropHelper::handleTrackDragEnterEvent(
        QDragEnterEvent* event,
        const QString& group,
        UserSettingsPointer pConfig) {
    if (allowLoadToPlayer(group, pConfig) &&
            dragEnterAccept(*event->mimeData(), group, true, false)) {
        event->acceptProposedAction();
    } else {
        qDebug() << "Ignoring drag enter event, loading not allowed";
        event->ignore();
    }
}

//static
void DragAndDropHelper::handleTrackDropEvent(
        QDropEvent* event,
        TrackDropTarget& target,
        const QString& group,
        UserSettingsPointer pConfig) {
    if (allowLoadToPlayer(group, pConfig)) {
        if (allowDeckCloneAttempt(*event, group)) {
            event->accept();
            emit target.cloneDeck(event->mimeData()->text(), group);
            return;
        } else {
            QList<TrackFile> files = dropEventFiles(
                    *event->mimeData(), group, true, false);
            if (!files.isEmpty()) {
                event->accept();
                target.emitTrackDropped(files.at(0).location(), group);
                return;
            }
        }
    }
    event->ignore();
}
