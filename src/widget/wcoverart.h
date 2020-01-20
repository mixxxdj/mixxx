#pragma once

#include <QColor>
#include <QDomNode>
#include <QMouseEvent>
#include <QWidget>
#include <QTimer>

#include "mixer/basetrackplayer.h"
#include "preferences/usersettings.h"
#include "library/coverartcache.h"
#include "skin/skincontext.h"
#include "widget/trackdroptarget.h"
#include "widget/wbasewidget.h"
#include "widget/wcoverartmenu.h"

class DlgCoverArtFullSize;

class WCoverArt : public QWidget, public WBaseWidget, public TrackDropTarget {
    Q_OBJECT
  public:
    WCoverArt(QWidget* parent, UserSettingsPointer pConfig,
              const QString& group, BaseTrackPlayer* pPlayer);
    ~WCoverArt() override;

    void setup(const QDomNode& node, const SkinContext& context);

  public slots:
    void slotLoadTrack(TrackPointer);
    void slotLoadingTrack(TrackPointer pNewTrack, TrackPointer pOldTrack);
    void slotReset();
    void slotEnable(bool);

  signals:
    void trackDropped(const QString& filename, const QString& group) override;
    void cloneDeck(const QString& sourceGroup, const QString& targetGroup) override;

  private slots:
    void slotCoverFound(
            const QObject* pRequestor,
            const CoverInfo& coverInfo,
            const QPixmap& pixmap,
            quint16 requestedHash,
            bool coverInfoUpdated);
    void slotCoverInfoSelected(const CoverInfoRelative& coverInfo);
    void slotReloadCoverArt();
    void slotTrackCoverArtUpdated();

  protected:
    void paintEvent(QPaintEvent* /*unused*/) override;
    void resizeEvent(QResizeEvent* /*unused*/) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;

    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    bool event(QEvent* pEvent) override;

  private:
    QPixmap scaledCoverArt(const QPixmap& normal);

    const QString m_group;
    UserSettingsPointer m_pConfig;
    bool m_bEnable;
    WCoverArtMenu* m_pMenu;
    TrackPointer m_loadedTrack;
    QPixmap m_loadedCover;
    QPixmap m_loadedCoverScaled;
    QPixmap m_defaultCover;
    QPixmap m_defaultCoverScaled;
    CoverInfo m_lastRequestedCover;
    BaseTrackPlayer* m_pPlayer;
    DlgCoverArtFullSize* m_pDlgFullSize;
    QTimer m_clickTimer;
};
