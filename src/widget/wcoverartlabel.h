#pragma once

#include <QByteArray>
#include <QLabel>
#include <QMouseEvent>
#include <QPixmap>
#include <QWidget>

#include "library/coverart.h"
#include "track/track_decl.h"
#include "util/parented_ptr.h"

class WCoverArtMenu;
class DlgCoverArtFullSize;
class CoverInfo;
class CoverInfoRelative;

/// This widget is for the Track Properties dialog.
/// It can have a context menu (in DlgTrackfetcher it doesn't), and compared to
/// WCoverArt it's updated by its parent, not by track change signals.
class WCoverArtLabel : public QLabel {
    Q_OBJECT
  public:
    explicit WCoverArtLabel(QWidget* pParent = nullptr, WCoverArtMenu* m_pWCoverArtMenu = nullptr);

    ~WCoverArtLabel() override; // Verifies that the base destructor is virtual

    void setCoverArt(const CoverInfo& coverInfo, const QPixmap& px);
    void loadTrack(TrackPointer pTrack);
    void loadData(const QByteArray& data);

  protected:
    void mousePressEvent(QMouseEvent* event) override;

  private slots:
      void slotCoverMenu(const QPoint& pos);

  private:
    WCoverArtMenu* m_pCoverMenu;

    const parented_ptr<DlgCoverArtFullSize> m_pDlgFullSize;

    const QPixmap m_defaultCover;

    QByteArray m_Data;

    TrackPointer m_pLoadedTrack;

    QPixmap m_loadedCover;

    CoverInfo m_coverInfo;
};
