#ifndef WCOVERART_H
#define WCOVERART_H

#include <QColor>
#include <QDomNode>
#include <QMouseEvent>
#include <QWidget>

#include "configobject.h"
#include "dlgcoverartfullsize.h"
#include "trackinfoobject.h"
#include "library/coverartcache.h"
#include "skin/skincontext.h"
#include "widget/wbasewidget.h"
#include "widget/wcoverartmenu.h"

class WCoverArt : public QWidget, public WBaseWidget {
    Q_OBJECT
  public:
    WCoverArt(QWidget* parent, ConfigObject<ConfigValue>* pConfig,
              const QString& group);
    virtual ~WCoverArt();

    void setup(QDomNode node, const SkinContext& context);

  public slots:
    void slotLoadTrack(TrackPointer);
    void slotReset();
    void slotEnable(bool);

  signals:
    void trackDropped(QString filename, QString group);

  private slots:
    void slotCoverFound(const QObject* pRequestor, int requestReference,
                        const CoverInfo& info, QPixmap pixmap, bool fromCache);
    void slotCoverArtSelected(const CoverArt& art);
    void slotReloadCoverArt();
    void slotTrackCoverArtUpdated();

  protected:
    void paintEvent(QPaintEvent*);
    void resizeEvent(QResizeEvent*);
    void mousePressEvent(QMouseEvent*);
    void leaveEvent(QEvent*);

    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

  private:
    QPixmap scaledCoverArt(const QPixmap& normal);

    QString m_group;
    ConfigObject<ConfigValue>* m_pConfig;
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
