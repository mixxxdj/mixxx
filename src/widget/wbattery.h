#ifndef WBATTERY_H
#define WBATTERY_H

#include <QList>
#include <QMap>
#include <QPixmap>
#include <QScopedPointer>

#include "battery/battery.h"
#include "widget/wlabel.h"
#include "widget/wwidget.h"
#include "skin/skincontext.h"
#include "widget/wpixmapstore.h"

class WBattery : public WWidget {
    Q_OBJECT
  public:
    WBattery(QWidget* parent=NULL);
    virtual ~WBattery();

    void setup(QDomNode node, const SkinContext& context);

  public slots:
    // gets information from battery and updates the Pixmap
    void update();

  protected:
    void paintEvent(QPaintEvent *);

  private:
    // returns a formatted time string
    QString getTimeLeft();

    // sets all pixmaps in target for 0 <= i <= 100
    void setPixmap(QMap<int, PaintablePointer>* target,
                   const QString& filename, const QList<int>& charge);

    QScopedPointer<Battery> m_pBattery;
    PaintablePointer m_pPixmap;
    PaintablePointer m_pPixmapCharged;
    QMap<int, PaintablePointer> m_qmPixmapsCharging;
    QMap<int, PaintablePointer> m_qmPixmapsDischarging;
};

#endif /* WBATTERY_H */
