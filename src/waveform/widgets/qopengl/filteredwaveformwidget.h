#pragma once

#include "waveform/widgets/qopengl/waveformwidget.h"

class WaveformWidgetFactory;

namespace qopengl {
class FilteredWaveformWidget;
}

class qopengl::FilteredWaveformWidget : public qopengl::WaveformWidget {
    Q_OBJECT
  public:
    ~FilteredWaveformWidget() override;

    WaveformWidgetType::Type getType() const override {
        return WaveformWidgetType::QOpenGLFilteredWaveform;
    }

    static inline QString getWaveformWidgetName() {
        return tr("Filtered (QOpenGL)");
    }
    static inline bool useOpenGl() {
        return true;
    }
    static inline bool useOpenGles() {
        return false;
    }
    static inline bool useOpenGLShaders() {
        return true;
    }
    static inline bool developerOnly() {
        return false;
    }

  protected:
    void castToQWidget() override;
    void paintEvent(QPaintEvent* event) override;

  private:
    FilteredWaveformWidget(const QString& group, QWidget* parent);
    friend class ::WaveformWidgetFactory;
};
