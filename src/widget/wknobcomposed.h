#ifndef WKNOBCOMPOSED_H
#define WKNOBCOMPOSED_H

#include <QWidget>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QWheelEvent>

#include "widget/wwidget.h"
#include "widget/knobeventhandler.h"
#include "widget/wpixmapstore.h"
#include "widget/wimagestore.h"
#include "skin/skincontext.h"

class WKnobComposed : public WWidget {
    Q_OBJECT
  public:
    explicit WKnobComposed(QWidget* pParent=nullptr);

    void setup(const QDomNode& node, const SkinContext& context);

    void onConnectedControlChanged(double dParameter, double dValue) override;

  protected:
    void wheelEvent(QWheelEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void paintEvent(QPaintEvent* /*unused*/) override;

  private:
    void clear();
    void setPixmapBackground(PixmapSource source, Paintable::DrawMode mode);
    void setPixmapKnob(PixmapSource source, Paintable::DrawMode mode);

    double m_dCurrentAngle;
    PaintablePointer m_pKnob;
    PaintablePointer m_pPixmapBack;
    KnobEventHandler<WKnobComposed> m_handler;
    double m_dMinAngle;
    double m_dMaxAngle;
    double m_dKnobCenterXOffset;
    double m_dKnobCenterYOffset;
    friend class KnobEventHandler<WKnobComposed>;
};

#endif /* WKNOBCOMPOSED_H */
