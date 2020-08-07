#pragma once

#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMouseEvent>

#include "preferences/usersettings.h"
#include "track/track.h"
#include "util/parented_ptr.h"
#include "widget/trackdroptarget.h"
#include "widget/wlabel.h"

class TrackCollectionManager;
class WTrackMenu;

class WTrackText : public WLabel, public TrackDropTarget {
    Q_OBJECT
  public:
    WTrackText(
            QWidget* pParent,
            UserSettingsPointer pConfig,
            TrackCollectionManager* pTrackCollectionManager,
            const QString& group);
    ~WTrackText() override;

  signals:
    void trackDropped(QString fileName, QString group) override;
    void cloneDeck(QString source_group, QString target_group) override;

  public slots:
    void slotTrackLoaded(TrackPointer track);
    void slotLoadingTrack(TrackPointer pNewTrack, TrackPointer pOldTrack);

  private slots:
    void slotTrackChanged(TrackId);
    void contextMenuEvent(QContextMenuEvent* event) override;

  private:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

    void updateLabel();

    const QString m_group;
    UserSettingsPointer m_pConfig;
    TrackPointer m_pCurrentTrack;
    const parented_ptr<WTrackMenu> m_pTrackMenu;
};
