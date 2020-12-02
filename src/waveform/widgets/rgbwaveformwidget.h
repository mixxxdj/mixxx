#ifndef RGBWAVEFORMWIDGET_H
#define RGBWAVEFORMWIDGET_H

#include <QByteArrayData>
#include <QString>
#include <QWidget>

#include "waveform/widgets/waveformwidgettype.h"
#include "waveformwidgetabstract.h"

class QObject;
class QPaintEvent;

class RGBWaveformWidget : public QWidget, public WaveformWidgetAbstract {
    Q_OBJECT
  public:
    virtual ~RGBWaveformWidget();

    virtual WaveformWidgetType::Type getType() const { return WaveformWidgetType::RGBWaveform; }

    static inline QString getWaveformWidgetName() { return tr("RGB"); }
    static inline bool useOpenGl() { return false; }
    static inline bool useOpenGles() { return false; }
    static inline bool useOpenGLShaders() { return false; }
    static inline bool developerOnly() { return false; }

  protected:
    virtual void castToQWidget();
    virtual void paintEvent(QPaintEvent* event);

  private:
    RGBWaveformWidget(const QString& group, QWidget* parent);
    friend class WaveformWidgetFactory;
};

#endif // RGBWAVEFORMWIDGET_H
