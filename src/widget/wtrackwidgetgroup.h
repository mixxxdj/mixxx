#pragma once

#include "skin/skincontext.h"
#include "track/track.h"
#include "util/parented_ptr.h"
#include "widget/wwidgetgroup.h"

class WTrackMenu;
class TrackCollectionManager;

class WTrackWidgetGroup : public WWidgetGroup {
    Q_OBJECT
  public:
    WTrackWidgetGroup(QWidget* pParent,
            UserSettingsPointer pConfig,
            TrackCollectionManager* pTrackCollectionManager);
    ~WTrackWidgetGroup() override;
    void setup(const QDomNode& node, const SkinContext& context) override;

  public slots:
    void slotTrackLoaded(TrackPointer track);
    void slotLoadingTrack(TrackPointer pNewTrack, TrackPointer pOldTrack);

  protected:
    void paintEvent(QPaintEvent* pe) override;
    void contextMenuEvent(QContextMenuEvent* event) override;

  private slots:
    void slotTrackChanged(TrackId);

  private:
    void updateColor();

    TrackPointer m_pCurrentTrack;
    QColor m_trackColor;
    int m_trackColorAlpha;

    const parented_ptr<WTrackMenu> m_pTrackMenu;
};
