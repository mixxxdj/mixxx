#ifndef WAVEFORMWIDGETABSTRACT_H
#define WAVEFORMWIDGETABSTRACT_H

#include <QWidget>
#include <QString>

#include "waveform/renderers/waveformwidgetrenderer.h"
#include "waveformwidgettype.h"
#include "trackinfoobject.h"

// NOTE(vRince) This class represent objects the waveformwidgetfactory can
// holds, IMPORTANT all WaveformWidgetAbstract MUST inherist QWidget too !!  we
// can't do it here because QWidget and QGLWidget are both QWidgets so they
// already have a common QWidget base class (ambigous polymorphism)

class WaveformWidgetAbstract : public WaveformWidgetRenderer {
  public:
    WaveformWidgetAbstract( const char* group);
    virtual ~WaveformWidgetAbstract();

    //Type is use by the factory to safely up-cast waveform widgets
    virtual WaveformWidgetType::Type getType() const = 0;

    bool isValid() const { return (m_widget && m_initSuccess); }
    QWidget* getWidget() { return m_widget; }

    void hold();
    void release();

    virtual void preRender();
    virtual void render();
    virtual void postRender() {}

    virtual void resize( int width, int height);

  protected:
    QWidget* m_widget;
    bool m_initSuccess;

    WaveformWidgetAbstract();
    //this is the factory resposability to trigger QWidget casting after constructor
    virtual void castToQWidget() = 0;

    friend class WaveformWidgetFactory;
};

#endif // WAVEFORMWIDGETABSTRACT_H
