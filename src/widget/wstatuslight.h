#pragma once

#include <QPaintEvent>
#include <QWidget>
#include <QString>
#include <QDomNode>
#include <QPixmap>
#include <QVector>

#include "widget/wwidget.h"
#include "widget/wpixmapstore.h"
#include "skin/legacy/skincontext.h"

/// A general purpose status light for indicating boolean events.
class WStatusLight : public WWidget  {
   Q_OBJECT
  public:
    explicit WStatusLight(QWidget *parent=nullptr);

    void setup(const QDomNode& node, const SkinContext& context);

  public slots:
    void onConnectedControlChanged(double dParameter, double dValue) override;

  protected:
    void paintEvent(QPaintEvent * /*unused*/) override;

  private:
    void setPixmap(int iState,
            const PixmapSource& source,
            Paintable::DrawMode mode,
            double scaleFactor);
    void setNoPos(int iNoPos);

    // Current position
    int m_iPos;

    PaintablePointer m_pPixmapBackground;

    // Associated pixmaps
    QVector<PaintablePointer> m_pixmaps;
};
