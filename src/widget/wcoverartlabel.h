#pragma once

#include <QLabel>
#include <QMouseEvent>
#include <QWidget>
#include <QPixmap>
#include <QPointer>

#include "track/track.h"
#include "widget/wcoverartmenu.h"

class DlgCoverArtFullSize;

class WCoverArtLabel : public QLabel {
    Q_OBJECT
  public:
    explicit WCoverArtLabel(QWidget* parent = nullptr);
    ~WCoverArtLabel() override = default;

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
    QPointer<WCoverArtMenu> m_pCoverMenu;
    QPointer<DlgCoverArtFullSize> m_pDlgFullSize;
    QPixmap m_defaultCover;
};
