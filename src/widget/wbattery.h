#ifndef WBATTERY_H
#define WBATTERY_H

#include <QByteArrayData>
#include <QDomNode>
#include <QList>
#include <QPixmap>
#include <QScopedPointer>
#include <QString>
#include <QVector>

#include "skin/pixmapsource.h"
#include "skin/skincontext.h"
#include "util/battery/battery.h"
#include "widget/paintable.h"
#include "widget/wlabel.h"
#include "widget/wpixmapstore.h"
#include "widget/wwidget.h"

class QObject;
class QPaintEvent;
class QWidget;

class WBattery : public WWidget {
    Q_OBJECT
  public:
    explicit WBattery(QWidget* parent = nullptr);
    ~WBattery() override = default;

    void setup(const QDomNode& node, const SkinContext& context);

    static QString formatTooltip(double dPercentage);

  private slots:
    // gets information from battery and updates the Pixmap
    void slotStateChanged();

  protected:
    void paintEvent(QPaintEvent * /*unused*/) override;

  private:
    void setPixmap(PaintablePointer* ppPixmap, const PixmapSource& source,
                   Paintable::DrawMode mode, double scaleFactor);

    QScopedPointer<Battery> m_pBattery;
    PaintablePointer m_pCurrentPixmap;

    PaintablePointer m_pPixmapBack;
    PaintablePointer m_pPixmapCharged;
    QVector<PaintablePointer> m_dischargingPixmaps;
    QVector<PaintablePointer> m_chargingPixmaps;
};

#endif /* WBATTERY_H */
