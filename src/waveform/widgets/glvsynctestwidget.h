#pragma once

#include "waveform/widgets/glwaveformwidgetabstract.h"

class GLVSyncTestWidget : public GLWaveformWidgetAbstract {
    Q_OBJECT
  public:
    GLVSyncTestWidget(const QString& group, QWidget* parent);
    virtual ~GLVSyncTestWidget();

    virtual WaveformWidgetType::Type getType() const { return WaveformWidgetType::GLVSyncTest; }

    static inline QString getWaveformWidgetName() { return tr("VSyncTest"); }
    static inline bool useOpenGl() { return true; }
    static inline bool useOpenGles() { return false; }
    static inline bool useOpenGLShaders() { return false; }
    static inline bool developerOnly() { return true; }

  protected:
    virtual void castToQWidget();
    virtual void paintEvent(QPaintEvent* event);
    virtual mixxx::Duration render();

  private:
    friend class WaveformWidgetFactory;
};
