#ifndef WTRACKPROPERTY_H
#define WTRACKPROPERTY_H

#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMouseEvent>

#include "configobject.h"
#include "skin/skincontext.h"
#include "trackinfoobject.h"
#include "widget/wlabel.h"
#include "control/stringatom.h"

class WTrackProperty : public WLabel {
    Q_OBJECT
  public:
    WTrackProperty(const StringAtom& group, ConfigObject<ConfigValue>* pConfig, QWidget* pParent);
    virtual ~WTrackProperty();

    void setup(QDomNode node, const SkinContext& context);

  signals:
    void trackDropped(QString filename, StringAtom group);

  public slots:
    void slotTrackLoaded(TrackPointer track);
    void slotTrackUnloaded(TrackPointer track);

  private slots:
    void updateLabel(TrackInfoObject*);

  private:
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

    StringAtom m_pGroup;
    ConfigObject<ConfigValue>* m_pConfig;
    TrackPointer m_pCurrentTrack;
    QString m_property;
};


#endif /* WTRACKPROPERTY_H */
