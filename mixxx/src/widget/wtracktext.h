#ifndef WTRACKTEXT_H
#define WTRACKTEXT_H

#include "widget/wlabel.h"
#include "trackinfoobject.h"

class WTrackText : public WLabel {
    Q_OBJECT
  public:
    WTrackText(QWidget *parent);
    virtual ~WTrackText();

  public slots:
    void slotTrackLoaded(TrackPointer track);
    void slotTrackUnloaded(TrackPointer track);

  private slots:
    void updateLabel(TrackInfoObject*);

  private:
    TrackPointer m_pCurrentTrack;
};

#endif /* WTRACKTEXT_H */
