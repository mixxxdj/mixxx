#include "waveformrenderbackground.h"

#include "waveformwidgetrenderer.h"
#include "widget/wimagestore.h"

WaveformRenderBackground::WaveformRenderBackground(
    WaveformWidgetRenderer* waveformWidgetRenderer)
        : WaveformRendererAbstract(waveformWidgetRenderer),
          m_backgroundColor(0, 0, 0) {
}

WaveformRenderBackground::~WaveformRenderBackground() {
}

void WaveformRenderBackground::setup(const QDomNode& node,
                                     const SkinContext& context) {
    m_backgroundColor = m_waveformRenderer->getWaveformSignalColors()->getBgColor();
    QString backgroundPixmapPath = context.selectString(node, "BgPixmap");
    if (!backgroundPixmapPath.isEmpty()) {
        m_backgroundPixmapPath = context.makeSkinPath(backgroundPixmapPath);
    }
    setDirty(true);
}

bool WaveformRenderBackground::hasImage() {
    if (isDirty()) {
        generateImage();
    }

    return !m_backgroundImage.isNull();
}

void WaveformRenderBackground::drawImage(QPainter* painter) {
    // since we use opaque widget we need to draw the background !
    painter->drawImage(QPoint(0, 0), m_backgroundImage);

    // This produces a white back ground with Linux QT 4.6 QGlWidget and
    // Intel i915 driver and has peroformance issues on other setups. See #6363
    //painter->drawPixmap(QPoint(0, 0), m_backgroundPixmap);
}

void WaveformRenderBackground::draw(QPainter* painter,
        QPaintEvent* /*event*/) {
    if (hasImage()) {
        drawImage(painter);
        return;
    }
    // If there is no background image, just fill the painter with the
    // background color.
    painter->fillRect(0,
            0,
            m_waveformRenderer->getWidth(),
            m_waveformRenderer->getHeight(),
            m_backgroundColor);
}

void WaveformRenderBackground::generateImage() {
    m_backgroundImage = QImage();
    if (!m_backgroundPixmapPath.isEmpty()) {
        QImage backgroundImage = *WImageStore::getImage(
                m_backgroundPixmapPath,
                scaleFactor());

        if (!backgroundImage.isNull()) {
            if (backgroundImage.width() == m_waveformRenderer->getWidth() &&
                    backgroundImage.height() == m_waveformRenderer->getHeight()) {
                m_backgroundImage = backgroundImage.convertToFormat(QImage::Format_RGB32);
            } else {
                m_backgroundImage = QImage(m_waveformRenderer->getWidth(),
                                           m_waveformRenderer->getHeight(),
                                           QImage::Format_RGB32);
                QPainter painter(&m_backgroundImage);
                painter.setRenderHint(QPainter::SmoothPixmapTransform);
                painter.drawImage(m_backgroundImage.rect(),
                                  backgroundImage, backgroundImage.rect());
            }
        }
    }
    setDirty(false);
}
