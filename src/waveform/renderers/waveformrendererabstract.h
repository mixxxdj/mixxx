#pragma once

#include <QtGlobal>

QT_FORWARD_DECLARE_CLASS(QDomNode)
QT_FORWARD_DECLARE_CLASS(QPaintEvent)
QT_FORWARD_DECLARE_CLASS(QPainter)

namespace rendergraph {
class Node;
}

class SkinContext;
class WaveformWidgetRenderer;

class WaveformRendererAbstract {
  public:
    /// The type of cursor for which the waveform is rendered
    ///  Play: the play cursor is the area of focus for the renderer
    ///  Slip: the slip cursor is the area of focus for the renderer
    enum PositionSource {
        Play,
        Slip
    };
    explicit WaveformRendererAbstract(
            WaveformWidgetRenderer* waveformWidgetRenderer);
    virtual ~WaveformRendererAbstract();

    virtual bool init() {return true; }
    virtual void setup(const QDomNode& node, const SkinContext& context) = 0;
    virtual void draw(QPainter* painter, QPaintEvent* event) = 0;

    virtual void onResize() {}
    virtual void onSetTrack() {}
    virtual void update() {
    }

  protected:
    bool isDirty() const {
        return m_dirty;
    }
    void setDirty(bool dirty = true) {
        m_dirty = dirty;
    }

    double scaleFactor() const {
        return m_scaleFactor;
    }
    void setScaleFactor(double scaleFactor) {
        m_scaleFactor = scaleFactor;
    }

    WaveformWidgetRenderer* m_waveformRenderer;

  private:

    bool m_dirty;
    double m_scaleFactor;

    friend class WaveformWidgetRenderer;
};
