#pragma once

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
    void leaveEvent(QEvent* e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseDoubleClickEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
    void inputActivity();

  private:
    KnobEventHandler<WKnob> m_handler;
    friend class KnobEventHandler<WKnob>;
};
