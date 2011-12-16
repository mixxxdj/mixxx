#include "emptywaveformwidget.h"

#include <QPainter>

#include "waveform/waveformwidgetrenderer.h"
#include "waveform/waveformrenderbackground.h"

EmptyWaveformWidget::EmptyWaveformWidget( const char* group, QWidget* parent) :
    WaveformWidgetAbstract(group),
    QWidget(parent) {
    //Empty means just a background ;)
    m_waveformWidgetRenderer->addRenderer<WaveformRenderBackground>();
    m_waveformWidgetRenderer->init();
}

EmptyWaveformWidget::~EmptyWaveformWidget() {
}

void EmptyWaveformWidget::castToQWidget() {
    m_widget = static_cast<QWidget*>(this);
}

void EmptyWaveformWidget::paintEvent( QPaintEvent* event) {
    QPainter painter(this);
    m_waveformWidgetRenderer->draw(&painter,event);
}
