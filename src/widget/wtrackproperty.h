#ifndef WTRACKPROPERTY_H
#define WTRACKPROPERTY_H

#include "widget/wlabel.h"
#include "trackinfoobject.h"
#include "skin/skincontext.h"

class WTrackProperty : public WLabel {
    Q_OBJECT
  public:
    WTrackProperty(QWidget* pParent);
    virtual ~WTrackProperty();

    void setup(QDomNode node, const SkinContext& context);

  public slots:
    void slotTrackLoaded(TrackPointer track);
    void slotTrackUnloaded(TrackPointer track);

  private slots:
    void updateLabel(TrackInfoObject*);

  private:
    TrackPointer m_pCurrentTrack;
    QString m_property;
};


#endif /* WTRACKPROPERTY_H */
