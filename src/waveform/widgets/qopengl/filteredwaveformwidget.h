#pragma once

#include "util/class.h"
#include "waveform/widgets/qopengl/waveformwidget.h"

class WaveformWidgetFactory;

namespace qopengl {
class FilteredWaveformWidget;
}

class qopengl::FilteredWaveformWidget final : public qopengl::WaveformWidget {
    Q_OBJECT
  public:
    ~FilteredWaveformWidget() override;

    WaveformWidgetType::Type getType() const override {
        return WaveformWidgetType::QOpenGLFilteredWaveform;
    }

    static inline QString getWaveformWidgetName() {
        return tr("Filtered (all-shaders)");
    }
    static constexpr bool useOpenGl() {
        return true;
    }
    static constexpr bool useOpenGles() {
        return false;
    }
    static constexpr bool useOpenGLShaders() {
        return true;
    }
    static constexpr bool developerOnly() {
        return false;
    }

  protected:
    void castToQWidget() override;
    void paintEvent(QPaintEvent* event) override;

  private:
    FilteredWaveformWidget(const QString& group, QWidget* parent);
    friend class ::WaveformWidgetFactory;

    DISALLOW_COPY_AND_ASSIGN(FilteredWaveformWidget);
};
