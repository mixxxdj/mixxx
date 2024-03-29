#pragma once

#include <QScopedPointer>
#include <QVector>

#include "widget/paintable.h"
#include "widget/wpixmapstore.h"
#include "widget/wwidget.h"

class SkinContext;
class QDomNode;
class PixmapSource;
class Battery;

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
