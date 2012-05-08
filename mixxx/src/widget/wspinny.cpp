#include <math.h>
#include "mathstuff.h"
#include "wpixmapstore.h"
#include "controlobject.h"
#include "controlobjectthreadmain.h"
#include "sharedglcontext.h"
#include "wspinny.h"

WSpinny::WSpinny(QWidget* parent, VinylControlManager* pVCMan)
        : QGLWidget(SharedGLContext::getContext(), parent),
          m_pBG(NULL),
          m_pFG(NULL),
          m_pGhost(NULL),
          m_pPlay(NULL),
          m_pPlayPos(NULL),
          m_pVisualPlayPos(NULL),
          m_pDuration(NULL),
          m_pTrackSamples(NULL),
          m_pBPM(NULL),
          m_pScratch(NULL),
          m_pScratchToggle(NULL),
          m_pScratchPos(NULL),
          m_pVinylControlSpeedType(NULL),
          m_pVinylControlEnabled(NULL),
          m_bVinylActive(false),
          m_bSignalActive(true),
          m_iSize(0),
          m_iSignalUpdateTick(0),
          m_fAngle(0.0f),
          m_fGhostAngle(0.0f),
          m_dPausedPosition(0.0f),
          m_bGhostPlayback(false),
          m_iStartMouseX(-1),
          m_iStartMouseY(-1),
          m_iFullRotations(0),
          m_dPrevTheta(0.) {
#ifdef __VINYLCONTROL__
    m_pVCManager = pVCMan;
    m_pVinylControl = NULL;
#endif
    //Drag and drop
    setAcceptDrops(true);
}

WSpinny::~WSpinny()
{
    //Don't delete these because the pixmap store takes care of them.
    //delete m_pBG;
    //delete m_pFG;
    //delete m_pGhost;
    WPixmapStore::deletePixmap(m_pBG);
    WPixmapStore::deletePixmap(m_pFG);
    WPixmapStore::deletePixmap(m_pGhost);
    delete m_pPlay;
    delete m_pPlayPos;
    delete m_pVisualPlayPos;
    delete m_pDuration;
    delete m_pTrackSamples;
    delete m_pTrackSampleRate;
    delete m_pBPM;
    delete m_pScratch;
    delete m_pScratchToggle;
    delete m_pScratchPos;
#ifdef __VINYLCONTROL__
    delete m_pVinylControlSpeedType;
    delete m_pVinylControlEnabled;
    delete m_pSignalEnabled;
    delete m_pRate;
#endif

}

void WSpinny::setup(QDomNode node, QString group)
{
    m_group = group;

    // Set pixmaps
    m_pBG = WPixmapStore::getPixmap(WWidget::getPath(WWidget::selectNodeQString(node,
                                                    "PathBackground")));
    m_pFG = WPixmapStore::getPixmap(WWidget::getPath(WWidget::selectNodeQString(node,
                                                    "PathForeground")));
    m_pGhost = WPixmapStore::getPixmap(WWidget::getPath(WWidget::selectNodeQString(node,
                                                    "PathGhost")));
    if (m_pBG && !m_pBG->isNull()) {
        setFixedSize(m_pBG->size());
    }

#ifdef __VINYLCONTROL__
    m_iSize = MIXXX_VINYL_SCOPE_SIZE;
    m_qImage = QImage(m_iSize, m_iSize, QImage::Format_ARGB32);
    //fill with transparent black
    m_qImage.fill(qRgba(0,0,0,0));
#endif

    m_pPlay = new ControlObjectThreadMain(ControlObject::getControl(
                        ConfigKey(group, "play")));
    m_pPlayPos = new ControlObjectThreadMain(ControlObject::getControl(
                        ConfigKey(group, "playposition")));
    m_pVisualPlayPos = new ControlObjectThreadMain(ControlObject::getControl(
                        ConfigKey(group, "visual_playposition")));
    m_pDuration = new ControlObjectThreadMain(ControlObject::getControl(
                        ConfigKey(group, "duration")));
    m_pTrackSamples = new ControlObjectThreadMain(ControlObject::getControl(
                        ConfigKey(group, "track_samples")));
    m_pTrackSampleRate = new ControlObjectThreadMain(
                                    ControlObject::getControl(
                                    ConfigKey(group, "track_samplerate")));
    m_pBPM = new ControlObjectThreadMain(ControlObject::getControl(
                        ConfigKey(group, "bpm")));

    m_pScratch = new ControlObjectThreadMain(ControlObject::getControl(
                        ConfigKey(group, "scratch2")));
    m_pScratchToggle = new ControlObjectThreadMain(ControlObject::getControl(
                        ConfigKey(group, "scratch_position_enable")));
    m_pScratchPos = new ControlObjectThreadMain(ControlObject::getControl(
                        ConfigKey(group, "scratch_position")));

    Q_ASSERT(m_pPlayPos);
    Q_ASSERT(m_pDuration);

    //Repaint when visual_playposition changes.
    connect(m_pVisualPlayPos, SIGNAL(valueChanged(double)),
            this, SLOT(updateAngle(double)));

#ifdef __VINYLCONTROL__
    m_pVinylControlSpeedType = new ControlObjectThreadMain(ControlObject::getControl(
                        ConfigKey(group, "vinylcontrol_speed_type")));
    if (m_pVinylControlSpeedType)
    {
        //Initialize the rotational speed.
        this->updateVinylControlSpeed(m_pVinylControlSpeedType->get());
    }
    m_pVinylControlEnabled = new ControlObjectThreadMain(ControlObject::getControl(
                        ConfigKey(group, "vinylcontrol_enabled")));
    m_pSignalEnabled = new ControlObjectThreadMain(ControlObject::getControl(
                        ConfigKey(group, "vinylcontrol_signal_enabled")));
    m_pRate = new ControlObjectThreadMain(ControlObject::getControl(
                        ConfigKey(group, "rate")));

    //Match the vinyl control's set RPM so that the spinny widget rotates at the same
    //speed as your physical decks, if you're using vinyl control.
    connect(m_pVinylControlSpeedType, SIGNAL(valueChanged(double)),
            this, SLOT(updateVinylControlSpeed(double)));

    //Make sure vinyl control proxies are up to date
    connect(m_pVinylControlEnabled, SIGNAL(valueChanged(double)),
            this, SLOT(updateVinylControlEnabled(double)));

    //Check the rate to see if we are stopped
    connect(m_pRate, SIGNAL(valueChanged(double)),
            this, SLOT(updateRate(double)));
#else
    //if no vinyl control, just call it 33
    this->updateVinylControlSpeed(33.0);
#endif
}

void WSpinny::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e); //ditch unused param warning

    QPainter p(this);

    if (m_pBG) {
        p.drawPixmap(0, 0, *m_pBG);
    }

#ifdef __VINYLCONTROL__
    // Overlay the signal quality drawing if vinyl is active
    if (m_bVinylActive && m_bSignalActive)
    {
        //reduce cpu load by only updating every 3 times
        m_iSignalUpdateTick = (m_iSignalUpdateTick + 1) % 3;
        if (m_iSignalUpdateTick == 0)
        {
            unsigned char * buf = m_pVinylControl->getScopeBytemap();
            int r,g,b;
            QColor qual_color = QColor();
            float signalQuality = m_pVinylControl->getTimecodeQuality();

            //color is related to signal quality
            //hsv:  s=1, v=1
            //h is the only variable.
            //h=0 is red, h=120 is green
            qual_color.setHsv((int)(120.0 * signalQuality), 255, 255);
            qual_color.getRgb(&r, &g, &b);

            if (buf) {
                for (int y=0; y<m_iSize; y++) {
                    QRgb *line = (QRgb *)m_qImage.scanLine(y);
                    for(int x=0; x<m_iSize; x++) {
                        //use xwax's bitmap to set alpha data only
                        //adjust alpha by 3/4 so it's not quite so distracting
                        //setpixel is slow, use scanlines instead
                        //m_qImage.setPixel(x, y, qRgba(r,g,b,(int)buf[x+m_iSize*y] * .75));
                        *line = qRgba(r,g,b,(int)(buf[x+m_iSize*y] * .75));
                        line++;
                    }
                }
                p.drawImage(this->rect(), m_qImage);
            }
        }
        else
        {
            //draw the last good image
            p.drawImage(this->rect(), m_qImage);
        }
    }
#endif

    //To rotate the foreground pixmap around the center of the image,
    //we use the classic trick of translating the coordinate system such that
    //the origin is at the center of the image. We then rotate the coordinate system,
    //and draw the pixmap at the corner.
    p.translate(width() / 2, height() / 2);

    if (m_bGhostPlayback)
        p.save();

    if (m_pFG && !m_pFG->isNull()) {
        //Now rotate the pixmap and draw it on the screen.
        p.rotate(m_fAngle);
        p.drawPixmap(-(width() / 2), -(height() / 2), *m_pFG);
    }

    if (m_bGhostPlayback && m_pGhost && !m_pGhost->isNull())
    {
        p.restore();
        p.save();
        p.rotate(m_fGhostAngle);
        p.drawPixmap(-(width() / 2), -(height() / 2), *m_pGhost);

        //Rotate back to the playback position (not the ghost positon),
        //and draw the beat marks from there.
        p.restore();

        /*
        //Draw a line where the next 4 beats are
        double bpm = m_pBPM->get();
        double duration = m_pDuration->get();
        if (bpm <= 0. || duration <= 0.) {
            return; //Prevent div by zero
        }
        double beatLengthInSec = 60. / bpm;
        double beatLengthNormalized = beatLengthInSec / duration; //Noramlized to duration
        double beatAngle = calculateAngle(beatLengthNormalized);
        //qDebug() << "beatAngle:" << beatAngle;
        //qDebug() << "beatLenInSec:" << beatLengthInSec << "norm:" << beatLengthNormalized;
        p.rotate(m_fAngle);
        for (int i = 0; i < 4; i++) {
            QLineF beatLine(-(width()*0.6 / 2), -(height()*0.6 / 2),
                            -(width()*0.8 / 2), -(height()*0.8 / 2));
            //p.drawPoint(-(width()*0.5 / 2), -(height()*0.5 / 2));
            p.drawLine(beatLine);
            p.rotate(beatAngle);
        } */
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
        return 0.0f;
    }

    // Convert playpos to seconds.
    double t = playpos * trackFrames / trackSampleRate;

    // Bad samplerate or number of track samples.
    if (isnan(t)) {
        return 0.0f;
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
        qDebug() << "Angle clamping failed!" << t << originalAngle << "->" << angle
                 << "Please file a bug or email mixxx-devel@lists.sourceforge.net";
        return 0.0;
    }
    return angle;
}

/** Given a normalized playpos, calculate the integer number of rotations
    that it would take to wind the vinyl to that position. */
int WSpinny::calculateFullRotations(double playpos)
{
    if (isnan(playpos))
        return 0.0f;
    //Convert playpos to seconds.
    //double t = playpos * m_pDuration->get();
    double t = playpos * (m_pTrackSamples->get()/2 /  // Stereo audio!
                          m_pTrackSampleRate->get());

    //33 RPM is approx. 0.5 rotations per second.
    //qDebug() << t;
    double angle = 360*m_dRotationsPerSecond*t;

    return (((int)angle+180) / 360);
}

//Inverse of calculateAngle()
double WSpinny::calculatePositionFromAngle(double angle)
{
    if (isnan(angle)) {
        return 0.0f;
    }

    //33 RPM is approx. 0.5 rotations per second.
    double t = angle/(360.0 * m_dRotationsPerSecond); //time in seconds

    double trackFrames = m_pTrackSamples->get() / 2;
    double trackSampleRate = m_pTrackSampleRate->get();
    if (isnan(trackFrames) || isnan(trackSampleRate) ||
        trackFrames <= 0 || trackSampleRate <= 0) {
        return 0.0f;
    }

    //Convert t from seconds into a normalized playposition value.
    //double playpos = t / m_pDuration->get();
    double playpos = t * trackSampleRate / trackFrames;
    if (isnan(playpos)) {
        return 0.0;
    }
    return playpos;
}

/** Update the playback angle saved in the widget and repaint.
    @param playpos A normalized (0.0-1.0) playback position. (Not an angle!)
*/
void WSpinny::updateAngle(double playpos) {
    m_fAngle = calculateAngle(playpos);
}

void WSpinny::updateRate(double rate) {
    //if rate is zero, updateAngle won't get called,
    if (rate == 0.0 && m_bVinylActive)
    {
    }
}

//Update the angle using the ghost playback position.
void WSpinny::updateAngleForGhost()
{
    qint64 elapsed = m_time.elapsed();
    double duration = m_pDuration->get();
    double newPlayPos = m_dPausedPosition +
                         (((double)elapsed)/1000.)/duration;
    m_fGhostAngle = calculateAngle(newPlayPos);
}

void WSpinny::updateVinylControlSpeed(double rpm)
{
    m_dRotationsPerSecond = rpm/60.;
}

void WSpinny::updateVinylControlEnabled(double enabled)
{
#ifdef __VINYLCONTROL__
    if (enabled)
    {
        if (m_pVinylControl == NULL)
        {
            m_pVinylControl = m_pVCManager->getVinylControlProxyForChannel(m_group);
            if (m_pVinylControl != NULL)
            {
                m_bVinylActive = true;
                m_bSignalActive = m_pSignalEnabled->get();
                connect(m_pVinylControl, SIGNAL(destroyed()),
                    this, SLOT(invalidateVinylControl()));
            }
        }
        else
        {
            m_bVinylActive = true;
        }
    }
    else
    {
        m_bVinylActive = false;
    }
#endif
}

void WSpinny::invalidateVinylControl()
{
#ifdef __VINYLCONTROL__
    m_bVinylActive = false;
    m_pVinylControl = NULL;
#endif
}


void WSpinny::mouseMoveEvent(QMouseEvent * e)
{
    int y = e->y();
    int x = e->x();

    //Keeping these around in case we want to switch to control relative
    //to the original mouse position.
    //int dX = x-m_iStartMouseX;
    //int dY = y-m_iStartMouseY;

    //Coordinates from center of widget
    double c_x = x - width()/2;
    double c_y = y - height()/2;
    double theta = (180.0f/M_PI)*atan2(c_x, -c_y);

    //qDebug() << "c_x:" << c_x << "c_y:" << c_y <<
    //            "dX:" << dX << "dY:" << dY;

    //When we finish one full rotation (clockwise or anticlockwise),
    //we'll need to manually add/sub 360 degrees because atan2()'s range is
    //only within -180 to 180 degrees. We need a wider range so your position
    //in the song can be tracked.
    if (m_dPrevTheta > 100 && theta < 0) {
        m_iFullRotations++;
    }
    else if (m_dPrevTheta < -100 && theta > 0) {
        m_iFullRotations--;
    }

    m_dPrevTheta = theta;
    theta += m_iFullRotations*360;

    //qDebug() << "c t:" << theta << "pt:" << m_dPrevTheta <<
    //            "icr" << m_iFullRotations;

    if (e->buttons() & Qt::LeftButton && !m_bVinylActive)
    {
        //Convert deltaTheta into a percentage of song length.
        double absPos = calculatePositionFromAngle(theta);

        double absPosInSamples = absPos * m_pTrackSamples->get();
        m_pScratchPos->slotSet(absPosInSamples - m_dInitialPos);
    }
    else if (e->buttons() & Qt::MidButton)
    {
    }
    else if (e->buttons() & Qt::NoButton)
    {
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

    if (e->button() == Qt::LeftButton)
    {
        QApplication::setOverrideCursor(QCursor(Qt::ClosedHandCursor));

        // Coordinates from center of widget
        double c_x = x - width()/2;
        double c_y = y - height()/2;
        double theta = (180.0f/M_PI)*atan2(c_x, -c_y);
        m_dPrevTheta = theta;
        m_iFullRotations = calculateFullRotations(m_pPlayPos->get());
        theta += m_iFullRotations * 360.0;
        m_dInitialPos = calculatePositionFromAngle(theta) * m_pTrackSamples->get();

        m_pScratchPos->slotSet(0);
        m_pScratchToggle->slotSet(1.0f);

        //Trigger a mouse move to immediately line up the vinyl with the cursor
        mouseMoveEvent(e);
    }
    else if (e->button() == Qt::MidButton)
    {
    }
    else if (e->button() == Qt::RightButton)
    {
        //Stop playback and start the timer for ghost playback
        m_time.start();
        m_dPausedPosition = m_pPlayPos->get();
        updateAngleForGhost(); //Need to recalc the ghost angle right away
        m_bGhostPlayback = true;
        m_ghostPaintTimer.start(30);
        connect(&m_ghostPaintTimer, SIGNAL(timeout()),
                this, SLOT(updateAngleForGhost()));

        //TODO: Ramp down (brake) over a period of 1 beat
        //      instead? Would be sweet.
        m_pPlay->slotSet(0.0f);
    }
}

void WSpinny::mouseReleaseEvent(QMouseEvent * e)
{
    if (e->button() == Qt::LeftButton)
    {
        QApplication::restoreOverrideCursor();
        m_pScratchToggle->slotSet(0.0f);
        m_iFullRotations = 0;
    }
    else if (e->button() == Qt::RightButton)
    {
        //Start playback by jumping forwards in the song as if playback
        //was never paused. (useful for bleeping or adding silence breaks)
        qint64 elapsed = m_time.elapsed();
        //qDebug() << "elapsed:" << elapsed;
        m_ghostPaintTimer.stop();
        m_bGhostPlayback = false;

        //Convert elapsed to seconds, then normalize it to the duration so we can
        //move the playback position ahead by the elapsed amount.
        double duration = m_pDuration->get();
        double newPlayPos = m_dPausedPosition + (((double)elapsed)/1000.)/duration;
        //qDebug() << m_dPausedPosition << newPlayPos;
        m_pPlay->slotSet(1.0f);
        m_pPlayPos->slotSet(newPlayPos);
        //m_bRightButtonPressed = true;
    }
}

void WSpinny::wheelEvent(QWheelEvent *e)
{
    Q_UNUSED(e); //ditch unused param warning

    /*
    double wheelDirection = ((QWheelEvent *)e)->delta() / 120.;
    double newValue = getValue() + (wheelDirection);
    this->updateValue(newValue);

    e->accept();
    */
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

void WSpinny::dropEvent(QDropEvent * event)
{
    if (event->mimeData()->hasUrls()) {
        QList<QUrl> urls(event->mimeData()->urls());
        QUrl url = urls.first();
        QString name = url.toLocalFile();
        //If the file is on a network share, try just converting the URL to a string...
        if (name == "")
            name = url.toString();

        event->accept();
        emit(trackDropped(name, m_group));
    } else {
        event->ignore();
    }
}
