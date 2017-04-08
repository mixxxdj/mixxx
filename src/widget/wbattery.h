#ifndef WBATTERY_H
#define WBATTERY_H

#include <QList>
#include <QPixmap>
#include <QScopedPointer>
#include <QVector>

#include "skin/skincontext.h"
#include "widget/wlabel.h"
#include "widget/wpixmapstore.h"
#include "widget/wwidget.h"
#include "util/battery/battery.h"

class WBattery : public WWidget {
    Q_OBJECT
  public:
    explicit WBattery(QWidget* parent=nullptr);

    void setup(const QDomNode& node, const SkinContext& context);

  public slots:
    // gets information from battery and updates the Pixmap
    void update();

  protected:
    void paintEvent(QPaintEvent * /*unused*/) override;

  private:
    void setPixmap(PaintablePointer* ppPixmap, const PixmapSource& source,
                   Paintable::DrawMode mode, double scaleFactor);

    QScopedPointer<Battery> m_pBattery;
    PaintablePointer m_pCurrentPixmap;

    PaintablePointer m_pPixmapBack;
    PaintablePointer m_pPixmapUnknown;
    PaintablePointer m_pPixmapCharged;
    QVector<PaintablePointer> m_dischargingPixmaps;
    QVector<PaintablePointer> m_chargingPixmaps;
};

#endif /* WBATTERY_H */
