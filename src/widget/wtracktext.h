#ifndef WTRACKTEXT_H
#define WTRACKTEXT_H

#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMouseEvent>

#include "preferences/usersettings.h"
#include "trackinfoobject.h"
#include "widget/wlabel.h"

class WTrackText : public WLabel {
    Q_OBJECT
  public:
    WTrackText(const char* group, UserSettingsPointer pConfig, QWidget *parent);
    virtual ~WTrackText();

  signals:
    void trackDropped(QString fileName, QString group);

  public slots:
    void slotTrackLoaded(TrackPointer track);
    void slotLoadingTrack(TrackPointer pNewTrack, TrackPointer pOldTrack);

  private slots:
    void updateLabel(TrackInfoObject*);

  private:
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

    const char* m_pGroup;
    UserSettingsPointer m_pConfig;
    TrackPointer m_pCurrentTrack;
};


#endif /* WTRACKTEXT_H */
