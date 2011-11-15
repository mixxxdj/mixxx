#ifndef WAVEFORMRENDERERABSTRACT_H
#define WAVEFORMRENDERERABSTRACT_H

#include <QPainter>
#include <QPaintEvent>
#include <QDomNode>

class WaveformWidgetRenderer;

class WaveformRendererAbstract {
  public:
    WaveformRendererAbstract(WaveformWidgetRenderer* waveformWidgetRenderer);
    virtual ~WaveformRendererAbstract();

    virtual void init() = 0;
    virtual void setup(const QDomNode& node) = 0;
    virtual void draw(QPainter* painter, QPaintEvent* event) = 0;
    virtual void onResize() {
    }

  protected:
    bool isDirty() const {
        return m_dirty;
    }
    void setDirty(bool dirty = true) {
        m_dirty = dirty;
    }
    WaveformWidgetRenderer* m_waveformWidget;
    bool m_dirty;

    friend class WaveformWidgetRenderer;
};

#endif // WAVEFORMRENDERERABSTRACT_H
