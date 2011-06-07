#ifndef EMPTYWAVEFORM_H
#define EMPTYWAVEFORM_H

#include "waveform/waveformwidgetabstract.h"
#include <QWidget>

//This class can be used as a template file to create new WaveformWidgets
//it contain minimal set of method to re-implement

class EmptyWaveform : public WaveformWidgetAbstract, public QWidget
{
public:
    virtual ~EmptyWaveform();

    virtual QString getWaveformWidgetName() { return "Empty";}
    virtual WaveformWidgetType::Type getType() const { return WaveformWidgetType::EmptyWaveform;}
    virtual bool useOpenGl() const { return false;}

protected:
    virtual void castToQWidget();
    virtual void paintEvent(QPaintEvent* event);

private:
    EmptyWaveform() {}
    EmptyWaveform( const char* group, QWidget* parent);
    friend class WaveformWidgetFactory;
};

#endif // EMPTYWAVEFORM_H
