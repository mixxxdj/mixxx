#include "waveform/waveformrenderbackground.h"

#include "waveform/waveformwidgetrenderer.h"
#include "widget/wskincolor.h"
#include "widget/wwidget.h"

WaveformRenderBackground::WaveformRenderBackground(
    WaveformWidgetRenderer* waveformWidgetRenderer)
        : WaveformRendererAbstract(waveformWidgetRenderer),
          m_backgroundColor(0, 0, 0) {
}

WaveformRenderBackground::~WaveformRenderBackground() {
}

void WaveformRenderBackground::init() {
}

void WaveformRenderBackground::setup(const QDomNode& node) {
    m_backgroundColor.setNamedColor(
        WWidget::selectNodeQString(node, "BgColor"));
    m_backgroundColor = WSkinColor::getCorrectColor(m_backgroundColor);
    m_backgroundPixmapPath = WWidget::selectNodeQString(node, "BgPixmap");
    setDirty(true);
}

void WaveformRenderBackground::draw(QPainter* painter,
                                    QPaintEvent* /*event*/) {
    if (isDirty()) {
        generatePixmap();
    }
    if (!m_backgroundPixmap.isNull()) {
        painter->drawPixmap(QPoint(0, 0), m_backgroundPixmap);
    }
}

void WaveformRenderBackground::generatePixmap() {
    if (m_backgroundPixmapPath != "") {
        m_backgroundPixmap = QPixmap(WWidget::getPath(m_backgroundPixmapPath));
    } else {
        m_backgroundPixmap = QPixmap();
    }
    setDirty(false);
}
