#ifndef SOFTWAREWAVEFORMWIDGET_H
#define SOFTWAREWAVEFORMWIDGET_H

#include "waveformwidgetabstract.h"
#include <QWidget>

class SoftwareWaveformWidget : public WaveformWidgetAbstract, public QWidget
{
public:
    virtual ~SoftwareWaveformWidget();

    virtual QString getWaveformWidgetName() { return "Filtered";}
    virtual WaveformWidgetType::Type getType() const { return WaveformWidgetType::SoftwareWaveform;}

    virtual bool useOpenGl() const { return false;}
    virtual bool useOpenGLShaders() const { return false;}

protected:
    virtual void castToQWidget();
    virtual void paintEvent(QPaintEvent* event);

private:
    SoftwareWaveformWidget() {}
    SoftwareWaveformWidget( const char* group, QWidget* parent);
    friend class WaveformWidgetFactory;
};

#endif // SOFTWAREWAVEFORMWIDGET_H
