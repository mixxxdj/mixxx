#ifndef GLRGBWAVEFORMWIDGET_H
#define GLRGBWAVEFORMWIDGET_H

#include "waveform/widgets/baseqopenglwidget.h"

class GLRGBWaveformWidget : public BaseQOpenGLWidget {
    Q_OBJECT
  public:
    GLRGBWaveformWidget(const char* group, QWidget* parent);
    virtual ~GLRGBWaveformWidget();

    virtual WaveformWidgetType::Type getType() const { return WaveformWidgetType::GLRGBWaveform; }

    static inline QString getWaveformWidgetName() { return tr("RGB"); }
    static inline bool useOpenGl() { return true; }
    static inline bool useOpenGLShaders() { return false; }
    static inline bool developerOnly() { return false; }

  protected:
    virtual void castToQWidget();
    virtual void paintEvent(QPaintEvent* event);

  protected slots:
    mixxx::Duration render() override;

  private:
    friend class WaveformWidgetFactory;
};

#endif // GLRGBWAVEFORMWIDGET_H
