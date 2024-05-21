#pragma once

#include "glwaveformwidgetabstract.h"

class QtSimpleWaveformWidget : public GLWaveformWidgetAbstract {
    Q_OBJECT
  public:
    QtSimpleWaveformWidget(const QString& group, QWidget* parent);
    virtual ~QtSimpleWaveformWidget();

    virtual WaveformWidgetType::Type getType() const {
        return WaveformWidgetType::Simple;
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
    friend class WaveformWidgetFactory;
};
