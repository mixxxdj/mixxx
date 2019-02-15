#ifndef WTRACKTEXT_H
#define WTRACKTEXT_H

#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMouseEvent>

#include "preferences/usersettings.h"
#include "track/track.h"
#include "widget/trackdroptarget.h"
#include "widget/wlabel.h"

class WTrackText : public WLabel, public TrackDropTarget {
    Q_OBJECT
  public:
    WTrackText(const char* group, UserSettingsPointer pConfig, QWidget *pParent);

  signals:
    void trackDropped(QString fileName, QString group);
    void cloneDeck(QString source_group, QString target_group);

  public slots:
    void slotTrackLoaded(TrackPointer track);
    void slotLoadingTrack(TrackPointer pNewTrack, TrackPointer pOldTrack);

  private slots:
    void updateLabel(Track*);

  private:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

    const char* m_pGroup;
    UserSettingsPointer m_pConfig;
    TrackPointer m_pCurrentTrack;
};


#endif /* WTRACKTEXT_H */
