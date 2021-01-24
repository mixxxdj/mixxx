#pragma once

#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMouseEvent>

#include "preferences/usersettings.h"
#include "skin/skincontext.h"
#include "track/track_decl.h"
#include "track/trackid.h"
#include "util/parented_ptr.h"
#include "widget/trackdroptarget.h"
#include "widget/wlabel.h"

class Library;
class WTrackMenu;

class WTrackProperty : public WLabel, public TrackDropTarget {
    Q_OBJECT
  public:
    WTrackProperty(
            QWidget* pParent,
            UserSettingsPointer pConfig,
            Library* pLibrary,
            const QString& group);
    ~WTrackProperty() override;

    void setup(const QDomNode& node, const SkinContext& context) override;

signals:
  void trackDropped(const QString& filename, const QString& group) override;
  void cloneDeck(const QString& sourceGroup, const QString& targetGroup) override;

public slots:
  void slotTrackLoaded(TrackPointer pTrack);
  void slotLoadingTrack(TrackPointer pNewTrack, TrackPointer pOldTrack);

protected:
  void contextMenuEvent(QContextMenuEvent* event) override;

private slots:
  void slotTrackChanged(TrackId);

private:
  void dragEnterEvent(QDragEnterEvent* event) override;
  void dropEvent(QDropEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void mouseDoubleClickEvent(QMouseEvent* event) override;

  void updateLabel();

  const QString m_group;
  const UserSettingsPointer m_pConfig;
  TrackPointer m_pCurrentTrack;
  QString m_property;

  const parented_ptr<WTrackMenu> m_pTrackMenu;
};
