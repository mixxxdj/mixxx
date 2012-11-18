#include "waveformrenderbackground.h"
#include "waveformwidgetrenderer.h"

#include "widget/wskincolor.h"
#include "widget/wwidget.h"

WaveformRenderBackground::WaveformRenderBackground(
    WaveformWidgetRenderer* waveformWidgetRenderer)
        : WaveformRendererAbstract(waveformWidgetRenderer),
          m_backgroundColor(0, 0, 0) {
}

WaveformRenderBackground::~WaveformRenderBackground() {
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

    //since we use opaque widget we need to draw the background !
    painter->drawPixmap(QPoint(0, 0), m_backgroundPixmap);
}

void WaveformRenderBackground::generatePixmap() {
    if (m_backgroundPixmapPath != "") {
        QPixmap backgroundPixmap(WWidget::getPath(m_backgroundPixmapPath));

        if (!backgroundPixmap.isNull()){
            if (backgroundPixmap.width() == m_waveformRenderer->getWidth() &&
                    backgroundPixmap.height() == m_waveformRenderer->getHeight()) {
                m_backgroundPixmap = backgroundPixmap;
            } else {
                qWarning() << "WaveformRenderBackground::generatePixmap - file("
                           << WWidget::getPath(m_backgroundPixmapPath)
                           << ")" << backgroundPixmap.width()
                           << "x" << backgroundPixmap.height()
                           << "do not fit the waveform widget size"
                           << m_waveformRenderer->getWidth()
                           << "x" << m_waveformRenderer->getHeight();

                m_backgroundPixmap = QPixmap(m_waveformRenderer->getWidth(),
                                             m_waveformRenderer->getHeight());
                QPainter painter(&m_backgroundPixmap);
                painter.setRenderHint(QPainter::SmoothPixmapTransform);
                painter.drawPixmap(m_backgroundPixmap.rect(),
                                   backgroundPixmap, backgroundPixmap.rect());
            }
        } else {
            qWarning() << "WaveformRenderBackground::generatePixmap - file("
                       << WWidget::getPath(m_backgroundPixmapPath)
                       << ") is not valid ...";
            m_backgroundPixmap = QPixmap(m_waveformRenderer->getWidth(),
                                         m_waveformRenderer->getHeight());
            m_backgroundPixmap.fill(m_backgroundColor);
        }
    } else {
        qWarning() << "WaveformRenderBackground::generatePixmap - no background file";
        m_backgroundPixmap = QPixmap(m_waveformRenderer->getWidth(),
                                     m_waveformRenderer->getHeight());
        m_backgroundPixmap.fill(m_backgroundColor);
    }
    setDirty(false);
}
