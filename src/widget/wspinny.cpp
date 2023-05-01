#include "widget/wspinny.h"

WSpinny::WSpinny(
        QWidget* parent,
        const QString& group,
        UserSettingsPointer pConfig,
        VinylControlManager* pVCMan,
        BaseTrackPlayer* pPlayer)
        : WSpinnyBase(parent, group, pConfig, pVCMan, pPlayer) {
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

#ifdef __VINYLCONTROL__
    // Overlay the signal quality drawing if vinyl is active
    if (m_bVinylActive && m_bSignalActive) {
        // draw the last good image
        p.drawImage(this->rect(), m_qImage);
    }
#endif

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
        p.drawImage(-(m_ghostImageScaled.width() / 2),
                    -(m_ghostImageScaled.height() / 2), m_ghostImageScaled);

        //Rotate back to the playback position (not the ghost position),
        //and draw the beat marks from there.
        p.restore();
    }

    if (m_pFgImage && !m_pFgImage->isNull()) {
        // Now rotate the image and draw it on the screen.
        p.rotate(m_fAngle);
        p.drawImage(-(m_fgImageScaled.width() / 2),
                    -(m_fgImageScaled.height() / 2), m_fgImageScaled);
    }
}

void WSpinny::coverChanged() {
}
