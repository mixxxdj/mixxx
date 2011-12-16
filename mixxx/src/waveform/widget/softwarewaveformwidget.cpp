#include "softwarewaveformwidget.h"

#include <QPainter>

#include "waveform/waveformwidgetrenderer.h"
#include "waveform/waveformrenderbackground.h"
#include "waveform/waveformrendermark.h"
#include "waveform/waveformrendermarkrange.h"

//For the moment we use the same signal renderer until we build a brand new one for the GL version
//a one that actually needs **hardware accelation**

#include "waveform/waveformrendererfilteredsignal.h"

SoftwareWaveformWidget::SoftwareWaveformWidget( const char* group, QWidget* parent) :
    WaveformWidgetAbstract(group),
    QWidget(parent) {
    qDebug() << "SoftwareWaveformWidget";

    m_waveformWidgetRenderer->addRenderer<WaveformRenderBackground>();
    m_waveformWidgetRenderer->addRenderer<WaveformRenderMarkRange>();
    m_waveformWidgetRenderer->addRenderer<WaveformRendererFilteredSignal>();
    m_waveformWidgetRenderer->addRenderer<WaveformRenderMark>();
    m_waveformWidgetRenderer->init();
}

SoftwareWaveformWidget::~SoftwareWaveformWidget() {
}

void SoftwareWaveformWidget::castToQWidget() {
    m_widget = static_cast<QWidget*>(this);
}

void SoftwareWaveformWidget::paintEvent( QPaintEvent* event) {
    QPainter painter(this);
    m_waveformWidgetRenderer->draw(&painter,event);
}
