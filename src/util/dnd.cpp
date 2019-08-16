#include "util/dnd.h"

#include <QRegExp>

#include "control/controlobject.h"
#include "library/parserm3u.h"
#include "library/parserpls.h"
#include "mixer/playermanager.h"
#include "sources/soundsourceproxy.h"
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
        const QString& file,
        QList<QFileInfo>* files) {
    QFileInfo fileInfo(file);

    // Since the user just dropped these files into Mixxx we have permission
    // to touch the file. Create a security token to keep this permission
    // across reboots.
    Sandbox::createSecurityToken(fileInfo);

    if (!fileInfo.exists()) {
        return false;
    }

    // Filter out invalid URLs (eg. files that aren't supported audio
    // filetypes, etc.)
    if (!SoundSourceProxy::isFileSupported(fileInfo)) {
        return false;
    }

    files->append(fileInfo);
    return true;
}

} // anonymous namespace

//static
QList<QFileInfo> DragAndDropHelper::supportedTracksFromUrls(
        const QList<QUrl>& urls,
        bool firstOnly,
        bool acceptPlaylists) {
    QList<QFileInfo> fileLocations;
    for (QUrl url : urls) {

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
                addFileToList(playlistFile, &fileLocations);
            }
        } else if (acceptPlaylists && url.toString().endsWith(".pls")) {
            QScopedPointer<ParserPls> playlist_parser(new ParserPls());
            QList<QString> track_list = playlist_parser->parse(file);
            foreach (const QString& playlistFile, track_list) {
                addFileToList(playlistFile, &fileLocations);
            }
        } else {
            addFileToList(file, &fileLocations);
        }

        if (firstOnly && !fileLocations.isEmpty()) {
            break;
        }
    }

    return fileLocations;
}

//static
bool DragAndDropHelper::allowLoadToPlayer(
        const QString& group,
        UserSettingsPointer pConfig) {
    return allowLoadToPlayer(
            group, ControlObject::get(ConfigKey(group, "play")) > 0.0, pConfig);
}

//static
bool DragAndDropHelper::allowLoadToPlayer(
        const QString& group,
        bool isPlaying,
        UserSettingsPointer pConfig) {
    // Always allow loads to preview decks.
    if (PlayerManager::isPreviewDeckGroup(group)) {
        return true;
    }

    return !isPlaying ||
            pConfig->getValueString(ConfigKey("[Controls]", "AllowTrackLoadToPlayingDeck")).toInt();
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
    QList<QFileInfo> files = dropEventFiles(mimeData, sourceIdentifier, firstOnly, acceptPlaylists);
    return !files.isEmpty();
}

//static
QList<QFileInfo> DragAndDropHelper::dropEventFiles(
        const QMimeData& mimeData,
        const QString& sourceIdentifier,
        bool firstOnly,
        bool acceptPlaylists) {
    qDebug() << "dropEventFiles()" << mimeData.hasUrls() << mimeData.urls();
    qDebug() << "mimeData.hasText()" << mimeData.hasText() << mimeData.text();

    if (!mimeData.hasUrls() ||
            (mimeData.hasText() && mimeData.text() == sourceIdentifier)) {
        return QList<QFileInfo>();
    }

    QList<QFileInfo> files = DragAndDropHelper::supportedTracksFromUrls(
            mimeData.urls(), firstOnly, acceptPlaylists);
    return files;
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
            emit(target.cloneDeck(event->mimeData()->text(), group));
            return;
        } else {
            QList<QFileInfo> files = dropEventFiles(
                    *event->mimeData(), group, true, false);
            if (!files.isEmpty()) {
                event->accept();
                target.emitTrackDropped(files.at(0).absoluteFilePath(), group);
                return;
            }
        }
    }
    event->ignore();
}
