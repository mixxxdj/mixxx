#pragma once

#include <QEvent>
#include <QString>

#ifdef __STEM__
#include "engine/engine.h"
#endif

/// Mixin to mark a widget as a drop target for tracks.
///
/// This class is *not* derived from QObject (inheriting from 2 QObject classes
/// is not possible). Therefore, the signals are marked as pure-virtual and
/// you need to use the `emitFoo` methods to emit a signal.
class TrackDropTarget {
  public:
    virtual ~TrackDropTarget() {
    }

    void emitCloneDeck(const QString& sourceGroup, const QString& targetGroup) {
        emit cloneDeck(sourceGroup, targetGroup); // clazy:exclude=incorrect-emit
    }

#ifdef __STEM__
    void emitTrackDropped(const QString& filename,
            const QString& group,
            mixxx::StemChannelSelection stemMask = mixxx::StemChannel::All) {
        qDebug() << "[TrackDropTarget] -> emitTrackDropped -> filename "
                 << filename << " group: " << group
                 << " stemMask: " << stemMask;
        emit trackDropped(filename, group, stemMask);
    }
#else
    void emitTrackDropped(const QString& filename, const QString& group) {
        emit trackDropped(filename, group); // clazy:exclude=incorrect-emit
    }
#endif

    virtual bool handleDragAndDropEventFromWindow(QEvent* pEvent) {
        pEvent->ignore();
        return false;
    }

  signals:
#ifdef __STEM__
    virtual void trackDropped(const QString& filename,
            const QString& group,
            mixxx::StemChannelSelection stemMask) = 0;
#else
    virtual void trackDropped(const QString& filename, const QString& group) = 0;
#endif

    virtual void cloneDeck(const QString& sourceGroup, const QString& targetGroup) = 0;
};
