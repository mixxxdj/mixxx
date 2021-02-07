#pragma once

#include <QString>

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

    void emitTrackDropped(const QString& filename, const QString& group) {
        emit trackDropped(filename, group); // clazy:exclude=incorrect-emit
    }

  signals:
    virtual void trackDropped(const QString& filename, const QString& group) = 0;
    virtual void cloneDeck(const QString& sourceGroup, const QString& targetGroup) = 0;
};
