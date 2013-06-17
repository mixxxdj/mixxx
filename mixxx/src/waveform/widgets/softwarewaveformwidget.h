#ifndef SOFTWAREWAVEFORMWIDGET_H
#define SOFTWAREWAVEFORMWIDGET_H

#include <QWidget>

#include "waveformwidgetabstract.h"

class SoftwareWaveformWidget : public QWidget, public WaveformWidgetAbstract {
    Q_OBJECT
  public:
    virtual ~SoftwareWaveformWidget();

    virtual WaveformWidgetType::Type getType() const { return WaveformWidgetType::SoftwareWaveform;}

    static inline QString getWaveformWidgetName() { return tr("Filtered") + " - " + tr("Software");}
    static inline bool useOpenGl() { return false;}
    static inline bool useOpenGLShaders() { return false;}

  protected:
    virtual void castToQWidget();
    virtual void paintEvent(QPaintEvent* event);

  private:
    SoftwareWaveformWidget() {}
    SoftwareWaveformWidget(const char* group, QWidget* parent);
    friend class WaveformWidgetFactory;
};

#endif // SOFTWAREWAVEFORMWIDGET_H
