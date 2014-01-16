#ifndef WTRACKPROPERTY_H
#define WTRACKPROPERTY_H

#include <QDragEnterEvent>
#include <QDropEvent>

#include "widget/wlabel.h"
#include "trackinfoobject.h"
#include "skin/skincontext.h"

class WTrackProperty : public WLabel {
    Q_OBJECT
  public:
    WTrackProperty(const char* group, ConfigObject<ConfigValue>* pConfig, QWidget* pParent);
    virtual ~WTrackProperty();

    void setup(QDomNode node, const SkinContext& context);
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);

  signals:
    void trackDropped(QString filename, QString group);

  public slots:
    void slotTrackLoaded(TrackPointer track);
    void slotTrackUnloaded(TrackPointer track);

  private slots:
    void updateLabel(TrackInfoObject*);

  private:
    const char* m_pGroup;
    ConfigObject<ConfigValue>* m_pConfig;
    TrackPointer m_pCurrentTrack;
    QString m_property;
};


#endif /* WTRACKPROPERTY_H */
