#pragma once

#include <QDialog>

#include "library/ui_dlgcoverartfullsize.h"
#include "library/coverart.h"
#include "mixer/basetrackplayer.h"
#include "track/track.h"
#include "widget/wcoverartmenu.h"
#include "util/parented_ptr.h"

class DlgCoverArtFullSize
        : public QDialog,
          public Ui::DlgCoverArtFullSize {
    Q_OBJECT
  public:
    explicit DlgCoverArtFullSize(QWidget* parent, BaseTrackPlayer* pPlayer = nullptr);
    ~DlgCoverArtFullSize() override = default;

    void init(TrackPointer pTrack);
    void mousePressEvent(QMouseEvent* /* unused */) override;
    void resizeEvent(QResizeEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

  public slots:
    void slotLoadTrack(TrackPointer);
    void slotCoverFound(const QObject* pRequestor,
            const CoverInfoRelative& info, QPixmap pixmap, bool fromCache);
    void slotTrackCoverArtUpdated();

    // slots that handle signals from WCoverArtMenu
    void slotCoverMenu(const QPoint& pos);
    void slotCoverInfoSelected(const CoverInfoRelative& coverInfo);
    void slotReloadCoverArt();

  private:
    QPixmap m_pixmap;
    TrackPointer m_pLoadedTrack;
    BaseTrackPlayer* m_pPlayer;
    parented_ptr<WCoverArtMenu> m_pCoverMenu;
};
