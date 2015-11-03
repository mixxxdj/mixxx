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
    WKnobComposed(QWidget* pParent=NULL);
    virtual ~WKnobComposed();

    void setup(QDomNode node, const SkinContext& context);

  protected:
    void wheelEvent(QWheelEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void paintEvent(QPaintEvent*);

  private:
    void clear();
    void setPixmapBackground(const QString& filename, Paintable::DrawMode mode);
    void setPixmapKnob(const QString& filename);

    PaintablePointer m_pKnob;
    PaintablePointer m_pPixmapBack;
    KnobEventHandler<WKnobComposed> m_handler;
    double m_dMinAngle;
    double m_dMaxAngle;
    friend class KnobEventHandler<WKnobComposed>;
};

#endif /* WKNOBCOMPOSED_H */
