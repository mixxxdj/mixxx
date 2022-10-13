#pragma once

#include <QString>
#include <QWidget>

#include "util/duration.h"
#include "waveform/renderers/waveformwidgetrenderer.h"
#include "waveformwidgettype.h"
#include "widget/wglwidget.h"

#ifdef MIXXX_USE_QOPENGL
namespace qopengl {
class IWaveformWidget;
}
#endif

class VSyncThread;

// NOTE(vRince) This class represent objects the waveformwidgetfactory can
// holds, IMPORTANT all WaveformWidgetAbstract MUST inherist QWidget too !!  we
// can't do it here because QWidget and WGLWidget are both QWidgets so they
// already have a common QWidget base class (ambiguous polymorphism)

class WaveformWidgetAbstract : public WaveformWidgetRenderer {
  public:
    WaveformWidgetAbstract(const QString& group);
    virtual ~WaveformWidgetAbstract();

    //Type is use by the factory to safely up-cast waveform widgets
    virtual WaveformWidgetType::Type getType() const = 0;

    bool isValid() const { return (m_widget && m_initSuccess); }
    QWidget* getWidget() { return m_widget; }

    virtual WGLWidget* getGLWidget() {
        return nullptr;
    }

#ifdef MIXXX_USE_QOPENGL
    // Derived classes that implement the IWaveformWidget
    // interface should return this
    virtual qopengl::IWaveformWidget* qopenglWaveformWidget() {
        return nullptr;
    }
#endif

    void hold();
    void release();

    virtual void preRender(VSyncThread* vsyncThread);
    virtual mixxx::Duration render();
    virtual void resize(int width, int height);

  protected:
    QWidget* m_widget;
    bool m_initSuccess;

    //this is the factory resposability to trigger QWidget casting after constructor
    virtual void castToQWidget() = 0;

    friend class WaveformWidgetFactory;
};
