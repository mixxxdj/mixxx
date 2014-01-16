#ifndef WBATTERY_H
#define WBATTERY_H

#include <QList>
#include <QVector>
#include <QPixmap>
#include <QScopedPointer>

#include "widget/wlabel.h"
#include "widget/wwidget.h"
#include "skin/skincontext.h"
#include "widget/wpixmapstore.h"

class Battery;

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
    void setPixmap(PaintablePointer* ppPixmap, const QString& filename);
    void setPixmaps(QVector<PaintablePointer>* pPixmaps, int iPos,
                    const QString& filename);


    QScopedPointer<Battery> m_pBattery;
    PaintablePointer m_pCurrentPixmap;

    PaintablePointer m_pPixmapBack;
    PaintablePointer m_pPixmapUnknown;
    PaintablePointer m_pPixmapCharged;
    QVector<PaintablePointer> m_dischargingPixmaps;
    QVector<PaintablePointer> m_chargingPixmaps;
};

#endif /* WBATTERY_H */
