#ifndef WTRACKTEXT_H
#define WTRACKTEXT_H

#include <QDropEvent>

#include "widget/wlabel.h"
#include "trackinfoobject.h"

class WTrackText : public WLabel {
    Q_OBJECT
  public:
    WTrackText(QWidget *parent);
    virtual ~WTrackText();

    void dropEvent(QDropEvent *event);

  signals:
    void trackDropped(QString fileName, QString group);

  public slots:
    void slotTrackLoaded(TrackPointer track);
    void slotTrackUnloaded(TrackPointer track);

  private slots:
    void updateLabel(TrackInfoObject*);

  private:
    TrackPointer m_pCurrentTrack;
};

#endif /* WTRACKTEXT_H */
