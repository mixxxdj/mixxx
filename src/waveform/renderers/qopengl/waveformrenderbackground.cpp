#include "waveform/renderers/qopengl/waveformrenderbackground.h"

#include "waveform/renderers/waveformwidgetrenderer.h"
#include "widget/wimagestore.h"
#include "widget/wskincolor.h"
#include "widget/wwidget.h"

using namespace qopengl;

WaveformRenderBackground::WaveformRenderBackground(
        WaveformWidgetRenderer* waveformWidgetRenderer)
        : WaveformRenderer(waveformWidgetRenderer),
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

void WaveformRenderBackground::renderGL() {
    if (isDirty()) {
        // TODO @m0dB
        //   generateImage();
    }

    // If there is no background image, just fill the painter with the
    // background color.
    if (m_backgroundImage.isNull()) {
        glClearColor(m_backgroundColor.redF(),
                m_backgroundColor.greenF(),
                m_backgroundColor.blueF(),
                1.f);
        glClear(GL_COLOR_BUFFER_BIT);
        return;
    }

    // painter->drawImage(QPoint(0, 0), m_backgroundImage);
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
                        backgroundImage,
                        backgroundImage.rect());
            }
        }
    }
    setDirty(false);
}
