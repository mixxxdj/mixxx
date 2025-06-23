#pragma once

#include "waveform/widgets/nonglwaveformwidgetabstract.h"

class QWidget;

// This class can be used as a template file to create new WaveformWidgets it
// contain minimal set of method to re-implement

class EmptyWaveformWidget : public NonGLWaveformWidgetAbstract {
    Q_OBJECT
  public:
    virtual ~EmptyWaveformWidget();

    virtual WaveformWidgetType::Type getType() const { return WaveformWidgetType::EmptyWaveform; }

    static inline QString getWaveformWidgetName() { return tr("Empty"); }
    static inline bool useOpenGl() { return false; }
    static inline bool useOpenGles() { return false; }
    static inline bool useOpenGLShaders() { return false; }
    static inline bool useTextureForWaveform() {
        return false;
    }
    static inline WaveformWidgetCategory category() {
        return WaveformWidgetCategory::Default;
    }

  protected:
    virtual void castToQWidget();
    virtual void paintEvent(QPaintEvent* event);
    virtual mixxx::Duration render();

  private:
    EmptyWaveformWidget(const QString& group, QWidget* parent);
    friend class WaveformWidgetFactory;
};
