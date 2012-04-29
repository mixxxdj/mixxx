#ifndef SOFTWAREWAVEFORMWIDGET_H
#define SOFTWAREWAVEFORMWIDGET_H

#include <QWidget>

#include "waveformwidgetabstract.h"

class SoftwareWaveformWidget : public QWidget, public WaveformWidgetAbstract {
    Q_OBJECT
  public:
    virtual ~SoftwareWaveformWidget();

    virtual QString getWaveformWidgetName() { return tr("Filtered");}
    virtual WaveformWidgetType::Type getType() const { return WaveformWidgetType::SoftwareWaveform;}

    virtual bool useOpenGl() const { return false;}
    virtual bool useOpenGLShaders() const { return false;}

  protected:
    virtual void castToQWidget();
    virtual void paintEvent(QPaintEvent* event);

    virtual void updateVisualSamplingPerPixel();

  private:
    SoftwareWaveformWidget() {}
    SoftwareWaveformWidget(const char* group, QWidget* parent);
    friend class WaveformWidgetFactory;
};

#endif // SOFTWAREWAVEFORMWIDGET_H
