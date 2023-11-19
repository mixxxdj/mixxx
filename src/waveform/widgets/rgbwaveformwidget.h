#pragma once

#include "nonglwaveformwidgetabstract.h"

class QWidget;

class RGBWaveformWidget : public NonGLWaveformWidgetAbstract {
    Q_OBJECT
  public:
    virtual ~RGBWaveformWidget();

    virtual WaveformWidgetType::Type getType() const { return WaveformWidgetType::RGBWaveform; }

    static inline QString getWaveformWidgetName() { return tr("RGB"); }
    static inline bool useOpenGl() { return false; }
    static inline bool useOpenGles() { return false; }
    static inline bool useOpenGLShaders() { return false; }
    static inline WaveformWidgetCategory category() {
        return WaveformWidgetCategory::Software;
    }

  protected:
    virtual void castToQWidget();
    virtual void paintEvent(QPaintEvent* event);

  private:
    RGBWaveformWidget(const QString& group, QWidget* parent);
    friend class WaveformWidgetFactory;
};
