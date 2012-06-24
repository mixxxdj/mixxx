#ifndef WAVEFORMRENDERMARK_H
#define WAVEFORMRENDERMARK_H

#include "waveform/renderers/waveformrendererabstract.h"
#include "waveformmarkset.h"

#include <QPixmap>
#include <vector>

#include "util.h"

class ControlObject;

class WaveformRenderMark : public WaveformRendererAbstract {
public:
    explicit WaveformRenderMark(WaveformWidgetRenderer* waveformWidgetRenderer);

    virtual void init();
    virtual void setup(const QDomNode& node);
    virtual void draw(QPainter* painter, QPaintEvent* event);

private:
    void generateMarkPixmap(WaveformMark& mark);

    WaveformMarkSet m_marks;
    DISALLOW_COPY_AND_ASSIGN(WaveformRenderMark);
};

#endif
