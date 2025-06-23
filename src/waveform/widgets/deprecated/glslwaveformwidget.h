#pragma once

#include "waveform/widgets/deprecated/glwaveformwidgetabstract.h"

class GLSLWaveformWidget : public GLWaveformWidgetAbstract {
    Q_OBJECT
  public:
    enum class GlslType {
        Filtered,
        RGB,
        RGBStacked,
    };
    GLSLWaveformWidget(
            const QString& group,
            QWidget* parent,
            GlslType type);
    ~GLSLWaveformWidget() override = default;

    void resizeRenderer(int width, int height, float devicePixelRatio) override;

  protected:
    void castToQWidget() override;
    void paintEvent(QPaintEvent* event) override;
    mixxx::Duration render() override;

  private:
    friend class WaveformWidgetFactory;
};

class GLSLFilteredWaveformWidget : public GLSLWaveformWidget {
    Q_OBJECT
  public:
    GLSLFilteredWaveformWidget(const QString& group, QWidget* parent);
    ~GLSLFilteredWaveformWidget() override = default;

    WaveformWidgetType::Type getType() const override { return WaveformWidgetType::GLSLFilteredWaveform; }

    static inline QString getWaveformWidgetName() { return tr("Filtered"); }
    static inline bool useOpenGl() { return true; }
    static inline bool useOpenGles() { return false; }
    static inline bool useOpenGLShaders() { return true; }
    static inline bool useTextureForWaveform() {
        return true;
    }
    static inline WaveformWidgetCategory category() {
        return WaveformWidgetCategory::Legacy;
    }
};

class GLSLRGBWaveformWidget : public GLSLWaveformWidget {
    Q_OBJECT
  public:
    GLSLRGBWaveformWidget(const QString& group, QWidget* parent);
    ~GLSLRGBWaveformWidget() override = default;

    WaveformWidgetType::Type getType() const override { return WaveformWidgetType::GLSLRGBWaveform; }

    static inline QString getWaveformWidgetName() { return tr("RGB"); }
    static inline bool useOpenGl() { return true; }
    static inline bool useOpenGles() { return false; }
    static inline bool useOpenGLShaders() { return true; }
    static inline bool useTextureForWaveform() {
        return true;
    }
    static inline WaveformWidgetCategory category() {
        return WaveformWidgetCategory::Legacy;
    }
};

class GLSLRGBStackedWaveformWidget : public GLSLWaveformWidget {
    Q_OBJECT
  public:
    GLSLRGBStackedWaveformWidget(const QString& group, QWidget* parent);
    ~GLSLRGBStackedWaveformWidget() override = default;

    WaveformWidgetType::Type getType() const override {
        return WaveformWidgetType::GLSLRGBStackedWaveform;
    }

    static inline QString getWaveformWidgetName() {
        return tr("RGB Stacked");
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
    static inline bool useTextureForWaveform() {
        return true;
    }
    static inline WaveformWidgetCategory category() {
        return WaveformWidgetCategory::Legacy;
    }
};
