#ifndef QTHSVWAVEFORMWIDGET_H
#define QTHSVWAVEFORMWIDGET_H

#include <QByteArrayData>
#include <QGLWidget>
#include <QString>
#include <QStringBuilder>

#include "util/duration.h"
#include "waveform/widgets/waveformwidgettype.h"
#include "waveformwidgetabstract.h"

class QObject;
class QPaintEvent;
class QWidget;

class QtHSVWaveformWidget : public QGLWidget, public WaveformWidgetAbstract {
    Q_OBJECT
  public:
    virtual ~QtHSVWaveformWidget();

    virtual WaveformWidgetType::Type getType() const { return WaveformWidgetType::QtHSVWaveform; }

    static inline QString getWaveformWidgetName() { return tr("HSV") + " - Qt"; }
    static inline bool useOpenGl() { return true; }
    static inline bool useOpenGles() { return true; }
    static inline bool useOpenGLShaders() { return false; }
    static inline bool developerOnly() { return false; }

  protected:
    virtual void castToQWidget();
    virtual void paintEvent(QPaintEvent* event);
    virtual mixxx::Duration render();

  private:
    QtHSVWaveformWidget(const QString& group, QWidget* parent);
    friend class WaveformWidgetFactory;
};

#endif // QTHSVWAVEFORMWIDGET_H
