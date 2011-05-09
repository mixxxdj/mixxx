#include <math.h>
#include "mathstuff.h"
#include "wpixmapstore.h"
#include "controlobject.h"
#include "controlobjectthreadmain.h"
#include "sharedglcontext.h"
#include "wspinny.h"

/** Speed of the vinyl rotation. */
const double ROTATIONS_PER_SECOND = 0.50f; //Roughly 33 RPM 

WSpinny::WSpinny(QWidget* parent) : 
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
    m_fAngle(0.0f),
    m_fGhostAngle(0.0f),
    m_dPausedPosition(0.0f),
    m_bGhostPlayback(false),
    m_iStartMouseX(-1),
    m_iStartMouseY(-1),
    m_iCompleteRotations(0),
    m_dPrevTheta(0.),
    QGLWidget(SharedGLContext::getContext(), parent)
{
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
}

void WSpinny::paintEvent(QPaintEvent *e)
{
    QPainter p(this);
    
    if (m_pBG) {
        p.drawPixmap(0, 0, *m_pBG);
    }

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
double WSpinny::calculateAngle(double playpos)
{
    if (isnan(playpos))
        return 0.0f;

    //Convert playpos to seconds.
    //double t = playpos * m_pDuration->get();
    double t = playpos * (m_pTrackSamples->get()/2 /  // Stereo audio!
                          m_pTrackSampleRate->get());

    if (isnan(t)) //Bad samplerate or number of track samples.
        return 0.0f;

    //33 RPM is approx. 0.5 rotations per second.
    //qDebug() << t;
    double angle = 360*ROTATIONS_PER_SECOND*t;
    //Clamp within -180 and 180 degrees
    //qDebug() << "pc:" << angle;
    //angle = ((angle + 180) % 360.) - 180;
    //modulo for doubles :)
    if (angle > 0)
    {
        int x = (angle+180)/360;
        angle = angle - (360*x);
    } else 
    {
        int x = (angle-180)/360;
        angle = angle - (360*x);
    }
    qDebug() << "poc:" << angle;
    qDebug() << playpos;

    Q_ASSERT(angle <= 180 && angle >= -180);
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
    double angle = 360*ROTATIONS_PER_SECOND*t;

    return (((int)angle+180) / 360);
}

//Inverse of calculateAngle()
double WSpinny::calculatePositionFromAngle(double angle)
{
    if (isnan(angle))
        return 0.0f;

    //33 RPM is approx. 0.5 rotations per second.
    double t = angle/(360*ROTATIONS_PER_SECOND); //time in seconds

    //Convert t from seconds into a normalized playposition value.
    //double playpos = t / m_pDuration->get();
    double playpos = t / (m_pTrackSamples->get()/2 /  // Stereo audio!
                          m_pTrackSampleRate->get());
    return playpos; 
}

/** Update the playback angle saved in the widget and repaint.
    @param playpos A normalized (0.0-1.0) playback position. (Not an angle!)
*/
void WSpinny::updateAngle(double playpos)
{
    m_fAngle = calculateAngle(playpos);
    update();
}

//Update the angle using the ghost playback position. 
void WSpinny::updateAngleForGhost()
{
    qint64 elapsed = m_time.elapsed();
    double duration = m_pDuration->get();
    double newPlayPos = m_dPausedPosition + 
                         (((double)elapsed)/1000.)/duration;
    m_fGhostAngle = calculateAngle(newPlayPos);
    update();
}


void WSpinny::mouseMoveEvent(QMouseEvent * e)
{
    int y = e->y();
    int x = e->x();

    //Keeping these around in case we want to switch to control relative
    //to the original mouse position.
    int dX = x-m_iStartMouseX;
    int dY = y-m_iStartMouseY;

    //Coordinates from center of widget
    int c_x = x - width()/2;
    int c_y = y - height()/2;
    double theta = (180.0f/M_PI)*atan2(c_x, -c_y);

    //qDebug() << "c_x:" << c_x << "c_y:" << c_y << 
    //            "dX:" << dX << "dY:" << dY;

    //When we finish one full rotation (clockwise or anticlockwise), 
    //we'll need to manually add/sub 360 degrees because atan2()'s range is
    //only within -180 to 180 degrees. We need a wider range so your position
    //in the song can be tracked.
    if (m_dPrevTheta > 100 && theta < 0) {
        m_iCompleteRotations++;
    }
    else if (m_dPrevTheta < -100 && theta > 0) {
        m_iCompleteRotations--;
    }

    m_dPrevTheta = theta;
    theta += m_iCompleteRotations*360;
   
    qDebug() << "c t:" << theta << "pt:" << m_dPrevTheta << 
                "icr" << m_iCompleteRotations;

    if (e->buttons() & Qt::LeftButton)
    {
        //Convert deltaTheta into a percentage of song length.
        double absPos = calculatePositionFromAngle(theta);

        double absPosInSamples = absPos * m_pTrackSamples->get();
        m_pScratchPos->slotSet(absPosInSamples);
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

    if (e->button() == Qt::LeftButton)
    {
        QApplication::setOverrideCursor(QCursor(Qt::ClosedHandCursor));

        double initialPosInSamples = m_pPlayPos->get() * m_pTrackSamples->get();
        m_pScratchPos->slotSet(initialPosInSamples);
        m_pScratchToggle->slotSet(1.0f);

        qDebug() << "cfr:" << calculateFullRotations(m_pPlayPos->get());
        m_iCompleteRotations = calculateFullRotations(m_pPlayPos->get());

        m_dPrevTheta = /*(m_iCompleteRotations)*360 +*/ calculateAngle(m_pPlayPos->get());

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
        m_iCompleteRotations = 0;
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
