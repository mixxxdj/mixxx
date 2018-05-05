#ifndef WCOVERART_H
#define WCOVERART_H

#include <QColor>
#include <QDomNode>
#include <QMouseEvent>
#include <QWidget>

#include "preferences/usersettings.h"
#include "track/track.h"
#include "library/coverartcache.h"
#include "skin/skincontext.h"
#include "widget/wbasewidget.h"
#include "widget/wcoverartmenu.h"

class DlgCoverArtFullSize;

class WCoverArt : public QWidget, public WBaseWidget {
    Q_OBJECT
  public:
    WCoverArt(QWidget* parent, UserSettingsPointer pConfig,
              const QString& group);
    ~WCoverArt() override;

    void setup(const QDomNode& node, const SkinContext& context);

  public slots:
    void slotLoadTrack(TrackPointer);
    void slotLoadingTrack(TrackPointer pNewTrack, TrackPointer pOldTrack);
    void slotReset();
    void slotEnable(bool);

  signals:
    void trackDropped(QString filename, QString group);

  private slots:
    void slotCoverFound(const QObject* pRequestor,
                        const CoverInfo& info, QPixmap pixmap, bool fromCache);
    void slotCoverInfoSelected(const CoverInfo& coverInfo);
    void slotReloadCoverArt();
    void slotTrackCoverArtUpdated();

  protected:
    void paintEvent(QPaintEvent* /*unused*/) override;
    void resizeEvent(QResizeEvent* /*unused*/) override;
    void mousePressEvent(QMouseEvent* /*unused*/) override;
    void leaveEvent(QEvent* /*unused*/) override;

    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

  private:
    QPixmap scaledCoverArt(const QPixmap& normal);

    QString m_group;
    UserSettingsPointer m_pConfig;
    bool m_bEnable;
    WCoverArtMenu* m_pMenu;
    TrackPointer m_loadedTrack;
    QPixmap m_loadedCover;
    QPixmap m_loadedCoverScaled;
    QPixmap m_defaultCover;
    QPixmap m_defaultCoverScaled;
    CoverInfo m_lastRequestedCover;
    DlgCoverArtFullSize* m_pDlgFullSize;
};

#endif // WCOVERART_H
