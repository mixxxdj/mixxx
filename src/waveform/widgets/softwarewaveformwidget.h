#pragma once

#include "nonglwaveformwidgetabstract.h"
#include "waveform/renderers/waveformrenderersignalbase.h"

class QWidget;

class SoftwareWaveformWidget : public NonGLWaveformWidgetAbstract {
    Q_OBJECT
  public:
    virtual ~SoftwareWaveformWidget();

    virtual WaveformWidgetType::Type getType() const {
        return WaveformWidgetType::Filtered;
    }

    static inline QString getWaveformWidgetName() { return tr("Filtered"); }
    static inline bool useOpenGl() { return false; }
    static inline bool useOpenGles() { return false; }
    static inline bool useOpenGLShaders() { return false; }
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
    SoftwareWaveformWidget(const QString& groupp,
            QWidget* parent,
            WaveformRendererSignalBase::Options options);
    friend class WaveformWidgetFactory;
};
