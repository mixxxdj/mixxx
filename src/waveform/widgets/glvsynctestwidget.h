#ifndef GLVSYNCTESTWIDGET_H
#define GLVSYNCTESTWIDGET_H

#include <QByteArrayData>
#include <QGLWidget>
#include <QString>

#include "util/duration.h"
#include "waveform/widgets/waveformwidgettype.h"
#include "waveformwidgetabstract.h"

class QObject;
class QPaintEvent;
class QWidget;

class GLVSyncTestWidget : public QGLWidget, public WaveformWidgetAbstract {
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
#endif // GLVSYNCTESTWIDGET_H
