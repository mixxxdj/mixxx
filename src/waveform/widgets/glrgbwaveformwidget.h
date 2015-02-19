#ifndef GLRGBWAVEFORMWIDGET_H
#define GLRGBWAVEFORMWIDGET_H

#include <QGLWidget>

#include "waveformwidgetabstract.h"

class GLRGBWaveformWidget : public QGLWidget, public WaveformWidgetAbstract {
    Q_OBJECT
  public:
    GLRGBWaveformWidget(const StringAtom& group, QWidget* parent);
    virtual ~GLRGBWaveformWidget();

    virtual WaveformWidgetType::Type getType() const { return WaveformWidgetType::GLRGBWaveform; }

    static inline QString getWaveformWidgetName() { return tr("RGB"); }
    static inline bool useOpenGl() { return true; }
    static inline bool useOpenGLShaders() { return false; }
    static inline bool developerOnly() { return false; }

  protected:
    virtual void castToQWidget();
    virtual void paintEvent(QPaintEvent* event);
    virtual int render();

  private:
    friend class WaveformWidgetFactory;
};

#endif // GLRGBWAVEFORMWIDGET_H
