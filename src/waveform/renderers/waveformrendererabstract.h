#pragma once

#include <qglobal.h>

QT_FORWARD_DECLARE_CLASS(QDomNode)
QT_FORWARD_DECLARE_CLASS(QPaintEvent)
QT_FORWARD_DECLARE_CLASS(QPainter)

class SkinContext;
class WaveformWidgetRenderer;

#ifdef MIXXX_USE_QOPENGL
namespace qopengl {
class IWaveformRenderer;
}
#endif

class WaveformRendererAbstract {
  public:
    explicit WaveformRendererAbstract(
            WaveformWidgetRenderer* waveformWidgetRenderer);
    virtual ~WaveformRendererAbstract();

    virtual bool init() {return true; }
    virtual void setup(const QDomNode& node, const SkinContext& context) = 0;
    virtual void draw(QPainter* painter, QPaintEvent* event) = 0;

    virtual void onResize() {}
    virtual void onSetTrack() {}

#ifdef MIXXX_USE_QOPENGL
    virtual qopengl::IWaveformRenderer* qopenglWaveformRenderer() {
        return nullptr;
    }
#endif

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
