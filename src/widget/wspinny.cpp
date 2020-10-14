#include "widget/wspinny.h"

#include <QApplication>
#include <QMimeData>
#include <QStylePainter>
#include <QUrl>
#include <QWindow>
#include <QtDebug>

#include "control/controlobject.h"
#include "control/controlproxy.h"
#include "library/coverartcache.h"
#include "library/coverartutils.h"
#include "track/track.h"
#include "util/compatibility.h"
#include "util/dnd.h"
#include "util/math.h"
#include "vinylcontrol/vinylcontrol.h"
#include "vinylcontrol/vinylcontrolmanager.h"
#include "waveform/sharedglcontext.h"
#include "waveform/visualplayposition.h"
#include "waveform/vsyncthread.h"
#include "wimagestore.h"

// The SampleBuffers format enables antialiasing.
WSpinny::WSpinny(
        QWidget* parent,
        const QString& group,
        UserSettingsPointer pConfig,
        VinylControlManager* pVCMan,
        BaseTrackPlayer* pPlayer)
        : QGLWidget(parent, SharedGLContext::getWidget()),
          WBaseWidget(this),
          m_group(group),
          m_pConfig(pConfig),
          m_pPlayPos(nullptr),
          m_pVisualPlayPos(nullptr),
          m_pTrackSamples(nullptr),
          m_pTrackSampleRate(nullptr),
          m_pScratchToggle(nullptr),
          m_pScratchPos(nullptr),
          m_pVinylControlSpeedType(nullptr),
          m_pVinylControlEnabled(nullptr),
          m_pSignalEnabled(nullptr),
          m_pSlipEnabled(nullptr),
          m_bShowCover(true),
          m_dInitialPos(0.),
          m_iVinylInput(-1),
          m_bVinylActive(false),
          m_bSignalActive(true),
          m_iVinylScopeSize(0),
          m_fAngle(0.0f),
          m_dAngleCurrentPlaypos(-1),
          m_dAngleLastPlaypos(-1),
          m_fGhostAngle(0.0f),
          m_dGhostAngleCurrentPlaypos(-1),
          m_dGhostAngleLastPlaypos(-1),
          m_iStartMouseX(-1),
          m_iStartMouseY(-1),
          m_iFullRotations(0),
          m_dPrevTheta(0.),
          m_dTheta(0.),
          m_dRotationsPerSecond(MIXXX_VINYL_SPEED_33_NUM / 60),
          m_bClampFailedWarning(false),
          m_bGhostPlayback(false),
          m_pPlayer(pPlayer),
          m_pDlgCoverArt(new DlgCoverArtFullSize(parent, pPlayer)),
          m_pCoverMenu(new WCoverArtMenu(this)) {
#ifdef __VINYLCONTROL__
    m_pVCManager = pVCMan;
#endif
    //Drag and drop
    setAcceptDrops(true);
    qDebug() << "WSpinny(): Created QGLWidget, Context"
             << "Valid:" << context()->isValid()
             << "Sharing:" << context()->isSharing();
    if (QGLContext::currentContext() != context()) {
        makeCurrent();
    }

    CoverArtCache* pCache = CoverArtCache::instance();
    if (pCache) {
        connect(pCache,
                &CoverArtCache::coverFound,
                this,
                &WSpinny::slotCoverFound);
    }

    if (m_pPlayer != nullptr) {
        connect(m_pPlayer, SIGNAL(newTrackLoaded(TrackPointer)),
                this, SLOT(slotLoadTrack(TrackPointer)));
        connect(m_pPlayer, SIGNAL(loadingTrack(TrackPointer, TrackPointer)),
                this, SLOT(slotLoadingTrack(TrackPointer, TrackPointer)));
        // just in case a track is already loaded
        slotLoadTrack(m_pPlayer->getLoadedTrack());
    }

    connect(m_pCoverMenu, SIGNAL(coverInfoSelected(const CoverInfoRelative&)),
        this, SLOT(slotCoverInfoSelected(const CoverInfoRelative&)));
    connect(m_pCoverMenu, SIGNAL(reloadCoverArt()),
        this, SLOT(slotReloadCoverArt()));

    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_OpaquePaintEvent);

    setAutoFillBackground(false);
    setAutoBufferSwap(false);
}

WSpinny::~WSpinny() {
#ifdef __VINYLCONTROL__
    m_pVCManager->removeSignalQualityListener(this);
#endif
}

void WSpinny::onVinylSignalQualityUpdate(const VinylSignalQualityReport& report) {
#ifdef __VINYLCONTROL__
    if (!m_bVinylActive || !m_bSignalActive) {
        return;
    }
    // Skip reports for vinyl inputs we don't care about.
    if (report.processor != m_iVinylInput) {
        return;
    }
    int r,g,b;
    QColor qual_color = QColor();
    float signalQuality = report.timecode_quality;

    // color is related to signal quality
    // hsv:  s=1, v=1
    // h is the only variable.
    // h=0 is red, h=120 is green
    qual_color.setHsv(static_cast<int>(120.0 * signalQuality), 255, 255);
    qual_color.getRgb(&r, &g, &b);

    for (int y = 0; y < m_iVinylScopeSize; ++y) {
        QRgb *line = reinterpret_cast<QRgb *>(m_qImage.scanLine(y));
        for (int x = 0; x < m_iVinylScopeSize; ++x) {
            // use xwax's bitmap to set alpha data only
            // adjust alpha by 3/4 so it's not quite so distracting
            // setpixel is slow, use scanlines instead
            //m_qImage.setPixel(x, y, qRgba(r,g,b,(int)buf[x+m_iVinylScopeSize*y] * .75));
            *line = qRgba(r,g,b,static_cast<int>(report.scope[x+m_iVinylScopeSize*y] * .75));
            line++;
        }
    }
#endif
}

void WSpinny::setup(const QDomNode& node, const SkinContext& context) {
    // Set images
    QDomElement backPathElement = context.selectElement(node, "PathBackground");
    m_pBgImage = WImageStore::getImage(context.getPixmapSource(backPathElement),
                                       context.getScaleFactor());
    Paintable::DrawMode bgmode = context.selectScaleMode(backPathElement,
                                                         Paintable::FIXED);
    if (m_pBgImage && !m_pBgImage->isNull() && bgmode == Paintable::FIXED) {
        setFixedSize(m_pBgImage->size());
    } else {
        setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    }
    m_pMaskImage = WImageStore::getImage(
            context.getPixmapSource(context.selectNode(node, "PathMask")),
            context.getScaleFactor());
    m_pFgImage = WImageStore::getImage(
            context.getPixmapSource(context.selectNode(node,"PathForeground")),
            context.getScaleFactor());
    if (m_pFgImage && !m_pFgImage->isNull()) {
        m_fgImageScaled = m_pFgImage->scaled(
                size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    m_pGhostImage = WImageStore::getImage(
            context.getPixmapSource(context.selectNode(node,"PathGhost")),
            context.getScaleFactor());
    if (m_pGhostImage && !m_pGhostImage->isNull()) {
        m_ghostImageScaled = m_pGhostImage->scaled(
                size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    m_bShowCover = context.selectBool(node, "ShowCover", false);

#ifdef __VINYLCONTROL__
    // Find the vinyl input we should listen to reports about.
    if (m_pVCManager) {
        m_iVinylInput = m_pVCManager->vinylInputFromGroup(m_group);
    }
    m_iVinylScopeSize = MIXXX_VINYL_SCOPE_SIZE;
    m_qImage = QImage(m_iVinylScopeSize, m_iVinylScopeSize, QImage::Format_ARGB32);
    // fill with transparent black
    m_qImage.fill(qRgba(0,0,0,0));
#endif

    m_pPlayPos = new ControlProxy(
            m_group, "playposition", this, ControlFlag::NoAssertIfMissing);
    m_pVisualPlayPos = VisualPlayPosition::getVisualPlayPosition(m_group);
    m_pTrackSamples = new ControlProxy(
            m_group, "track_samples", this, ControlFlag::NoAssertIfMissing);
    m_pTrackSampleRate = new ControlProxy(
            m_group, "track_samplerate", this, ControlFlag::NoAssertIfMissing);

    m_pScratchToggle = new ControlProxy(
            m_group, "scratch_position_enable", this, ControlFlag::NoAssertIfMissing);
    m_pScratchPos = new ControlProxy(
            m_group, "scratch_position", this, ControlFlag::NoAssertIfMissing);

    m_pSlipEnabled = new ControlProxy(
            m_group, "slip_enabled", this, ControlFlag::NoAssertIfMissing);
    m_pSlipEnabled->connectValueChanged(this, &WSpinny::updateSlipEnabled);

#ifdef __VINYLCONTROL__
    m_pVinylControlSpeedType = new ControlProxy(
            m_group, "vinylcontrol_speed_type", this, ControlFlag::NoAssertIfMissing);
    // Initialize the rotational speed.
    updateVinylControlSpeed(m_pVinylControlSpeedType->get());

    m_pVinylControlEnabled = new ControlProxy(
            m_group, "vinylcontrol_enabled", this, ControlFlag::NoAssertIfMissing);
    m_pVinylControlEnabled->connectValueChanged(this,
            &WSpinny::updateVinylControlEnabled);

    m_pSignalEnabled = new ControlProxy(
            m_group, "vinylcontrol_signal_enabled", this, ControlFlag::NoAssertIfMissing);
    m_pSignalEnabled->connectValueChanged(this,
            &WSpinny::updateVinylControlSignalEnabled);

    // Match the vinyl control's set RPM so that the spinny widget rotates at
    // the same speed as your physical decks, if you're using vinyl control.
    m_pVinylControlSpeedType->connectValueChanged(this,
            &WSpinny::updateVinylControlSpeed);


#else
    //if no vinyl control, just call it 33
    this->updateVinylControlSpeed(33.0);
#endif
}

void WSpinny::slotLoadTrack(TrackPointer pTrack) {
    if (m_loadedTrack) {
        disconnect(m_loadedTrack.get(), SIGNAL(coverArtUpdated()),
                   this, SLOT(slotTrackCoverArtUpdated()));
    }
    m_lastRequestedCover = CoverInfo();
    m_loadedCover = QPixmap();
    m_loadedCoverScaled = QPixmap();
    m_loadedTrack = pTrack;
    if (m_loadedTrack) {
        connect(m_loadedTrack.get(), SIGNAL(coverArtUpdated()),
                this, SLOT(slotTrackCoverArtUpdated()));
    }

    slotTrackCoverArtUpdated();
}

void WSpinny::slotLoadingTrack(TrackPointer pNewTrack, TrackPointer pOldTrack) {
    Q_UNUSED(pNewTrack);
    if (m_loadedTrack && pOldTrack == m_loadedTrack) {
        disconnect(m_loadedTrack.get(), SIGNAL(coverArtUpdated()),
                   this, SLOT(slotTrackCoverArtUpdated()));
    }
    m_loadedTrack.reset();
    m_lastRequestedCover = CoverInfo();
    m_loadedCover = QPixmap();
    m_loadedCoverScaled = QPixmap();
    update();
}

void WSpinny::slotTrackCoverArtUpdated() {
    if (m_loadedTrack) {
        CoverArtCache::requestTrackCover(this, m_loadedTrack);
    }
}

void WSpinny::slotCoverFound(
        const QObject* pRequestor,
        const CoverInfo& coverInfo,
        const QPixmap& pixmap,
        quint16 requestedHash,
        bool coverInfoUpdated) {
    Q_UNUSED(requestedHash);
    Q_UNUSED(coverInfoUpdated);
    if (pRequestor == this &&
            m_loadedTrack &&
            m_loadedTrack->getLocation() == coverInfo.trackLocation) {
        m_loadedCover = pixmap;
        m_loadedCoverScaled = scaledCoverArt(pixmap);
        update();
    }
}

void WSpinny::slotCoverInfoSelected(const CoverInfoRelative& coverInfo) {
    if (m_loadedTrack != nullptr) {
        // Will trigger slotTrackCoverArtUpdated().
        m_loadedTrack->setCoverInfo(coverInfo);
    }
}

void WSpinny::slotReloadCoverArt() {
    if (!m_loadedTrack) {
        return;
    }
    guessTrackCoverInfoConcurrently(m_loadedTrack);
}

void WSpinny::paintEvent(QPaintEvent *e) {
    Q_UNUSED(e);
}

void WSpinny::render(VSyncThread* vSyncThread) {
    if (!isValid() || !isVisible()) {
        return;
    }

    auto window = windowHandle();
    if (window == nullptr || !window->isExposed()) {
        return;
    }

    if (!m_pVisualPlayPos.isNull() && vSyncThread != nullptr) {
        m_pVisualPlayPos->getPlaySlipAtNextVSync(
                vSyncThread,
                &m_dAngleCurrentPlaypos,
                &m_dGhostAngleCurrentPlaypos);
    }

    double scaleFactor = getDevicePixelRatioF(this);

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);

    if (m_pBgImage) {
        p.drawImage(rect(), *m_pBgImage, m_pBgImage->rect());
    }

    if (m_bShowCover && !m_loadedCoverScaled.isNull()) {
        // Some covers aren't square, so center them.
        int x = (width() - m_loadedCoverScaled.width() / scaleFactor) / 2;
        int y = (height() - m_loadedCoverScaled.height() / scaleFactor) / 2;
        p.drawPixmap(x, y, m_loadedCoverScaled);
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

    if (m_dAngleCurrentPlaypos != m_dAngleLastPlaypos) {
        m_fAngle = calculateAngle(m_dAngleCurrentPlaypos);
        m_dAngleLastPlaypos = m_dAngleCurrentPlaypos;
    }

    if (m_dGhostAngleCurrentPlaypos != m_dGhostAngleLastPlaypos) {
        m_fGhostAngle = calculateAngle(m_dGhostAngleCurrentPlaypos);
        m_dGhostAngleLastPlaypos = m_dGhostAngleCurrentPlaypos;
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

void WSpinny::swap() {
    if (!isValid() || !isVisible()) {
        return;
    }
    auto window = windowHandle();
    if (window == nullptr || !window->isExposed()) {
        return;
    }
    swapBuffers();
}


QPixmap WSpinny::scaledCoverArt(const QPixmap& normal) {
    if (normal.isNull()) {
        return QPixmap();
    }
    QPixmap scaled = normal.scaled(size() * getDevicePixelRatioF(this),
            Qt::KeepAspectRatio, Qt::SmoothTransformation);
    scaled.setDevicePixelRatio(getDevicePixelRatioF(this));
    return scaled;
}

void WSpinny::resizeEvent(QResizeEvent* /*unused*/) {
    m_loadedCoverScaled = scaledCoverArt(m_loadedCover);
    if (m_pFgImage && !m_pFgImage->isNull()) {
        m_fgImageScaled = m_pFgImage->scaled(
                size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    if (m_pGhostImage && !m_pGhostImage->isNull()) {
        m_ghostImageScaled = m_pGhostImage->scaled(
                size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
}

/* Convert between a normalized playback position (0.0 - 1.0) and an angle
   in our polar coordinate system.
   Returns an angle clamped between -180 and 180 degrees. */
double WSpinny::calculateAngle(double playpos) {
    double trackFrames = m_pTrackSamples->get() / 2;
    double trackSampleRate = m_pTrackSampleRate->get();
    if (isnan(playpos) || isnan(trackFrames) || isnan(trackSampleRate) ||
        trackFrames <= 0 || trackSampleRate <= 0) {
        return 0.0;
    }

    // Convert playpos to seconds.
    double t = playpos * trackFrames / trackSampleRate;

    // Bad samplerate or number of track samples.
    if (isnan(t)) {
        return 0.0;
    }

    //33 RPM is approx. 0.5 rotations per second.
    double angle = 360.0 * m_dRotationsPerSecond * t;
    //Clamp within -180 and 180 degrees
    //qDebug() << "pc:" << angle;
    //angle = ((angle + 180) % 360.) - 180;
    //modulo for doubles :)
    const double originalAngle = angle;
    if (angle > 0)
    {
        int x = (angle+180)/360;
        angle = angle - (360*x);
    } else
    {
        int x = (angle-180)/360;
        angle = angle - (360*x);
    }

    if (angle <= -180 || angle > 180) {
        // Only warn once per session. This can tank performance since it prints
        // like crazy.
        if (!m_bClampFailedWarning) {
            qDebug() << "Angle clamping failed!" << t << originalAngle << "->" << angle
                     << "Please file a bug or email mixxx-devel@lists.sourceforge.net";
            m_bClampFailedWarning = true;
        }
        return 0.0;
    }
    return angle;
}

/** Given a normalized playpos, calculate the integer number of rotations
    that it would take to wind the vinyl to that position. */
int WSpinny::calculateFullRotations(double playpos) {
    if (isnan(playpos)) {
        return 0;
    }
    //Convert playpos to seconds.
    double t = playpos * (m_pTrackSamples->get() / 2 /  // Stereo audio!
                          m_pTrackSampleRate->get());

    //33 RPM is approx. 0.5 rotations per second.
    //qDebug() << t;
    double angle = 360 * m_dRotationsPerSecond * t;

    return ((static_cast<int>(angle) + 180) / 360);
}

//Inverse of calculateAngle()
double WSpinny::calculatePositionFromAngle(double angle) {
    if (isnan(angle)) {
        return 0.0;
    }

    //33 RPM is approx. 0.5 rotations per second.
    double t = angle/(360.0 * m_dRotationsPerSecond); //time in seconds

    double trackFrames = m_pTrackSamples->get() / 2;
    double trackSampleRate = m_pTrackSampleRate->get();
    if (isnan(trackFrames) || isnan(trackSampleRate) ||
        trackFrames <= 0 || trackSampleRate <= 0) {
        return 0.0;
    }

    // Convert t from seconds into a normalized playposition value.
    double playpos = t * trackSampleRate / trackFrames;
    if (isnan(playpos)) {
        return 0.0;
    }
    return playpos;
}

void WSpinny::updateVinylControlSpeed(double rpm) {
    m_dRotationsPerSecond = rpm/60.;
}

void WSpinny::updateVinylControlSignalEnabled(double enabled) {
#ifdef __VINYLCONTROL__
    if (m_pVCManager == nullptr) {
        return;
    }
    m_bSignalActive = enabled;

    if (enabled && m_iVinylInput != -1) {
        m_pVCManager->addSignalQualityListener(this);
    } else {
        m_pVCManager->removeSignalQualityListener(this);
        // fill with transparent black
        m_qImage.fill(qRgba(0,0,0,0));
    }
#endif
}

void WSpinny::updateVinylControlEnabled(double enabled) {
    m_bVinylActive = enabled;
}

void WSpinny::updateSlipEnabled(double enabled) {
    m_bGhostPlayback = static_cast<bool>(enabled);
}

void WSpinny::mouseMoveEvent(QMouseEvent * e) {
    int y = e->y();
    int x = e->x();

    // Keeping these around in case we want to switch to control relative
    // to the original mouse position.
    //int dX = x-m_iStartMouseX;
    //int dY = y-m_iStartMouseY;

    //Coordinates from center of widget
    double c_x = x - width()/2;
    double c_y = y - height()/2;
    double theta = (180.0/M_PI)*atan2(c_x, -c_y);

    //qDebug() << "c_x:" << c_x << "c_y:" << c_y <<
    //            "dX:" << dX << "dY:" << dY;

    // When we finish one full rotation (clockwise or anticlockwise),
    // we'll need to manually add/sub 360 degrees because atan2()'s range is
    // only within -180 to 180 degrees. We need a wider range so your position
    // in the song can be tracked.
    if (m_dPrevTheta > 100 && theta < 0) {
        m_iFullRotations++;
    } else if (m_dPrevTheta < -100 && theta > 0) {
        m_iFullRotations--;
    }

    m_dPrevTheta = theta;
    theta += m_iFullRotations*360;

    //qDebug() << "c t:" << theta << "pt:" << m_dPrevTheta <<
    //            "icr" << m_iFullRotations;

    if (((e->buttons() & Qt::LeftButton) || (e->buttons() & Qt::RightButton)) &&
            !m_bVinylActive) {
        //Convert deltaTheta into a percentage of song length.
        double absPos = calculatePositionFromAngle(theta);
        double absPosInSamples = absPos * m_pTrackSamples->get();
        m_pScratchPos->set(absPosInSamples - m_dInitialPos);
    } else if (e->buttons() & Qt::MidButton) {
    } else if (e->buttons() & Qt::NoButton) {
        setCursor(QCursor(Qt::OpenHandCursor));
    }
}

void WSpinny::mousePressEvent(QMouseEvent * e) {
    if (m_loadedTrack == nullptr) {
        return;
    }

    if (m_pDlgCoverArt->isVisible()) {
        m_pDlgCoverArt->close();
        return;
    }

    if (m_pCoverMenu->isVisible()) {
        m_pCoverMenu->close();
        return;
    }

    if (e->button() == Qt::LeftButton) {
        int y = e->y();
        int x = e->x();

        m_iStartMouseX = x;
        m_iStartMouseY = y;

        //don't do anything if vinyl control is active
        if (m_bVinylActive) {
            return;
        }

        if (e->button() == Qt::LeftButton || e->button() == Qt::RightButton) {
            QApplication::setOverrideCursor(QCursor(Qt::ClosedHandCursor));

            // Coordinates from center of widget
            double c_x = x - width()/2;
            double c_y = y - height()/2;
            double theta = (180.0/M_PI)*atan2(c_x, -c_y);
            m_dPrevTheta = theta;
            m_iFullRotations = calculateFullRotations(m_pPlayPos->get());
            theta += m_iFullRotations * 360.0;
            m_dInitialPos = calculatePositionFromAngle(theta) * m_pTrackSamples->get();

            m_pScratchPos->set(0);
            m_pScratchToggle->set(1.0);

            // Trigger a mouse move to immediately line up the vinyl with the cursor
            mouseMoveEvent(e);
        }
    } else {
        if (!m_loadedCover.isNull()) {
            m_pDlgCoverArt->init(m_loadedTrack);
        } else if (!m_pDlgCoverArt->isVisible() && m_bShowCover) {
            m_pCoverMenu->popup(e->globalPos());
        }
    }
}

void WSpinny::mouseReleaseEvent(QMouseEvent * e)
{
    if (e->button() == Qt::LeftButton || e->button() == Qt::RightButton) {
        QApplication::restoreOverrideCursor();
        m_pScratchToggle->set(0.0);
        m_iFullRotations = 0;
    }
}

void WSpinny::showEvent(QShowEvent* event) {
    Q_UNUSED(event);
#ifdef __VINYLCONTROL__
    // If we want to draw the VC signal on this widget then register for
    // updates.
    if (m_bSignalActive && m_iVinylInput != -1 && m_pVCManager) {
        m_pVCManager->addSignalQualityListener(this);
    }
#endif
}

void WSpinny::hideEvent(QHideEvent* event) {
    Q_UNUSED(event);
#ifdef __VINYLCONTROL__
    // When we are hidden we do not want signal quality updates.
    if (m_pVCManager) {
        m_pVCManager->removeSignalQualityListener(this);
    }
#endif
    // fill with transparent black
    m_qImage.fill(qRgba(0,0,0,0));
}

bool WSpinny::event(QEvent* pEvent) {
    if (pEvent->type() == QEvent::ToolTip) {
        updateTooltip();
    }
    return QGLWidget::event(pEvent);
}

void WSpinny::dragEnterEvent(QDragEnterEvent* event) {
    DragAndDropHelper::handleTrackDragEnterEvent(event, m_group, m_pConfig);
}

void WSpinny::dropEvent(QDropEvent* event) {
    DragAndDropHelper::handleTrackDropEvent(event, *this, m_group, m_pConfig);
}
