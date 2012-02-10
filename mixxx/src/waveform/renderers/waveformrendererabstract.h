#ifndef WAVEFORMRENDERERABSTRACT_H
#define WAVEFORMRENDERERABSTRACT_H

class QDomNode;
class QPaintEvent;
class QPainter;

class WaveformWidgetRenderer;

class WaveformRendererAbstract {
  public:
    explicit WaveformRendererAbstract(WaveformWidgetRenderer* waveformWidgetRenderer);
    virtual ~WaveformRendererAbstract();

    virtual void init() = 0;
    virtual void setup(const QDomNode& node) = 0;
    virtual void draw(QPainter* painter, QPaintEvent* event) = 0;

    virtual void onResize() {}
    virtual void onSetTrack() {}

  protected:
    bool isDirty() const {
        return m_dirty;
    }
    void setDirty(bool dirty = true) {
        m_dirty = dirty;
    }
    WaveformWidgetRenderer* m_waveformRenderer;
    bool m_dirty;

    friend class WaveformWidgetRenderer;
};

#endif // WAVEFORMRENDERERABSTRACT_H
