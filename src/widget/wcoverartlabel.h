#pragma once

#include <QLabel>
#include <QMouseEvent>
#include <QWidget>
#include <QPixmap>

#include "track/track.h"
#include "widget/wcoverartmenu.h"
#include "util/parented_ptr.h"

class DlgCoverArtFullSize;

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

  private slots:
      void slotCoverMenu(const QPoint& pos);

  private:
    QPixmap m_loadedCover;
    TrackPointer m_pLoadedTrack;
    parented_ptr<WCoverArtMenu> m_pCoverMenu;
    parented_ptr<DlgCoverArtFullSize> m_pDlgFullSize;
    QPixmap m_defaultCover;
};
