#pragma once

#include "glwaveformwidgetabstract.h"

class QtRGBWaveformWidget : public GLWaveformWidgetAbstract {
    Q_OBJECT
  public:
    virtual ~QtRGBWaveformWidget();

    virtual WaveformWidgetType::Type getType() const {
        return WaveformWidgetType::RGB;
    }
    static inline bool useOpenGl() { return true; }
    static inline bool useOpenGles() { return true; }
    static inline bool useOpenGLShaders() { return false; }
    static inline WaveformWidgetCategory category() {
        return WaveformWidgetCategory::Legacy;
    }

  protected:
    virtual void castToQWidget();
    virtual void paintEvent(QPaintEvent* event);
    virtual mixxx::Duration render();

  private:
    QtRGBWaveformWidget(const QString& group, QWidget* parent);
    friend class WaveformWidgetFactory;
};
