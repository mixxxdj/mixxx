#ifndef TRACKSELECTOR_H
#define TRACKSELECTOR_H

#include <QObject>

#include "library/playlisttablemodel.h"
#include "track/track.h"
#include "util/class.h"


class ITrackSelector
{
  public:
    virtual ~ITrackSelector(){} // do not forget this

    virtual const QString getName() = 0;

    virtual const QString getId() = 0;

    virtual TrackPointer getNextTrackFromQueue() = 0;


};

Q_DECLARE_INTERFACE(ITrackSelector, "ITrackSelector") // define this out of namespace scope


/*!
    \class TrackSelector
    \brief Base class that is responsible for selecting the next AutoDJ tack
*/

class TrackSelector : public QObject, public ITrackSelector {
    Q_OBJECT
    Q_INTERFACES(ITrackSelector)
  public:
    TrackSelector(AutoDJProcessor* processor);

  protected:
    AutoDJProcessor* m_processor;
    char* m_id;
};

/*
class TrackSelector : public QObject {
    Q_OBJECT
  public:
    TrackSelector(AutoDJProcessor* processor);

    virtual const QString getName() = 0;

    virtual const char* getId() = 0;

    virtual TrackPointer getNextTrackFromQueue() = 0;

  protected:
    AutoDJProcessor* m_processor;
    char* m_id;
};
*/

class LinearSelector: public TrackSelector {
    Q_OBJECT
  public:
    LinearSelector(AutoDJProcessor* processor);

    const QString getName() {
        return tr("Linear");
    }

    const QString getId() {
        return QString("linear");
    }
    TrackPointer getNextTrackFromQueue();
};


class RandomSelector: public TrackSelector {
    Q_OBJECT
  public:
    RandomSelector(AutoDJProcessor* processor);

    const QString getName() {
        return tr("Random");
    }

    const QString getId() {
        return QString("random");
    }
    TrackPointer getNextTrackFromQueue();
};


#endif // TRACKSELECTOR_H
