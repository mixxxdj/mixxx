#include "widget/wspinnybase.h"

#include <QApplication>
#include <QtDebug>

#include "control/controlproxy.h"
#include "library/coverartcache.h"
#include "library/coverartutils.h"
#include "library/dlgcoverartfullsize.h"
#include "mixer/basetrackplayer.h"
#include "moc_wspinnybase.cpp"
#include "skin/legacy/skincontext.h"
#include "track/track.h"
#include "util/dnd.h"
#include "util/fpclassify.h"
#include "vinylcontrol/vinylcontrolmanager.h"
#include "waveform/visualplayposition.h"
#include "waveform/vsyncthread.h"
#include "wimagestore.h"

// The SampleBuffers format enables antialiasing.
WSpinnyBase::WSpinnyBase(
        QWidget* pParent,
        const QString& group,
        UserSettingsPointer pConfig,
        VinylControlManager* pVCMan,
        BaseTrackPlayer* pPlayer)
        : WGLWidget(pParent),
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
          m_pShowCoverProxy(nullptr),
          m_bShowCover(true),
          m_dInitialPos(0.),
          m_iVinylInput(-1),
          m_bVinylActive(false),
          m_bSignalActive(true),
          m_bDrawVinylSignalQuality(false),
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
          m_dRotationsPerSecond(MIXXX_VINYL_SPEED_33_NUM / 60),
          m_bClampFailedWarning(false),
          m_bGhostPlayback(false),
          m_pPlayer(pPlayer),
          m_pCoverMenu(new WCoverArtMenu(this)),
          m_pDlgCoverArt(new DlgCoverArtFullSize(this, pPlayer, m_pCoverMenu)) {
#ifdef __VINYLCONTROL__
    m_pVCManager = pVCMan;
#else
    Q_UNUSED(pVCMan);
#endif // __VINYLCONTROL__
    // Drag and drop
    setAcceptDrops(true);

    CoverArtCache* pCache = CoverArtCache::instance();
    if (pCache) {
        connect(pCache,
                &CoverArtCache::coverFound,
                this,
                &WSpinnyBase::slotCoverFound);
    }

    if (m_pPlayer != nullptr) {
        connect(m_pPlayer, &BaseTrackPlayer::newTrackLoaded, this, &WSpinnyBase::slotLoadTrack);
        connect(m_pPlayer, &BaseTrackPlayer::loadingTrack, this, &WSpinnyBase::slotLoadingTrack);
    }

    connect(m_pCoverMenu,
            &WCoverArtMenu::coverInfoSelected,
            this,
            &WSpinnyBase::slotCoverInfoSelected);
    connect(m_pCoverMenu, &WCoverArtMenu::reloadCoverArt, this, &WSpinnyBase::slotReloadCoverArt);

#ifdef MIXXX_USE_QOPENGL
    setTrackDropTarget(this);
#endif
}

WSpinnyBase::~WSpinnyBase() {
#ifdef __VINYLCONTROL__
    m_pVCManager->removeSignalQualityListener(this);
#endif
}

bool WSpinnyBase::shouldDrawVinylQuality() const {
#ifdef __VINYLCONTROL__
    return m_bVinylActive && m_bSignalActive && m_bDrawVinylSignalQuality;
#else
    return false;
#endif
}

void WSpinnyBase::onVinylSignalQualityUpdate(const VinylSignalQualityReport& report) {
#ifdef __VINYLCONTROL__
    if (!m_bVinylActive || !m_bSignalActive) {
        return;
    }
    // Skip reports for vinyl inputs we don't care about.
    if (report.processor != m_iVinylInput) {
        return;
    }
    QColor qual_color = QColor();
    float signalQuality = report.timecode_quality;

    // color is related to signal quality
    // hsv:  s=1, v=1
    // h is the only variable.
    // h=0 is red, h=120 is green
    qual_color.setHsv(static_cast<int>(120.0 * signalQuality), 255, 255);

    updateVinylSignalQualityImage(qual_color, report.scope);
    m_bDrawVinylSignalQuality = true;
#else
    Q_UNUSED(report);
#endif
}

void WSpinnyBase::setup(const QDomNode& node,
        const SkinContext& context,
        const ConfigKey& showCoverConfigKey) {
    if (m_pPlayer) {
        // just in case a track is already loaded
        slotLoadTrack(m_pPlayer->getLoadedTrack());
    }

    // Set images
    QDomElement backPathElement = context.selectElement(node, "PathBackground");
    m_pBgImage = WImageStore::getImage(context.getPixmapSource(backPathElement),
            context.getScaleFactor());
    Paintable::DrawMode bgmode = context.selectScaleMode(backPathElement,
            Paintable::DrawMode::Fixed);
    if (m_pBgImage && !m_pBgImage->isNull() && bgmode == Paintable::DrawMode::Fixed) {
        setFixedSize(m_pBgImage->size());
    } else {
        setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    }
    m_pMaskImage = WImageStore::getImage(
            context.getPixmapSource(context.selectNode(node, "PathMask")),
            context.getScaleFactor());
    m_pFgImage = WImageStore::getImage(
            context.getPixmapSource(context.selectNode(node, "PathForeground")),
            context.getScaleFactor());
    m_fgImageScaled = scaleToSize(m_pFgImage);
    m_pGhostImage = WImageStore::getImage(
            context.getPixmapSource(context.selectNode(node, "PathGhost")),
            context.getScaleFactor());
    m_ghostImageScaled = scaleToSize(m_pGhostImage);

    // Dynamic skin option, set in WSpinnyBase's <ShowCoverControl> node.
    if (showCoverConfigKey.isValid()) {
        m_pShowCoverProxy = new ControlProxy(
                showCoverConfigKey, this);
        m_pShowCoverProxy->connectValueChanged(
                this,
                [this](double v) {
                    m_bShowCover = v > 0.0;
                });
        m_bShowCover = m_pShowCoverProxy->get() > 0.0;
    } else {
        m_bShowCover = context.selectBool(node, "ShowCover", false);
    }

#ifdef __VINYLCONTROL__
    // Find the vinyl input we should listen to reports about.
    if (m_pVCManager) {
        m_iVinylInput = m_pVCManager->vinylInputFromGroup(m_group);
    }
    m_iVinylScopeSize = MIXXX_VINYL_SCOPE_SIZE;
    setupVinylSignalQuality();
    m_bDrawVinylSignalQuality = false;
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
    m_pSlipEnabled->connectValueChanged(this, &WSpinnyBase::updateSlipEnabled);

#ifdef __VINYLCONTROL__
    m_pVinylControlSpeedType = new ControlProxy(
            m_group, "vinylcontrol_speed_type", this, ControlFlag::NoAssertIfMissing);
    // Initialize the rotational speed.
    updateVinylControlSpeed(m_pVinylControlSpeedType->get());

    m_pVinylControlEnabled = new ControlProxy(
            m_group, "vinylcontrol_enabled", this, ControlFlag::NoAssertIfMissing);
    updateVinylControlEnabled(m_pVinylControlEnabled->get());
    m_pVinylControlEnabled->connectValueChanged(this,
            &WSpinnyBase::updateVinylControlEnabled);

    m_pSignalEnabled = new ControlProxy(
            m_group, "vinylcontrol_signal_enabled", this, ControlFlag::NoAssertIfMissing);
    updateVinylControlSignalEnabled(m_pSignalEnabled->get());
    m_pSignalEnabled->connectValueChanged(this,
            &WSpinnyBase::updateVinylControlSignalEnabled);

    // Match the vinyl control's set RPM so that the spinny widget rotates at
    // the same speed as your physical decks, if you're using vinyl control.
    m_pVinylControlSpeedType->connectValueChanged(this,
            &WSpinnyBase::updateVinylControlSpeed);

#else
    // if no vinyl control, just call it 33
    this->updateVinylControlSpeed(33.0);
#endif
}

void WSpinnyBase::setLoadedCover(const QPixmap& pixmap) {
    m_loadedCover = pixmap;
    m_loadedCoverScaled = scaleToSize(pixmap);
}

void WSpinnyBase::slotLoadTrack(TrackPointer pTrack) {
    if (m_pLoadedTrack) {
        disconnect(m_pLoadedTrack.get(),
                &Track::coverArtUpdated,
                this,
                &WSpinnyBase::slotTrackCoverArtUpdated);
    }
    m_lastRequestedCover = CoverInfo();

    setLoadedCover(QPixmap());

    m_pLoadedTrack = pTrack;
    if (m_pLoadedTrack) {
        connect(m_pLoadedTrack.get(),
                &Track::coverArtUpdated,
                this,
                &WSpinnyBase::slotTrackCoverArtUpdated);
    }

    slotTrackCoverArtUpdated();
}

void WSpinnyBase::slotLoadingTrack(TrackPointer pNewTrack, TrackPointer pOldTrack) {
    Q_UNUSED(pNewTrack);
    if (m_pLoadedTrack && pOldTrack == m_pLoadedTrack) {
        disconnect(m_pLoadedTrack.get(),
                &Track::coverArtUpdated,
                this,
                &WSpinnyBase::slotTrackCoverArtUpdated);
    }
    m_pLoadedTrack.reset();
    m_lastRequestedCover = CoverInfo();

    setLoadedCover(QPixmap());
    coverChanged();
}

void WSpinnyBase::slotTrackCoverArtUpdated() {
    if (m_pLoadedTrack) {
        CoverArtCache::requestTrackCover(this, m_pLoadedTrack);
    }
}

void WSpinnyBase::slotCoverFound(
        const QObject* pRequester,
        const CoverInfo& coverInfo,
        const QPixmap& pixmap) {
    if (pRequester == this &&
            m_pLoadedTrack &&
            m_pLoadedTrack->getLocation() == coverInfo.trackLocation) {
        setLoadedCover(pixmap);
        coverChanged();
    }
}

void WSpinnyBase::slotCoverInfoSelected(const CoverInfoRelative& coverInfo) {
    if (m_pLoadedTrack != nullptr) {
        // Will trigger slotTrackCoverArtUpdated().
        m_pLoadedTrack->setCoverInfo(coverInfo);
    }
}

void WSpinnyBase::slotReloadCoverArt() {
    if (!m_pLoadedTrack) {
        return;
    }
    const auto future = guessTrackCoverInfoConcurrently(m_pLoadedTrack);
    // Don't wait for the result and keep running in the background
    Q_UNUSED(future)
}

void WSpinnyBase::paintEvent(QPaintEvent* e) {
    Q_UNUSED(e);
}

void WSpinnyBase::render(VSyncThread* vSyncThread) {
    if (!shouldRender()) {
        return;
    }

    if (!m_pVisualPlayPos.isNull() && vSyncThread != nullptr) {
        m_pVisualPlayPos->getPlaySlipAtNextVSync(
                vSyncThread,
                &m_dAngleCurrentPlaypos,
                &m_dGhostAngleCurrentPlaypos);
    }

    if (m_dAngleCurrentPlaypos != m_dAngleLastPlaypos) {
        m_fAngle = static_cast<float>(calculateAngle(m_dAngleCurrentPlaypos));
        m_dAngleLastPlaypos = m_dAngleCurrentPlaypos;
    }

    if (m_dGhostAngleCurrentPlaypos != m_dGhostAngleLastPlaypos) {
        m_fGhostAngle = static_cast<float>(calculateAngle(m_dGhostAngleCurrentPlaypos));
        m_dGhostAngleLastPlaypos = m_dGhostAngleCurrentPlaypos;
    }

    draw();
}

void WSpinnyBase::swap() {
    if (!shouldRender()) {
        return;
    }
    makeCurrentIfNeeded();
    swapBuffers();
    doneCurrent();
}

QImage WSpinnyBase::scaleToSize(const QImage& image) const {
    if (image.isNull()) {
        return QImage();
    }
    QImage scaled = image.scaled(size() * devicePixelRatioF(),
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation);
    scaled.setDevicePixelRatio(devicePixelRatioF());
    return scaled;
}

QImage WSpinnyBase::scaleToSize(const std::shared_ptr<QImage>& image) const {
    return image ? scaleToSize(*image) : QImage();
}

QPixmap WSpinnyBase::scaleToSize(const QPixmap& pixmap) const {
    if (pixmap.isNull()) {
        return QPixmap();
    }
    QPixmap scaled = pixmap.scaled(size() * devicePixelRatioF(),
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation);
    scaled.setDevicePixelRatio(devicePixelRatioF());
    return scaled;
}

void WSpinnyBase::resizeEvent(QResizeEvent* event) {
    m_loadedCoverScaled = scaleToSize(m_loadedCover);
    m_fgImageScaled = scaleToSize(m_pFgImage);
    m_ghostImageScaled = scaleToSize(m_pGhostImage);

    WGLWidget::resizeEvent(event);
}

/* Convert between a normalized playback position (0.0 - 1.0) and an angle
   in our polar coordinate system.
   Returns an angle clamped between -180 and 180 degrees. */
double WSpinnyBase::calculateAngle(double playpos) {
    double trackFrames = m_pTrackSamples->get() / 2;
    double trackSampleRate = m_pTrackSampleRate->get();
    if (util_isnan(playpos) || util_isnan(trackFrames) || util_isnan(trackSampleRate) ||
            trackFrames <= 0 || trackSampleRate <= 0) {
        return 0.0;
    }

    // Convert playpos to seconds.
    double t = playpos * trackFrames / trackSampleRate;

    // Bad samplerate or number of track samples.
    if (util_isnan(t)) {
        return 0.0;
    }

    // 33 RPM is approx. 0.5 rotations per second.
    double angle = 360.0 * m_dRotationsPerSecond * t;
    // Clamp within -180 and 180 degrees
    // qDebug() << "pc:" << angle;
    // angle = ((angle + 180) % 360.) - 180;
    // modulo for doubles :)
    const double originalAngle = angle;
    if (angle > 0) {
        const auto x = static_cast<int>((angle + 180) / 360);
        angle = angle - (360 * x);
    } else {
        const auto x = static_cast<int>((angle - 180) / 360);
        angle = angle - (360 * x);
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
int WSpinnyBase::calculateFullRotations(double playpos) {
    if (util_isnan(playpos)) {
        return 0;
    }
    // Convert playpos to seconds.
    double t = playpos * (m_pTrackSamples->get() / 2 / // Stereo audio!
                                 m_pTrackSampleRate->get());

    // 33 RPM is approx. 0.5 rotations per second.
    // qDebug() << t;
    double angle = 360 * m_dRotationsPerSecond * t;

    return ((static_cast<int>(angle) + 180) / 360);
}

// Inverse of calculateAngle()
double WSpinnyBase::calculatePositionFromAngle(double angle) {
    if (util_isnan(angle)) {
        return 0.0;
    }

    // 33 RPM is approx. 0.5 rotations per second.
    double t = angle / (360.0 * m_dRotationsPerSecond); // time in seconds

    double trackFrames = m_pTrackSamples->get() / 2;
    double trackSampleRate = m_pTrackSampleRate->get();
    if (util_isnan(trackFrames) || util_isnan(trackSampleRate) ||
            trackFrames <= 0 || trackSampleRate <= 0) {
        return 0.0;
    }

    // Convert t from seconds into a normalized playposition value.
    double playpos = t * trackSampleRate / trackFrames;
    if (util_isnan(playpos)) {
        return 0.0;
    }
    return playpos;
}

void WSpinnyBase::updateVinylControlSpeed(double rpm) {
    m_dRotationsPerSecond = rpm / 60.;
}

void WSpinnyBase::updateVinylControlSignalEnabled(double enabled) {
#ifdef __VINYLCONTROL__
    if (m_pVCManager == nullptr) {
        return;
    }
    m_bSignalActive = enabled != 0;

    if (m_bSignalActive && m_iVinylInput != -1) {
        m_pVCManager->addSignalQualityListener(this);
    } else {
        m_pVCManager->removeSignalQualityListener(this);
        m_bDrawVinylSignalQuality = false;
    }
#else
    Q_UNUSED(enabled);
#endif
}

void WSpinnyBase::updateVinylControlEnabled(double enabled) {
    m_bVinylActive = enabled != 0;
}

void WSpinnyBase::updateSlipEnabled(double enabled) {
    m_bGhostPlayback = static_cast<bool>(enabled);
}

void WSpinnyBase::mouseMoveEvent(QMouseEvent* e) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    int y = static_cast<int>(e->position().y());
    int x = static_cast<int>(e->position().x());
#else
    int y = e->y();
    int x = e->x();
#endif

    // Keeping these around in case we want to switch to control relative
    // to the original mouse position.
    // int dX = x-m_iStartMouseX;
    // int dY = y-m_iStartMouseY;

    // Coordinates from center of widget
    double c_x = x - width() / 2;
    double c_y = y - height() / 2;
    double theta = (180.0 / M_PI) * atan2(c_x, -c_y);

    // qDebug() << "c_x:" << c_x << "c_y:" << c_y <<
    //             "dX:" << dX << "dY:" << dY;

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
    theta += m_iFullRotations * 360;

    // qDebug() << "c t:" << theta << "pt:" << m_dPrevTheta <<
    //             "icr" << m_iFullRotations;

    if (((e->buttons() & Qt::LeftButton) || (e->buttons() & Qt::RightButton)) &&
            !m_bVinylActive) {
        // Convert deltaTheta into a percentage of song length.
        double absPos = calculatePositionFromAngle(theta);
        double absPosInSamples = absPos * m_pTrackSamples->get();
        m_pScratchPos->set(absPosInSamples - m_dInitialPos);
    } else if (e->buttons() & Qt::MiddleButton) {
    } else if (e->buttons() & Qt::NoButton) {
        setCursor(QCursor(Qt::OpenHandCursor));
    }
}

void WSpinnyBase::mousePressEvent(QMouseEvent* e) {
    if (m_pLoadedTrack == nullptr) {
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
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        int y = static_cast<int>(e->position().y());
        int x = static_cast<int>(e->position().x());
#else
        int y = e->y();
        int x = e->x();
#endif

        m_iStartMouseX = x;
        m_iStartMouseY = y;

        // don't do anything if vinyl control is active
        if (m_bVinylActive) {
            return;
        }

        if (e->button() == Qt::LeftButton || e->button() == Qt::RightButton) {
            QApplication::setOverrideCursor(QCursor(Qt::ClosedHandCursor));

            // Coordinates from center of widget
            double c_x = x - width() / 2;
            double c_y = y - height() / 2;
            double theta = (180.0 / M_PI) * atan2(c_x, -c_y);
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
            m_pDlgCoverArt->init(m_pLoadedTrack);
        } else if (!m_pDlgCoverArt->isVisible() && m_bShowCover) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            m_pCoverMenu->popup(e->globalPosition().toPoint());
#else
            m_pCoverMenu->popup(e->globalPos());
#endif
        }
    }
}

void WSpinnyBase::mouseReleaseEvent(QMouseEvent* e) {
    if (e->button() == Qt::LeftButton || e->button() == Qt::RightButton) {
        QApplication::restoreOverrideCursor();
        m_pScratchToggle->set(0.0);
        m_iFullRotations = 0;
    }
}

void WSpinnyBase::showEvent(QShowEvent* event) {
    Q_UNUSED(event);
    WGLWidget::showEvent(event);
#ifdef __VINYLCONTROL__
    // If we want to draw the VC signal on this widget then register for
    // updates.
    if (m_bSignalActive && m_iVinylInput != -1 && m_pVCManager) {
        m_pVCManager->addSignalQualityListener(this);
    }
#endif
    WGLWidget::showEvent(event);
}

void WSpinnyBase::hideEvent(QHideEvent* event) {
    Q_UNUSED(event);
#ifdef __VINYLCONTROL__
    // When we are hidden we do not want signal quality updates.
    if (m_pVCManager) {
        m_pVCManager->removeSignalQualityListener(this);
    }
#endif
    m_bDrawVinylSignalQuality = false;
}

bool WSpinnyBase::event(QEvent* pEvent) {
    if (pEvent->type() == QEvent::ToolTip) {
        updateTooltip();
    }
    return WGLWidget::event(pEvent);
}

bool WSpinnyBase::handleDragAndDropEventFromWindow(QEvent* pEvent) {
    return event(pEvent);
}

void WSpinnyBase::dragEnterEvent(QDragEnterEvent* pEvent) {
    DragAndDropHelper::handleTrackDragEnterEvent(pEvent, m_group, m_pConfig);
}

void WSpinnyBase::dropEvent(QDropEvent* pEvent) {
    DragAndDropHelper::handleTrackDropEvent(pEvent, *this, m_group, m_pConfig);
}
