#ifndef WTRACKTEXT_H
#define WTRACKTEXT_H

#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMouseEvent>

#include "configobject.h"
#include "trackinfoobject.h"
#include "widget/wlabel.h"
#include "control/stringatom.h"

class WTrackText : public WLabel {
    Q_OBJECT
  public:
    WTrackText(const StringAtom& group, ConfigObject<ConfigValue>* pConfig, QWidget *parent);
    virtual ~WTrackText();

  signals:
    void trackDropped(QString fileName, StringAtom group);

  public slots:
    void slotTrackLoaded(TrackPointer track);
    void slotTrackUnloaded(TrackPointer track);

  private slots:
    void updateLabel(TrackInfoObject*);

  private:
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

    const StringAtom m_pGroup;
    ConfigObject<ConfigValue>* m_pConfig;
    TrackPointer m_pCurrentTrack;
};


#endif /* WTRACKTEXT_H */
