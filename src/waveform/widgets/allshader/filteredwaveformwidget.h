#pragma once

#include "util/class.h"
#include "waveform/widgets/allshader/waveformwidget.h"

class WaveformWidgetFactory;

namespace allshader {
class FilteredWaveformWidget;
}

class allshader::FilteredWaveformWidget final : public allshader::WaveformWidget {
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
