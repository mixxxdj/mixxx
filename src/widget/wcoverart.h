#ifndef WCOVERART_H
#define WCOVERART_H

#include <QColor>
#include <QDomNode>
#include <QMenu>
#include <QMouseEvent>
#include <QWidget>

#include "configobject.h"
#include "skin/skincontext.h"
#include "trackinfoobject.h"
#include "widget/wbasewidget.h"
#include "widget/wcoverartmenu.h"

class WCoverArt : public QWidget, public WBaseWidget {
    Q_OBJECT
  public:
    WCoverArt(QWidget* parent, ConfigObject<ConfigValue>* pConfig);
    virtual ~WCoverArt();

    void setup(QDomNode node, const SkinContext& context);

  public slots:
    void slotResetWidget();
    void slotEnableWidget(bool);
    void slotLoadCoverArt(const QString& coverLocation,
                          const QString& md5Hash,
                          int trackId);

  private slots:
    void slotPixmapFound(int trackId, QPixmap pixmap);

  protected:
    void paintEvent(QPaintEvent*);
    void resizeEvent(QResizeEvent*);
    void mousePressEvent(QMouseEvent*);
    void mouseMoveEvent(QMouseEvent*);
    void leaveEvent(QEvent*);

  private:
    QPixmap scaledCoverArt(QPixmap normal);
    void setToDefault();

    ConfigObject<ConfigValue>* m_pConfig;

    bool m_bEnableWidget;
    bool m_bCoverIsHovered;
    bool m_bCoverIsVisible;
    bool m_bDefaultCover;

    WCoverArtMenu* m_pMenu;

    QString m_sCoverTitle;
    QPixmap m_currentCover;
    QPixmap m_currentScaledCover;

    QPixmap m_iconHide;
    QPixmap m_iconShow;

    int m_lastRequestedTrackId;
    QString m_lastRequestedCoverLocation;
    QString m_lastRequestedMd5Hash;
};

#endif // WCOVERART_H
