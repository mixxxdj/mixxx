#pragma once

#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMouseEvent>

#include "preferences/usersettings.h"
#include "track/track_decl.h"
#include "track/trackid.h"
#include "util/parented_ptr.h"
#include "widget/trackdroptarget.h"
#include "widget/wlabel.h"

class Library;
class WTrackMenu;

class WTrackText : public WLabel, public TrackDropTarget {
    Q_OBJECT
  public:
    WTrackText(
            QWidget* pParent,
            UserSettingsPointer pConfig,
            Library* pLibrary,
            const QString& group);
    ~WTrackText() override;

  signals:
    void trackDropped(const QString& fileName, const QString& group) override;
    void cloneDeck(const QString& sourceGroup, const QString& targetGroup) override;

  public slots:
    void slotTrackLoaded(TrackPointer track);
    void slotLoadingTrack(TrackPointer pNewTrack, TrackPointer pOldTrack);

  protected:
    void contextMenuEvent(QContextMenuEvent* event) override;

  private slots:
    void slotTrackChanged(TrackId);

  private:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;

    void updateLabel();

    const QString m_group;
    UserSettingsPointer m_pConfig;
    TrackPointer m_pCurrentTrack;
    const parented_ptr<WTrackMenu> m_pTrackMenu;
};
