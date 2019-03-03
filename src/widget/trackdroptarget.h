
#ifndef TRACKDROPTARGET_H
#define TRACKDROPTARGET_H

#include <QString>

class TrackDropTarget {
  public:
    virtual ~TrackDropTarget() {
    }

  void emitTrackDropped(const QString& filename, const QString& group) {
      emit(trackDropped(filename, group));
  }

  signals:
    virtual void trackDropped(QString filename, QString group) = 0;
};

#endif // TRACKDROPTARGET
