
#ifndef WAVEFORMRENDERMARK_H
#define WAVEFORMRENDERMARK_H

#include <Qt>
#include <QPixmap>
#include <QVector>

#include "waveformrendererabstract.h"

class ControlObject;

class Mark
{
public:
    Mark() : m_point(0) {}

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
    WaveformRenderMark( WaveformWidgetRenderer* waveformWidgetRenderer);

    virtual void init();
    virtual void setup( const QDomNode& node);
    virtual void draw( QPainter* painter, QPaintEvent* event);

private:

    void setupMark( const QDomNode& node, Mark& mark);
    void generateMarkPixmap( Mark& mark);

    QVector<Mark> m_marks;
};

#endif
