#pragma once

#include <QByteArray>
#include <QLabel>
#include <QPixmap>

#include "track/track_decl.h"
#include "util/parented_ptr.h"

class WCoverArtMenu;
class DlgCoverArtFullSize;
class CoverInfo;
class CoverInfoRelative;

class WCoverArtLabel : public QLabel {
    Q_OBJECT
  public:
    explicit WCoverArtLabel(QWidget* parent = nullptr, WCoverArtMenu* m_pWCoverArtMenu = nullptr);

    ~WCoverArtLabel() override; // Verifies that the base destructor is virtual

    void setCoverInfoAndPixmap(const CoverInfo& coverInfo, const QPixmap& px);
    void loadTrack(TrackPointer pTrack);
    void setMaxSize(const QSize size);

  protected:
    void mousePressEvent(QMouseEvent* pEvent) override;
    void contextMenuEvent(QContextMenuEvent* pEvent) override;

  private slots:
      void slotCoverMenu(const QPoint& pos);

  private:
    void setPixmapAndResize(const QPixmap& px);

    WCoverArtMenu* m_pCoverMenu;

    const parented_ptr<DlgCoverArtFullSize> m_pDlgFullSize;

    TrackPointer m_pLoadedTrack;

    QSize m_maxSize;
    QSize m_pixmapSizeMax;
    QPixmap m_defaultCover;
    QPixmap m_loadedCover;
    QPixmap m_fullSizeCover;
};
