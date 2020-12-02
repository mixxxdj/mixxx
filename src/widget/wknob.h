#ifndef WKNOB_H
#define WKNOB_H

#include <QByteArrayData>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPixmap>
#include <QString>
#include <QWheelEvent>

#include "util/widgetrendertimer.h"
#include "widget/knobeventhandler.h"
#include "widget/wdisplay.h"

class QMouseEvent;
class QObject;
class QWheelEvent;
class QWidget;

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
    void mouseReleaseEvent(QMouseEvent *e) override;
    void inputActivity();

  private:
    WidgetRenderTimer m_renderTimer;

    KnobEventHandler<WKnob> m_handler;
    friend class KnobEventHandler<WKnob>;
};

#endif
