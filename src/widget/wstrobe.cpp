#include "widget/wstrobe.h"

#include <QApplication>
#include <QMimeData>
#include <QStylePainter>
#include <QUrl>
#include <QWindow>
#include <QtDebug>

#include "control/controlobject.h"
#include "control/controlproxy.h"
#include "library/coverartcache.h"
#include "moc_wstrobe.cpp"
#include "skin/legacy/skincontext.h"
#include "util/dnd.h"
#include "util/math.h"
#include "vinylcontrol/vinylcontrol.h"
#include "vinylcontrol/vinylcontrolmanager.h"
#include "waveform/sharedglcontext.h"
#include "waveform/visualplayposition.h"
#include "waveform/vsyncthread.h"
#include "wimagestore.h"

namespace {

constexpr double kBbigDotHeightFactor = 0.34;
constexpr double kBigDotWidthFactor = 0.40;
constexpr double kSmallDotHeightFactor = 0.23;
// The dots per round are taken from a real turn table
constexpr int kDotsPerRound60Pos = 203; // 93.98 %
constexpr int kDotsPerRound33Pos = 209; // 96.76 %
constexpr int kDotsPerRoundUnity = 216; // 100.00 %
constexpr int kDotsPerRound33Neg = 223; // 103.24 %

} // anonymous namespace

WStrobe::WStrobe(
        QWidget* pParent,
        const QString& group,
        UserSettingsPointer pConfig,
        BaseTrackPlayer* pPlayer)
        : WGLWidget(pParent),
          WBaseWidget(this),
          m_group(group),
          m_pConfig(pConfig),
          m_pPlay(nullptr),
          m_pPlayPos(nullptr),
          m_pVisualPlayPos(nullptr),
          m_pTrackSamples(nullptr),
          m_pTrackSampleRate(nullptr),
          m_pScratchToggle(nullptr),
          m_pScratchPos(nullptr),
          m_pSignalEnabled(nullptr),
          m_dInitialPos(0.),
          m_iVinylInput(-1),
          m_bVinylActive(false),
          m_bSignalActive(true),
          m_iVinylScopeSize(0),
          m_fAngle(0.0f),
          m_dAngleCurrentPlaypos(-1),
          m_dGhostAngleCurrentPlaypos(-1),
          m_dAngleLastPlaypos(-1),
          m_iStartMouseX(-1),
          m_iStartMouseY(-1),
          m_iFullRotations(0),
          m_dPrevTheta(0.),
          m_dTheta(0.),
          m_dRotationsPerSecond(100.0 / 3 / 60),
          m_bClampFailedWarning(false),
          m_pPlayer(pPlayer) {
    // Drag and drop
    setAcceptDrops(true);

    if (m_pPlayer != nullptr) {
        connect(m_pPlayer,
                &BaseTrackPlayer::newTrackLoaded,
                this,
                &WStrobe::slotLoadTrack);
        connect(m_pPlayer,
                &BaseTrackPlayer::loadingTrack,
                this,
                &WStrobe::slotLoadingTrack);
        // just in case a track is already loaded
        slotLoadTrack(m_pPlayer->getLoadedTrack());
    }

    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_OpaquePaintEvent);

#ifdef MIXXX_USE_QOPENGL
    setTrackDropTarget(this);
#endif
}

WStrobe::~WStrobe() {
}

void WStrobe::setup(const QDomNode& node, const SkinContext& context) {
    // Set images
    m_pDot60PosImage = WImageStore::getImage(
            context.getPixmapSource(context.selectNode(node, "PathDot60Pos")),
            context.getScaleFactor());
    m_pDot33PosImage = WImageStore::getImage(
            context.getPixmapSource(context.selectNode(node, "PathDot33Pos")),
            context.getScaleFactor());
    m_pDotUnityImage = WImageStore::getImage(
            context.getPixmapSource(context.selectNode(node, "PathDotUnity")),
            context.getScaleFactor());
    m_pDot33NegImage = WImageStore::getImage(
            context.getPixmapSource(context.selectNode(node, "PathDot33Neg")),
            context.getScaleFactor());

    m_pPlay = new ControlProxy(
            m_group, "play", this);
    m_pPlayPos = new ControlProxy(
            m_group, "playposition", this);
    m_pVisualPlayPos = VisualPlayPosition::getVisualPlayPosition(m_group);
    m_pTrackSamples = new ControlProxy(
            m_group, "track_samples", this);
    m_pTrackSampleRate = new ControlProxy(
            m_group, "track_samplerate", this);

    m_pScratchToggle = new ControlProxy(
            m_group, "scratch_position_enable", this);
    m_pScratchPos = new ControlProxy(
            m_group, "scratch_position", this);
}

void WStrobe::slotLoadTrack(TrackPointer pTrack) {
    m_loadedTrack = pTrack;
}

void WStrobe::slotLoadingTrack(TrackPointer pNewTrack, TrackPointer pOldTrack) {
    Q_UNUSED(pNewTrack);
    Q_UNUSED(pOldTrack);
    m_loadedTrack.reset();
    update();
}

void WStrobe::paintEvent(QPaintEvent* e) {
    Q_UNUSED(e);
}

void WStrobe::render(VSyncThread* vSyncThread) {
    if (!shouldRender()) {
        return;
    }

    if (!m_pVisualPlayPos.isNull() && vSyncThread) {
        m_pVisualPlayPos->getPlaySlipAtNextVSync(
                vSyncThread,
                &m_dAngleCurrentPlaypos,
                &m_dGhostAngleCurrentPlaypos);
    }

    if (m_dAngleCurrentPlaypos != m_dAngleLastPlaypos) {
        m_fAngle = calculateAngle(m_dAngleCurrentPlaypos);
        m_dAngleLastPlaypos = m_dAngleCurrentPlaypos;
    }

    qDebug() << m_fAngle << m_dAngleCurrentPlaypos;

    QPainter p(paintDevice());
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);

    double smallHight = height() * kSmallDotHeightFactor;
    double bigHight = height() * kBbigDotHeightFactor;
    double dot60PosWidth = height() *
            kBigDotWidthFactor /
            kDotsPerRoundUnity *
            kDotsPerRound60Pos;
    double dot33PosWidth = height() *
            kBigDotWidthFactor /
            kDotsPerRoundUnity *
            kDotsPerRound33Pos;
    double dotUnityWidth = height() *
            kBigDotWidthFactor /
            kDotsPerRoundUnity *
            kDotsPerRoundUnity;
    double dot33NegWidth = height() *
            kBigDotWidthFactor /
            kDotsPerRoundUnity *
            kDotsPerRound33Neg;

    double maxWidth = width();

    qDebug() << m_fAngle;

    // Calc how much of the left most dot is visible
    double left = m_fAngle / 360 * kDotsPerRound60Pos;
    left -= floor(left);
    left = -1 - left;
    left *= dot60PosWidth;
    while (left < maxWidth) {
        if (m_pDot60PosImage) {
            p.drawImage(QRectF(left, 0.0, dot60PosWidth, smallHight),
                    *m_pDot60PosImage,
                    QRectF(m_pDot60PosImage->rect()));
        }
        left += dot60PosWidth;
    }

    // Calc how much of the left most dot is visible
    left = m_fAngle / 360 * kDotsPerRound33Pos;
    left -= floor(left);
    left = -1 - left;
    left *= dot33PosWidth;
    // left = calculateOffset(m_dAngleCurrentPlaypos, 1.033) * dot33PosWidth;
    while (left < maxWidth) {
        if (m_pDot33PosImage) {
            p.drawImage(QRectF(left, smallHight, dot33PosWidth, smallHight),
                    *m_pDot33PosImage,
                    QRectF(m_pDot33PosImage->rect()));
        }
        left += dot33PosWidth;
    }

    // Calc how much of the left most dot is visible
    left = m_fAngle / 360 * kDotsPerRoundUnity;
    left -= floor(left);
    left = -1 - left;
    left *= dotUnityWidth;
    // left = calculateOffset(m_dAngleCurrentPlaypos, 1) * dotUnityWidth;
    while (left < maxWidth) {
        if (m_pDotUnityImage) {
            p.drawImage(QRectF(left, smallHight * 2, dotUnityWidth, bigHight),
                    *m_pDotUnityImage,
                    QRectF(m_pDotUnityImage->rect()));
        }
        left += dotUnityWidth;
    }

    // Calc how much of the left most dot is visible
    left = m_fAngle / 360 * kDotsPerRound33Neg;
    left -= floor(left);
    left = -1 - left;
    left *= dot33NegWidth;
    // left = calculateOffset(m_dAngleCurrentPlaypos, 0.967) * dot33NegWidth;
    while (left < maxWidth) {
        if (m_pDot33NegImage) {
            p.drawImage(QRectF(left, smallHight * 2 + bigHight, dot33NegWidth, smallHight),
                    *m_pDot33NegImage,
                    QRectF(m_pDot33NegImage->rect()));
        }
        left += dot33NegWidth;
    }
}

void WStrobe::swap() {
    if (!shouldRender()) {
        return;
    }
    makeCurrentIfNeeded();
    swapBuffers();
    doneCurrent();
}

double WStrobe::calculateAngle(double playpos) {
    // Convert between a normalized playback position (0.0 - 1.0) and an angle
    // in our polar coordinate system.
    // Returns an angle clamped between -180 and 180 degrees.
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
        int x = static_cast<int>((angle + 180) / 360);
        angle = angle - (360 * x);
    } else {
        int x = static_cast<int>((angle - 180) / 360);
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

void WStrobe::showEvent(QShowEvent* event) {
    WGLWidget::showEvent(event);
}

void WStrobe::hideEvent(QHideEvent* event) {
    Q_UNUSED(event);
    // fill with transparent black
    m_qImage.fill(qRgba(0, 0, 0, 0));
    WGLWidget::hideEvent(event);
}

bool WStrobe::event(QEvent* pEvent) {
    if (pEvent->type() == QEvent::ToolTip) {
        updateTooltip();
    }
    return WGLWidget::event(pEvent);
}

void WStrobe::dragEnterEvent(QDragEnterEvent* event) {
    DragAndDropHelper::handleTrackDragEnterEvent(event, m_group, m_pConfig);
}

void WStrobe::dropEvent(QDropEvent* event) {
    DragAndDropHelper::handleTrackDropEvent(event, *this, m_group, m_pConfig);
}
