#ifndef GLVSYNCTESTWIDGET_H
#define GLVSYNCTESTWIDGET_H

#include "waveform/widgets/baseqopenglwidget.h"

class GLVSyncTestWidget : public BaseQOpenGLWidget {
    Q_OBJECT
  public:
    GLVSyncTestWidget(const char* group, QWidget* parent);
    virtual ~GLVSyncTestWidget();

    virtual WaveformWidgetType::Type getType() const { return WaveformWidgetType::GLVSyncTest; }

    static inline QString getWaveformWidgetName() { return tr("VSyncTest"); }
    static inline bool useOpenGl() { return true; }
    static inline bool useOpenGLShaders() { return false; }
    static inline bool developerOnly() { return true; }

  protected:
    virtual void castToQWidget();
    virtual void paintEvent(QPaintEvent* event);

  protected slots:
    mixxx::Duration render() override;

  private:
    friend class WaveformWidgetFactory;
};
#endif // GLVSYNCTESTWIDGET_H
