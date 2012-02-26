#ifndef WAVEFORMRENDERMARK_H
#define WAVEFORMRENDERMARK_H

#include <QPixmap>
#include <QVector>

#include "util.h"
#include "waveform/renderers/waveformrendererabstract.h"

class ControlObject;

class Mark {
  public:
    Mark() : m_point(0) {}
    virtual ~Mark() {}

  private:
    ControlObject* m_point;
    QColor m_color;
    QColor m_textColor;
    QString m_text;
    Qt::Alignment m_align;
    QString m_pixmapPath;
    QPixmap m_pixmap;

    friend class WaveformRenderMark;
};

class WaveformRenderMark : public WaveformRendererAbstract {
  public:
    explicit WaveformRenderMark(WaveformWidgetRenderer* waveformWidgetRenderer);

    virtual void init();
    virtual void setup(const QDomNode& node);
    virtual void draw(QPainter* painter, QPaintEvent* event);

  private:
    void setupMark(const QDomNode& node, Mark& mark);
    void generateMarkPixmap(Mark& mark);

    QVector<Mark> m_marks;
    DISALLOW_COPY_AND_ASSIGN(WaveformRenderMark);
};

#endif
