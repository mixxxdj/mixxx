#include "util/dnd.h"

#include "control/controlobject.h"
#include "library/parser.h"
#include "mixer/playermanager.h"
#include "preferences/dialog/dlgprefdeck.h"
#include "sources/soundsourceproxy.h"
#include "track/track.h"

namespace {

QDrag* dragUrls(
        const QList<QUrl>& trackUrls,
        QWidget* pDragSource,
        const QString& sourceIdentifier) {
    if (trackUrls.isEmpty()) {
        return nullptr;
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
        mixxx::FileInfo fileInfo,
        QList<mixxx::FileInfo>* fileInfos) {
    if (!fileInfo.checkFileExists()) {
        return false;
    }

    // Filter out invalid URLs (eg. files that aren't supported audio
    // filetypes, etc.)
    if (!SoundSourceProxy::isFileSupported(fileInfo)) {
        return false;
    }

    fileInfos->append(std::move(fileInfo));
    return true;
}

// EVE - OLD
// QList<mixxx::FileInfo> dropEventFiles(
//        const QMimeData& mimeData,
//        const QString& sourceIdentifier,
//        bool firstOnly,
//        bool acceptPlaylists) {
//    qDebug() << "dropEventFiles()" << mimeData.hasUrls() << mimeData.urls();
//    qDebug() << "mimeData.hasText()" << mimeData.hasText() << mimeData.text();
//
//    if (!mimeData.hasUrls() ||
//            (mimeData.hasText() && mimeData.text() == sourceIdentifier)) {
//        return {};
//    }
//
//    return DragAndDropHelper::supportedTracksFromUrls(
//            mimeData.urls(),
//            firstOnly,
//            acceptPlaylists);
//}

#ifdef __STEM__
QList<QPair<mixxx::FileInfo, mixxx::StemChannelSelection>> dropEventFiles(
#else
QList<mixxx::FileInfo> dropEventFiles(
#endif
        const QMimeData& mimeData,
        const QString& sourceIdentifier,
        bool firstOnly,
        bool acceptPlaylists) {
    qDebug() << "[dropEventFiles()] -> mimeData.hasUrls()" << mimeData.hasUrls() << mimeData.urls();
    qDebug() << "[dropEventFiles()] -> mimeData.hasText()" << mimeData.hasText() << mimeData.text();

    // NO URLS | text matches the source identifier -> empty list
    if (!mimeData.hasUrls() ||
            (mimeData.hasText() && mimeData.text() == sourceIdentifier)) {
        return {};
    }

    // LIST -> supported tracks from URLs
    auto trackList = DragAndDropHelper::supportedTracksFromUrls(
            mimeData.urls(),
            firstOnly,
            acceptPlaylists);

#ifdef __STEM__
    // Extract stemMask if available
    mixxx::StemChannelSelection stemMask = mixxx::StemChannel::All; // Default: full track
    qDebug() << "[dropEventFiles()] -> before if (mimeData...";

    if (mimeData.hasFormat("application/x-mixxx-stem-mask")) {
        QByteArray data = mimeData.data("application/x-mixxx-stem-mask");
        QDataStream stream(&data, QIODevice::ReadOnly);

        // Default
        int stemValue = static_cast<int>(mixxx::StemChannel::All);
        qDebug() << "[dropEventFiles()] -> stemValue before deserialization:" << stemValue;

        // Deserialize the stem value
        stream >> stemValue;
        stemMask = static_cast<mixxx::StemChannelSelection>(stemValue);

        if (stream.status() != QDataStream::Ok) {
            qDebug() << "[dropEventFiles()] -> Failed to extract stem mask!";
        } else {
            qDebug() << "[dropEventFiles()] -> Successfully extracted stem mask:" << stemMask;
        }
    } else {
        qDebug() << "[dropEventFiles()] -> No stem mask format found!";
    }

    // Check stemMask
    if (stemMask == mixxx::StemChannel::All) {
        qDebug() << "[dropEventFiles()] -> Default stem mask applied: All";
    } else {
        qDebug() << "[dropEventFiles()] -> Custom stem mask applied:" << stemMask;
    }

    // Result list & pair= track + stemMask
    QList<QPair<mixxx::FileInfo, mixxx::StemChannelSelection>> result;
    for (const auto& track : trackList) {
        result.append(qMakePair(track, stemMask));
    }
    qDebug() << "[dropEventFiles()] -> stem-result:" << result;
    return result;
#else
    return trackList;
#endif
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

    bool allowLoadTrackIntoPlayingDeck = false;
    if (pConfig->exists(kConfigKeyLoadWhenDeckPlaying)) {
        int loadWhenDeckPlaying =
                pConfig->getValueString(kConfigKeyLoadWhenDeckPlaying).toInt();
        switch (static_cast<LoadWhenDeckPlaying>(loadWhenDeckPlaying)) {
        case LoadWhenDeckPlaying::Allow:
        case LoadWhenDeckPlaying::AllowButStopDeck:
            allowLoadTrackIntoPlayingDeck = true;
            break;
        case LoadWhenDeckPlaying::Reject:
            break;
        }
    } else {
        // support older version of this flag
        allowLoadTrackIntoPlayingDeck =
                pConfig->getValue<bool>(kConfigKeyAllowTrackLoadToPlayingDeck);
    }
    return allowLoadTrackIntoPlayingDeck;
}

// Helper function for DragAndDropHelper::mousePressed and DragAndDropHelper::mouseMoveInitiatesDrag
bool mouseMoveInitiatesDragHelper(QMouseEvent* pEvent, bool isPress) {
    if (pEvent->buttons() != Qt::LeftButton) {
        return false;
    }
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    const qreal x = pEvent->position().x();
    const qreal y = pEvent->position().y();
#else
    const qreal x = pEvent->x();
    const qreal y = pEvent->y();
#endif

    static qreal pressX{};
    static qreal pressY{};

    if (isPress) {
        pressX = x;
        pressY = y;
        return false;
    }

    const qreal threshold = static_cast<qreal>(QApplication::startDragDistance());
    // X and Y distance since press
    const qreal dx = x - pressX;
    const qreal dy = y - pressY;

    // Check if the distance surpasses the threshold. Per Pythagoras Theorem:
    // distance = sqrt(dx^2 + dy^2)
    // We can avoid the sqrt by squaring the threshold.

    return dx * dx + dy * dy >= threshold * threshold;
}

} // anonymous namespace

//static
QList<mixxx::FileInfo> DragAndDropHelper::supportedTracksFromUrls(
        const QList<QUrl>& urls,
        bool firstOnly,
        bool acceptPlaylists) {
    QList<mixxx::FileInfo> fileInfos;
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

        if (acceptPlaylists && Parser::isPlaylistFilenameSupported(file)) {
            const QList<QString> track_list = Parser::parse(file);
            for (const auto& playlistFile : track_list) {
                addFileToList(mixxx::FileInfo(playlistFile), &fileInfos);
            }
        } else {
            addFileToList(mixxx::FileInfo::fromQUrl(url), &fileInfos);
        }

        if (firstOnly && !fileInfos.isEmpty()) {
            break;
        }
    }

    return fileInfos;
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
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    const auto modifiers = event.modifiers();
#else
    const auto modifiers = event.keyboardModifiers();
#endif
    if (modifiers.testFlag(Qt::ShiftModifier)) {
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

// static
void DragAndDropHelper::mousePressed(QMouseEvent* pEvent) {
    mouseMoveInitiatesDragHelper(pEvent, true);
}

// static
bool DragAndDropHelper::mouseMoveInitiatesDrag(QMouseEvent* pEvent) {
    return mouseMoveInitiatesDragHelper(pEvent, false);
}

//static
bool DragAndDropHelper::dragEnterAccept(
        const QMimeData& mimeData,
        const QString& sourceIdentifier,
        bool firstOnly,
        bool acceptPlaylists) {
    // TODO(XXX): This operation blocks the UI when many
    // files are selected!
    const auto files = dropEventFiles(mimeData, sourceIdentifier, firstOnly, acceptPlaylists);
    return !files.isEmpty();
}

//static
QDrag* DragAndDropHelper::dragTrack(
        TrackPointer pTrack,
        QWidget* pDragSource,
        const QString& sourceIdentifier) {
    QList<QUrl> trackUrls;
    trackUrls.append(pTrack->getFileInfo().toQUrl());
    return dragUrls(trackUrls, pDragSource, sourceIdentifier);
}

// EVE - OLD
// static
// QDrag* DragAndDropHelper::dragTrackLocations(
//        const QList<QString>& locations,
//        QWidget* pDragSource,
//        const QString& sourceIdentifier) {
//    QList<QUrl> trackUrls;
//    foreach (QString location, locations) {
//        trackUrls.append(mixxx::FileInfo(location).toQUrl());
//    }
//    return dragUrls(trackUrls, pDragSource, sourceIdentifier);
//}

// static
#ifdef __STEM__
QDrag* DragAndDropHelper::dragTrackLocations(
        const QList<QString>& locations,
        const QList<mixxx::StemChannelSelection>& stemMasks,
        QWidget* pDragSource,
        const QString& sourceIdentifier) {
    Q_UNUSED(sourceIdentifier);
    qDebug() << "[DragAndDropHelper::dragTrackLocations] -> stemMasks " << stemMasks;
#else
QDrag* DragAndDropHelper::dragTrackLocations(
        const QList<QString>& locations,
        QWidget* pDragSource,
        const QString& sourceIdentifier) {
#endif
    QList<QUrl> trackUrls;
    for (const QString& location : locations) {
        trackUrls.append(mixxx::FileInfo(location).toQUrl());
    }

    QMimeData* pMimeData = new QMimeData;
    pMimeData->setUrls(trackUrls);

#ifdef __STEM__
    if (!stemMasks.isEmpty()) {
        QByteArray stemMaskData;
        QDataStream stream(&stemMaskData, QIODevice::WriteOnly);

        // Serialize the list of stem masks to check
        for (const mixxx::StemChannelSelection& mask : stemMasks) {
            stream << mask;
        }

        // Add the serialized stem mask data to the MIME data
        pMimeData->setData("application/x-mixxx-stem-mask", stemMaskData);
        qDebug() << "[DragAndDropHelper::dragTrackLocations] -> Serialized "
                    "stemMasks:"
                 << stemMaskData.toHex();
    }
#endif

    QDrag* pDrag = new QDrag(pDragSource);
    pDrag->setMimeData(pMimeData);
    // Qt::DropAction dropAction = pDrag->exec(Qt::CopyAction | Qt::MoveAction);
    pDrag->exec(Qt::CopyAction | Qt::MoveAction);
    return pDrag;
}

//static
void DragAndDropHelper::handleTrackDragEnterEvent(
        QDragEnterEvent* pEvent,
        const QString& group,
        UserSettingsPointer pConfig) {
    if (allowLoadToPlayer(group, pConfig) &&
            dragEnterAccept(*pEvent->mimeData(), group, true, false)) {
        pEvent->acceptProposedAction();
    } else {
        qDebug() << "Ignoring drag enter event, loading not allowed";
        pEvent->ignore();
    }
}

// static
void DragAndDropHelper::handleTrackDropEvent(
        QDropEvent* pEvent,
        TrackDropTarget& target,
        const QString& group,
        UserSettingsPointer pConfig) {
    if (allowLoadToPlayer(group, pConfig)) {
        if (allowDeckCloneAttempt(*pEvent, group)) {
            pEvent->accept();
            target.emitCloneDeck(pEvent->mimeData()->text(), group);
            return;
        } else {
#ifdef __STEM__
            const QList<QPair<mixxx::FileInfo, mixxx::StemChannelSelection>> files = dropEventFiles(
                    *pEvent->mimeData(), group, true, false);
            if (!files.isEmpty()) {
                pEvent->accept();
                qDebug() << "[DragAndDropHelper::handleTrackDropEvent] -> "
                            "before emitTrackDropped files "
                         << files;
                target.emitTrackDropped(files.at(0).first.location(), group, files.at(0).second);
#else
            const QList<mixxx::FileInfo> files = dropEventFiles(
                    *pEvent->mimeData(), group, true, false);
            if (!files.isEmpty()) {
                pEvent->accept();
                target.emitTrackDropped(files.at(0).location(), group);
#endif
                return;
            }
        }
    }
    pEvent->ignore();
}
