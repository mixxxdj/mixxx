#ifndef WCOVERART_H
#define WCOVERART_H

#include <QColor>
#include <QDomNode>
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
                          int trackId, bool cachedOnly);

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

    ConfigObject<ConfigValue>* m_pConfig;

    bool m_bEnableWidget;
    bool m_bCoverIsHovered;
    bool m_bCoverIsVisible;

    WCoverArtMenu* m_pMenu;

    QPixmap m_loadedCover;

    QPixmap m_iconHide;
    QPixmap m_iconShow;

    int m_lastRequestedTrackId;
    QPair<QString, QString> m_lastRequestedCover;
};

#endif // WCOVERART_H
