#ifndef DLGCOVERARTFULLSIZE_H
#define DLGCOVERARTFULLSIZE_H

#include <QDialog>
#include <QTimer>
#include <QPoint>

#include "library/ui_dlgcoverartfullsize.h"
#include "library/coverart.h"
#include "mixer/basetrackplayer.h"
#include "track/track.h"
#include "widget/wcoverartmenu.h"

class DlgCoverArtFullSize
        : public QDialog,
          public Ui::DlgCoverArtFullSize {
    Q_OBJECT
  public:
    DlgCoverArtFullSize(QWidget* parent = nullptr, BaseTrackPlayer* pPlayer = nullptr);
    virtual ~DlgCoverArtFullSize();

    void init(TrackPointer pTrack);
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* ) override;
    void mouseMoveEvent(QMouseEvent* ) override;
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
    WCoverArtMenu* m_pCoverMenu;
    QTimer m_clickTimer;
    QPoint m_dragStartPosition;
    bool m_coverPressed;
};

#endif // DLGCOVERARTFULLSIZE_H
