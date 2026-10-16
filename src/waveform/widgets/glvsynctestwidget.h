#pragma once

#include "waveform/widgets/glwaveformwidgetabstract.h"

class GLVSyncTestWidget : public GLWaveformWidgetAbstract {
    Q_OBJECT
  public:
    GLVSyncTestWidget(const QString& group, QWidget* pParent);
    virtual ~GLVSyncTestWidget();

    WaveformWidgetType::Type getType() const override {
        return WaveformWidgetType::VSyncTest;
    }

    static inline bool useOpenGl() { return true; }
    static inline bool useOpenGles() { return false; }
    static inline bool useOpenGLShaders() { return false; }
    static inline bool useTextureForWaveform() {
        return false;
    }
    static inline WaveformWidgetCategory category() {
        return WaveformWidgetCategory::DeveloperOnly;
    }

  protected:
    void castToQWidget() override;
    void paintEvent(QPaintEvent* event) override;
    mixxx::Duration render() override;

  private:
    friend class WaveformWidgetFactory;
};
