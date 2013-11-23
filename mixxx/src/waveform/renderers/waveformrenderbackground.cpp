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
    m_backgroundColor = m_waveformRenderer->getWaveformSignalColors()->getBgColor();
    m_backgroundPixmapPath = WWidget::selectNodeQString(node, "BgPixmap");
    setDirty(true);
}

void WaveformRenderBackground::draw(QPainter* painter,
                                    QPaintEvent* /*event*/) {
    if (isDirty()) {
        generateImage();
    }

    // since we use opaque widget we need to draw the background !
    painter->drawImage(QPoint(0, 0), m_backgroundImage);

    // This produces a white back ground with Linux QT 4.6 QGlWidget and
    // Intel i915 driver and has peroformance issues on other setups. See lp:981210
    //painter->drawPixmap(QPoint(0, 0), m_backgroundPixmap);
}

void WaveformRenderBackground::generateImage() {
    if (m_backgroundPixmapPath != "") {
        QImage backgroundImage(WWidget::getPath(m_backgroundPixmapPath));

        if (!backgroundImage.isNull()){
            if (backgroundImage.width() == m_waveformRenderer->getWidth() &&
                    backgroundImage.height() == m_waveformRenderer->getHeight()) {
                m_backgroundImage = backgroundImage.convertToFormat(QImage::Format_RGB32);
            } else {
                qWarning() << "WaveformRenderBackground::generateImage() - file("
                           << WWidget::getPath(m_backgroundPixmapPath)
                           << ")" << backgroundImage.width()
                           << "x" << backgroundImage.height()
                           << "do not fit the waveform widget size"
                           << m_waveformRenderer->getWidth()
                           << "x" << m_waveformRenderer->getHeight();

                m_backgroundImage = QImage(m_waveformRenderer->getWidth(),
                                             m_waveformRenderer->getHeight(),
                                             QImage::Format_RGB32);
                QPainter painter(&m_backgroundImage);
                painter.setRenderHint(QPainter::SmoothPixmapTransform);
                painter.drawImage(m_backgroundImage.rect(),
                                   backgroundImage, backgroundImage.rect());
            }
        } else {
            qWarning() << "WaveformRenderBackground::generatePixmap - file("
                       << WWidget::getPath(m_backgroundPixmapPath)
                       << ") is not valid ...";
            m_backgroundImage = QImage(m_waveformRenderer->getWidth(),
                                         m_waveformRenderer->getHeight(),
                                         QImage::Format_RGB32);
            m_backgroundImage.fill(m_backgroundColor.rgb());
        }
    } else {
        qWarning() << "WaveformRenderBackground::generatePixmap - no background file";
        m_backgroundImage = QImage(m_waveformRenderer->getWidth(),
                                     m_waveformRenderer->getHeight(),
                                     QImage::Format_RGB32);
        m_backgroundImage.fill(m_backgroundColor.rgb());
    }
    setDirty(false);
}
