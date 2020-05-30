#pragma once

#include "skin/skincontext.h"
#include "track/track.h"
#include "widget/wwidgetgroup.h"

class WTrackWidgetGroup : public WWidgetGroup {
    Q_OBJECT
  public:
    WTrackWidgetGroup(QWidget* pParent);
    void setup(const QDomNode& node, const SkinContext& context) override;

  public slots:
    void slotTrackLoaded(TrackPointer track);
    void slotLoadingTrack(TrackPointer pNewTrack, TrackPointer pOldTrack);

  protected:
    void paintEvent(QPaintEvent* pe) override;

  private slots:
    void slotTrackChanged(TrackId);

  private:
    void updateColor();

    TrackPointer m_pCurrentTrack;
    QColor m_trackColor;
    int m_trackColorAlpha;
};
