#pragma once

#include "glwaveformwidgetabstract.h"

class QtVSyncTestWidget : public GLWaveformWidgetAbstract {
    Q_OBJECT
  public:
    QtVSyncTestWidget(const QString& group, QWidget* parent);
    virtual ~QtVSyncTestWidget();

    virtual WaveformWidgetType::Type getType() const { return WaveformWidgetType::QtVSyncTest; }

    static inline QString getWaveformWidgetName() { return tr("VSyncTest") + " - Qt"; }
    static inline bool useOpenGl() { return true; }
    static inline bool useOpenGles() { return true; }
    static inline bool useOpenGLShaders() { return false; }
    static inline bool useTextureForWaveform() {
        return false;
    }
    static inline WaveformWidgetCategory category() {
        return WaveformWidgetCategory::DeveloperOnly;
    }

  protected:
    virtual void castToQWidget();
    virtual void paintEvent(QPaintEvent* event);
    virtual mixxx::Duration render();

  private:
    friend class WaveformWidgetFactory;
};
