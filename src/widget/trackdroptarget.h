
#ifndef TRACKDROPTARGET_H
#define TRACKDROPTARGET_H

#include <QString>

class TrackDropTarget {
signals:
    virtual void trackDropped(QString filename, QString group) = 0;
    virtual void cloneDeck(QString source_group, QString target_group) = 0;
};

#endif // TRACKDROPTARGET
