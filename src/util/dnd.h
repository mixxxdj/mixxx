#pragma once

#include <QDrag>
#include <QDropEvent>
#include <QList>
#include <QMimeData>
#include <QString>
#include <QUrl>

#include "preferences/usersettings.h"
#include "track/track_decl.h"
#include "util/fileinfo.h"
#include "widget/trackdroptarget.h"

class DragAndDropHelper final {
  public:
    DragAndDropHelper() = delete;

    static QList<mixxx::FileInfo> supportedTracksFromUrls(
            const QList<QUrl>& urls,
            bool firstOnly,
            bool acceptPlaylists);

    static bool allowDeckCloneAttempt(
            const QDropEvent& event,
            const QString& group);

    static bool dragEnterAccept(
            const QMimeData& mimeData,
            const QString& sourceIdentifier,
            bool firstOnly,
            bool acceptPlaylists);

    static QDrag* dragTrack(
            TrackPointer pTrack,
            QWidget* pDragSource,
            const QString& sourceIdentifier);

#ifdef __STEM__
    static QDrag* dragTrackLocations(
            const QList<QString>& locations,
            const QList<mixxx::StemChannelSelection>& stemMasks,
            QWidget* pDragSource,
            const QString& sourceIdentifier);
#else
    static QDrag* dragTrackLocations(
            const QList<QString>& locations,
            QWidget* pDragSource,
            const QString& sourceIdentifier);
#endif

    static void handleTrackDragEnterEvent(
            QDragEnterEvent* pEvent,
            const QString& group,
            UserSettingsPointer pConfig);

    static void handleTrackDropEvent(
            QDropEvent* pEvent,
            TrackDropTarget& target,
            const QString& group,
            UserSettingsPointer pConfig);

    static void mousePressed(QMouseEvent* pEvent);

    static bool mouseMoveInitiatesDrag(QMouseEvent* pEvent);
};
