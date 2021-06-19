#pragma once

#include <QWidget>

#include "waveform/widgets/waveformwidgetabstract.h"

// This class can be used as a template file to create new WaveformWidgets it
// contain minimal set of method to re-implement

class EmptyWaveformWidget : public QWidget, public WaveformWidgetAbstract {
    Q_OBJECT
  public:
    virtual ~EmptyWaveformWidget();

    virtual WaveformWidgetType::Type getType() const { return WaveformWidgetType::EmptyWaveform; }

    static inline QString getWaveformWidgetName() { return tr("Empty"); }
    static inline bool useOpenGl() { return false; }
    static inline bool useOpenGles() { return false; }
    static inline bool useOpenGLShaders() { return false; }
    static inline bool developerOnly() { return false; }

  protected:
    virtual void castToQWidget();
    virtual void paintEvent(QPaintEvent* event);
    virtual mixxx::Duration render();

  private:
    EmptyWaveformWidget(const QString& group, QWidget* parent);
    friend class WaveformWidgetFactory;
};
