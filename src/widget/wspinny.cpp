#define _USE_MATH_DEFINES
#include <cmath>

#include <QtDebug>
#include <QApplication>
#include <QUrl>
#include <QMimeData>

#include "mathstuff.h"
#include "wimagestore.h"
#include "controlobject.h"
#include "controlobjectthread.h"
#include "sharedglcontext.h"
#include "visualplayposition.h"
#include "widget/wspinny.h"
#include "vinylcontrol/vinylcontrolmanager.h"
#include "vinylcontrol/vinylcontrol.h"
#include "util/dnd.h"

WSpinny::WSpinny(QWidget* parent, VinylControlManager* pVCMan)
        : QGLWidget(parent, SharedGLContext::getWidget()),
          WBaseWidget(this),
          m_pBgImage(NULL),
          m_pFgImage(NULL),
          m_pGhostImage(NULL),
          m_pPlay(NULL),
          m_pPlayPos(NULL),
          m_pVisualPlayPos(NULL),
          m_pTrackSamples(NULL),
          m_pScratch(NULL),
          m_pScratchToggle(NULL),
          m_pScratchPos(NULL),
          m_pVinylControlSpeedType(NULL),
          m_pVinylControlEnabled(NULL),
          m_iVinylInput(-1),
          m_bVinylActive(false),
          m_bSignalActive(true),
          m_iVinylScopeSize(0),
          m_fAngle(0.0f),
          m_dAngleLastPlaypos(-1),
          m_fGhostAngle(0.0f),
          m_dGhostAngleLastPlaypos(-1),
          m_iStartMouseX(-1),
          m_iStartMouseY(-1),
          m_iFullRotations(0),
          m_dPrevTheta(0.),
          m_bClampFailedWarning(false) {
#ifdef __VINYLCONTROL__
    m_pVCManager = pVCMan;
#endif
    //Drag and drop
    setAcceptDrops(true);
    qDebug() << "WSpinny(): Created QGLWidget, Context"
             << "Valid:" << context()->isValid()
             << "Sharing:" << context()->isSharing();
}

WSpinny::~WSpinny() {
#ifdef __VINYLCONTROL__
    m_pVCManager->removeSignalQualityListener(this);
#endif
    // No need to delete anything if m_group is empty because setup() was not called.
    if (!m_group.isEmpty()) {
        WImageStore::deleteImage(m_pBgImage);
        WImageStore::deleteImage(m_pFgImage);
        WImageStore::deleteImage(m_pGhostImage);
        delete m_pPlay;
        delete m_pPlayPos;
        delete m_pTrackSamples;
        delete m_pTrackSampleRate;
        delete m_pScratch;
        delete m_pScratchToggle;
        delete m_pScratchPos;
        delete m_pSlipEnabled;
        delete m_pSlipPosition;
    #ifdef __VINYLCONTROL__
        delete m_pVinylControlSpeedType;
        delete m_pVinylControlEnabled;
        delete m_pSignalEnabled;
    #endif
    }
}

void WSpinny::onVinylSignalQualityUpdate(const VinylSignalQualityReport& report) {
#ifdef __VINYLCONTROL__
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
    qual_color.setHsv((int)(120.0 * signalQuality), 255, 255);
    qual_color.getRgb(&r, &g, &b);

    for (int y = 0; y < m_iVinylScopeSize; ++y) {
        QRgb *line = (QRgb *)m_qImage.scanLine(y);
        for (int x = 0; x < m_iVinylScopeSize; ++x) {
            // use xwax's bitmap to set alpha data only
            // adjust alpha by 3/4 so it's not quite so distracting
            // setpixel is slow, use scanlines instead
            //m_qImage.setPixel(x, y, qRgba(r,g,b,(int)buf[x+m_iVinylScopeSize*y] * .75));
            *line = qRgba(r,g,b,(int)(report.scope[x+m_iVinylScopeSize*y] * .75));
            line++;
        }
    }
#endif
}

void WSpinny::setup(QDomNode node, const SkinContext& context, QString group) {
    m_group = group;

    // Set images
    m_pBgImage = WImageStore::getImage(context.getSkinPath(context.selectString(node,
                                                    "PathBackground")));
    m_pFgImage = WImageStore::getImage(context.getSkinPath(context.selectString(node,
                                                    "PathForeground")));
    m_pGhostImage = WImageStore::getImage(context.getSkinPath(context.selectString(node,
                                                    "PathGhost")));
    if (m_pBgImage && !m_pBgImage->isNull()) {
        setFixedSize(m_pBgImage->size());
    }

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

    m_pPlay = new ControlObjectThread(
            group, "play");
    m_pPlayPos = new ControlObjectThread(
            group, "playposition");
    m_pVisualPlayPos = VisualPlayPosition::getVisualPlayPosition(group);

    m_pTrackSamples = new ControlObjectThread(
            group, "track_samples");
    m_pTrackSampleRate = new ControlObjectThread(
            group, "track_samplerate");

    m_pScratch = new ControlObjectThread(
            group, "scratch2");
    m_pScratchToggle = new ControlObjectThread(
            group, "scratch_position_enable");
    m_pScratchPos = new ControlObjectThread(
            group, "scratch_position");

    m_pSlipEnabled = new ControlObjectThread(
            group, "slip_enabled");
    m_pSlipPosition = new ControlObjectThread(
            group, "slip_playposition");

#ifdef __VINYLCONTROL__
    m_pVinylControlSpeedType = new ControlObjectThread(
            group, "vinylcontrol_speed_type");
    if (m_pVinylControlSpeedType)
    {
        //Initialize the rotational speed.
        this->updateVinylControlSpeed(m_pVinylControlSpeedType->get());
    }

    m_pVinylControlEnabled = new ControlObjectThread(
            group, "vinylcontrol_enabled");
    connect(m_pVinylControlEnabled, SIGNAL(valueChanged(double)),
            this, SLOT(updateVinylControlEnabled(double)));

    m_pSignalEnabled = new ControlObjectThread(
            group, "vinylcontrol_signal_enabled");
    connect(m_pSignalEnabled, SIGNAL(valueChanged(double)),
            this, SLOT(updateVinylControlSignalEnabled(double)));

    //Match the vinyl control's set RPM so that the spinny widget rotates at the same
    //speed as your physical decks, if you're using vinyl control.
    connect(m_pVinylControlSpeedType, SIGNAL(valueChanged(double)),
            this, SLOT(updateVinylControlSpeed(double)));



#else
    //if no vinyl control, just call it 33
    this->updateVinylControlSpeed(33.0);
#endif
}

void WSpinny::paintEvent(QPaintEvent *e) {
    Q_UNUSED(e); //ditch unused param warning

    QPainter p(this);
    p.setRenderHint(QPainter::SmoothPixmapTransform);

    if (m_pBgImage) {
        p.drawImage(0, 0, *m_pBgImage);
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

    bool bGhostPlayback = m_pSlipEnabled->get();

    if (bGhostPlayback) {
        p.save();
    }

    double playPosition = -1;
    double slipPosition = -1;
    m_pVisualPlayPos->getPlaySlipAt(0, &playPosition, &slipPosition);

    if (playPosition != m_dAngleLastPlaypos) {
        m_fAngle = calculateAngle(playPosition);
        m_dAngleLastPlaypos = playPosition;
    }

    if (slipPosition != m_dGhostAngleLastPlaypos) {
        m_fGhostAngle = calculateAngle(slipPosition);
        m_dGhostAngleLastPlaypos = slipPosition;
    }

    if (m_pFgImage && !m_pFgImage->isNull()) {
        // Now rotate the image and draw it on the screen.
        p.rotate(m_fAngle);
        p.drawImage(-(m_pFgImage->width() / 2),
                -(m_pFgImage->height() / 2), *m_pFgImage);
    }

    if (bGhostPlayback && m_pGhostImage && !m_pGhostImage->isNull()) {
        p.restore();
        p.save();
        p.rotate(m_fGhostAngle);
        p.drawImage(-(m_pGhostImage->width() / 2),
                -(m_pGhostImage->height() / 2), *m_pGhostImage);

        //Rotate back to the playback position (not the ghost positon),
        //and draw the beat marks from there.
        p.restore();
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
int WSpinny::calculateFullRotations(double playpos)
{
    if (isnan(playpos))
        return 0;
    //Convert playpos to seconds.
    double t = playpos * (m_pTrackSamples->get() / 2 /  // Stereo audio!
                          m_pTrackSampleRate->get());

    //33 RPM is approx. 0.5 rotations per second.
    //qDebug() << t;
    double angle = 360 * m_dRotationsPerSecond * t;

    return (((int)angle + 180) / 360);
}

//Inverse of calculateAngle()
double WSpinny::calculatePositionFromAngle(double angle)
{
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
    if (m_pVCManager == NULL) {
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

void WSpinny::mouseMoveEvent(QMouseEvent * e) {
    int y = e->y();
    int x = e->x();

    //Keeping these around in case we want to switch to control relative
    //to the original mouse position.
    //int dX = x-m_iStartMouseX;
    //int dY = y-m_iStartMouseY;

    //Coordinates from center of widget
    double c_x = x - width()/2;
    double c_y = y - height()/2;
    double theta = (180.0/M_PI)*atan2(c_x, -c_y);

    //qDebug() << "c_x:" << c_x << "c_y:" << c_y <<
    //            "dX:" << dX << "dY:" << dY;

    //When we finish one full rotation (clockwise or anticlockwise),
    //we'll need to manually add/sub 360 degrees because atan2()'s range is
    //only within -180 to 180 degrees. We need a wider range so your position
    //in the song can be tracked.
    if (m_dPrevTheta > 100 && theta < 0) {
        m_iFullRotations++;
    } else if (m_dPrevTheta < -100 && theta > 0) {
        m_iFullRotations--;
    }

    m_dPrevTheta = theta;
    theta += m_iFullRotations*360;

    //qDebug() << "c t:" << theta << "pt:" << m_dPrevTheta <<
    //            "icr" << m_iFullRotations;

    if ((e->buttons() & Qt::LeftButton || e->buttons() & Qt::RightButton) && !m_bVinylActive) {
        //Convert deltaTheta into a percentage of song length.
        double absPos = calculatePositionFromAngle(theta);
        double absPosInSamples = absPos * m_pTrackSamples->get();
        m_pScratchPos->slotSet(absPosInSamples - m_dInitialPos);
    } else if (e->buttons() & Qt::MidButton) {
    } else if (e->buttons() & Qt::NoButton) {
        setCursor(QCursor(Qt::OpenHandCursor));
    }
}

void WSpinny::mousePressEvent(QMouseEvent * e)
{
    int y = e->y();
    int x = e->x();

    m_iStartMouseX = x;
    m_iStartMouseY = y;

    //don't do anything if vinyl control is active
    if (m_bVinylActive)
        return;

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

        m_pScratchPos->slotSet(0);
        m_pScratchToggle->slotSet(1.0);

        if (e->button() == Qt::RightButton) {
            m_pSlipEnabled->slotSet(1.0);
        }

        // Trigger a mouse move to immediately line up the vinyl with the cursor
        mouseMoveEvent(e);
    }
}

void WSpinny::mouseReleaseEvent(QMouseEvent * e)
{
    if (e->button() == Qt::LeftButton || e->button() == Qt::RightButton) {
        QApplication::restoreOverrideCursor();
        m_pScratchToggle->slotSet(0.0);
        m_iFullRotations = 0;
        if (e->button() == Qt::RightButton) {
            m_pSlipEnabled->slotSet(0.0);
        }
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

/** DRAG AND DROP **/
void WSpinny::dragEnterEvent(QDragEnterEvent * event)
{
    // Accept the enter event if the thing is a filepath and nothing's playing
    // in this deck.
    if (event->mimeData()->hasUrls()) {
        if (m_pPlay && m_pPlay->get()) {
            event->ignore();
        } else {
            event->acceptProposedAction();
        }
    }
}

void WSpinny::dropEvent(QDropEvent * event) {
    if (event->mimeData()->hasUrls()) {
        QList<QFileInfo> files = DragAndDropHelper::supportedTracksFromUrls(
                event->mimeData()->urls(), true, false);
        if (!files.isEmpty()) {
            event->accept();
            emit(trackDropped(files.at(0).canonicalFilePath(), m_group));
            return;
        }
    }
    event->ignore();
}
