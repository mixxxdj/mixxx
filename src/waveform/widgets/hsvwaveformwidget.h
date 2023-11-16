#pragma once

#include "nonglwaveformwidgetabstract.h"

class QWidget;

class HSVWaveformWidget : public NonGLWaveformWidgetAbstract {
    Q_OBJECT
  public:
    virtual ~HSVWaveformWidget();

    virtual WaveformWidgetType::Type getType() const { return WaveformWidgetType::HSVWaveform; }

    static inline QString getWaveformWidgetName() { return tr("HSV"); }
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
    HSVWaveformWidget(const QString& group, QWidget* parent);
    friend class WaveformWidgetFactory;
};
