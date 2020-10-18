#pragma once

#include <QLabel>
#include <QMouseEvent>
#include <QPixmap>
#include <QWidget>

#include "track/track_decl.h"
#include "util/parented_ptr.h"

class WCoverArtMenu;
class DlgCoverArtFullSize;
class CoverInfo;
class CoverInfoRelative;

class WCoverArtLabel : public QLabel {
    Q_OBJECT
  public:
    explicit WCoverArtLabel(QWidget* parent = nullptr);
    ~WCoverArtLabel() override; // Verifies that the base destructor is virtual

    void setCoverArt(const CoverInfo& coverInfo, QPixmap px);
    void loadTrack(TrackPointer pTrack);

  signals:
    void coverInfoSelected(const CoverInfoRelative& coverInfo);
    void reloadCoverArt();

  protected:
    void mousePressEvent(QMouseEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;

  private slots:
      void slotCoverMenu(const QPoint& pos);

  private:
    const parented_ptr<WCoverArtMenu> m_pCoverMenu;
    const parented_ptr<DlgCoverArtFullSize> m_pDlgFullSize;

    const QPixmap m_defaultCover;

    TrackPointer m_pLoadedTrack;

    QPixmap m_loadedCover;
};
