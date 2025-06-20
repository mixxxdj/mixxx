#include "widget/wspinny.h"

#include <QPainter>

#include "moc_wspinny.cpp"

WSpinny::WSpinny(
        QWidget* pParent,
        const QString& group,
        UserSettingsPointer pConfig,
        VinylControlManager* pVCMan,
        BaseTrackPlayer* pPlayer)
        : WSpinnyBase(pParent, group, pConfig, pVCMan, pPlayer) {
}

void WSpinny::draw() {
    double scaleFactor = devicePixelRatioF();

    QPainter p(paintDevice());
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);

    if (m_pBgImage) {
        p.drawImage(rect(), *m_pBgImage, m_pBgImage->rect());
    }

    if (m_bShowCover && !m_loadedCoverScaled.isNull()) {
        // Some covers aren't square, so center them.
        double x = (width() - m_loadedCoverScaled.width() / scaleFactor) / 2;
        double y = (height() - m_loadedCoverScaled.height() / scaleFactor) / 2;
        p.drawPixmap(QPointF(x, y), m_loadedCoverScaled);
    }

    if (m_pMaskImage) {
        p.drawImage(rect(), *m_pMaskImage, m_pMaskImage->rect());
    }

    // Overlay the signal quality drawing if vinyl is active
    if (shouldDrawVinylQuality()) {
        // draw the last good image
        p.drawImage(this->rect(), m_qImage);
    }

    // To rotate the foreground image around the center of the image,
    // we use the classic trick of translating the coordinate system such that
    // the origin is at the center of the image. We then rotate the coordinate system,
    // and draw the image at the corner.
    p.translate(width() / 2, height() / 2);

    bool paintGhost = m_bGhostPlayback && m_pGhostImage && !m_pGhostImage->isNull();
    if (paintGhost) {
        p.save();
    }

    if (paintGhost) {
        p.restore();
        p.save();
        p.rotate(m_fGhostAngle);
        p.drawImage(QPointF(-m_ghostImageScaled.width() / scaleFactor / 2.0,
                            -m_ghostImageScaled.height() / scaleFactor / 2.0),
                m_ghostImageScaled);

        //Rotate back to the playback position (not the ghost position),
        //and draw the beat marks from there.
        p.restore();
    }

    if (m_pFgImage && !m_pFgImage->isNull()) {
        // Now rotate the image and draw it on the screen.
        p.rotate(m_fAngle);
        p.drawImage(QPointF(-m_fgImageScaled.width() / scaleFactor / 2.0,
                            -m_fgImageScaled.height() / scaleFactor / 2.0),
                m_fgImageScaled);
    }
}

void WSpinny::setupVinylSignalQuality() {
    m_qImage = QImage(m_iVinylScopeSize, m_iVinylScopeSize, QImage::Format_ARGB32);
}

void WSpinny::updateVinylSignalQualityImage(const QColor& qual_color, const unsigned char* data) {
    int r, g, b;
    qual_color.getRgb(&r, &g, &b);

    for (int y = 0; y < m_iVinylScopeSize; ++y) {
        QRgb* line = reinterpret_cast<QRgb*>(m_qImage.scanLine(y));
        for (int x = 0; x < m_iVinylScopeSize; ++x) {
            // use xwax's bitmap to set alpha data only
            // adjust alpha by 3/4 so it's not quite so distracting
            // setpixel is slow, use scanlines instead
            // m_qImage.setPixel(x, y, qRgba(r,g,b,(int)buf[x+m_iVinylScopeSize*y] * .75));
            *line = qRgba(r, g, b, static_cast<int>(data[x + m_iVinylScopeSize * y] * .75));
            line++;
        }
    }
}

void WSpinny::coverChanged() {
}
