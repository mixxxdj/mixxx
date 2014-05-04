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

class WCoverArt : public QWidget, public WBaseWidget {
    Q_OBJECT
  public:
    WCoverArt(QWidget* parent, ConfigObject<ConfigValue>* pConfig);
    virtual ~WCoverArt();

    void setup(QDomNode node, const SkinContext& context);

  public slots:
    void slotHideCoverArt();
    void slotLoadCoverArt(TrackPointer pTrack);

  protected:
    void paintEvent(QPaintEvent*);
    void resizeEvent(QResizeEvent*);
    void mousePressEvent(QMouseEvent*);
    void mouseMoveEvent(QMouseEvent*);
    void leaveEvent(QEvent*);

  private:
    ConfigObject<ConfigValue>* m_pConfig;

    bool m_bCoverIsHovered;
    bool m_bCoverIsVisible;

    const QString m_sDefaultCover;
    QString m_sCurrentCover;

    QImage m_coverArt;
};

#endif // WCOVERART_H
