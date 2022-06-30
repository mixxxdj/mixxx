#pragma once

#include <QPixmap>
#include <QString>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QWheelEvent>

#include "util/widgetrendertimer.h"
#include "widget/wdisplay.h"
#include "widget/knobeventhandler.h"

// This is used for knobs if the knob value is displayed by
// one of e.g. 64 pixmaps.
// If the knob value can be displayed by rotating a single
// SVG, use WKnobComposed.
class WKnob : public WDisplay {
   Q_OBJECT
  public:
    explicit WKnob(QWidget* pParent=nullptr);

  protected:
    void wheelEvent(QWheelEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseDoubleClickEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
    void inputActivity();

  private:
    WidgetRenderTimer m_renderTimer;

    KnobEventHandler<WKnob> m_handler;
    friend class KnobEventHandler<WKnob>;
};
