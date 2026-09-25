#pragma once

#include "nonglwaveformwidgetabstract.h"

class QWidget;

class SimpleSignalWaveformWidget : public NonGLWaveformWidgetAbstract {
    Q_OBJECT
  public:
    virtual ~SimpleSignalWaveformWidget();

    virtual WaveformWidgetType::Type getType() const {
        return WaveformWidgetType::RGB;
    }

    static inline bool useOpenGl() {
        return false;
    }
    static inline bool useOpenGles() {
        return false;
    }
    static inline bool useOpenGLShaders() {
        return false;
    }
    static inline bool useTextureForWaveform() {
        return false;
    }
    static inline WaveformWidgetCategory category() {
        return WaveformWidgetCategory::Software;
    }

  protected:
    virtual void castToQWidget();
    virtual void paintEvent(QPaintEvent* event);

  private:
    SimpleSignalWaveformWidget(const QString& group, QWidget* parent);
    friend class WaveformWidgetFactory;
};
