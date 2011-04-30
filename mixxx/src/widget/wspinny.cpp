#include <math.h>
#include "mathstuff.h"
#include "wpixmapstore.h"
#include "controlobject.h"
#include "controlobjectthreadmain.h"
#include "midi/pitchfilter.h"
#include "wspinny.h"

WSpinny::WSpinny(QWidget* parent) : WWidget(parent)
{
    qDebug() << "Creating a spinny widget!";
    m_pBG = NULL;
    m_pFG = NULL;
    m_pGhost = NULL;
    m_pPlay = NULL;
    m_pPlayPos = NULL;
    m_pVisualPlayPos = NULL;
    m_pDuration = NULL;
    m_pTrackSamples = NULL;
    m_pBPM = NULL;
    m_pScratch = NULL;
    m_pScratchToggle = NULL;
    m_pScratchPos = NULL;
    m_fAngle = 0.0f;
    m_fGhostAngle = 0.0f;
    m_dPausedPosition = 0.0f;
    m_bGhostPlayback = false;
    m_bScratchPlayback = false;
    m_iStartMouseX = -1;
    m_iStartMouseY = -1;
    m_iCompleteRotations = 0;
    m_dPrevTheta = 0.;
    m_dPrevPosOffset = 0.;
#define PHYSICS_TIMER_PERIOD 30 //milliseconds
    m_pitchFilter.init(PHYSICS_TIMER_PERIOD, 0, 1, 1); //XXX: should be set to latency
    m_pitchFilterTimer.start(PHYSICS_TIMER_PERIOD);
    //connect(&m_pitchFilterTimer, SIGNAL(timeout()),
    //        this, SLOT(updatePitchFilter()));
    m_dVelocity = 0.;
    m_dPrevVelocity = 0.;
    m_dAcceleration = 0.;
    m_dDeltaPosInSeconds = 0.;
    m_dPrevY = 0.;
}

WSpinny::~WSpinny()
{
    delete m_pBG;
    delete m_pFG;
    delete m_pGhost;
    delete m_pPlay;
    delete m_pPlayPos;
    delete m_pVisualPlayPos;
    delete m_pDuration;
    delete m_pTrackSamples;
    delete m_pBPM;
    delete m_pScratch;
    delete m_pScratchToggle;
    delete m_pScratchPos;
}

void WSpinny::setup(QDomNode node)
{
    // Set pixmaps
    /*
    bool bHorizontal = false;
    if (!selectNode(node, "Horizontal").isNull() &&
        selectNodeQString(node, "Horizontal")=="true")
        bHorizontal = true;
    */

    QString group = "[Channel1]";

    qDebug() << getPath(selectNodeQString(node, "PathBackground"));
    qDebug() << getPath(selectNodeQString(node, "PathBackground"));
    qDebug() << getPath(selectNodeQString(node, "PathGhost"));

    m_pBG = WPixmapStore::getPixmap(getPath(selectNodeQString(node, 
                                                    "PathBackground")));
    m_pFG = WPixmapStore::getPixmap(getPath(selectNodeQString(node, 
                                                    "PathForeground")));
    m_pGhost = WPixmapStore::getPixmap(getPath(selectNodeQString(node, 
                                                    "PathGhost")));
    setFixedSize(m_pBG->size());

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
    connect(m_pVisualPlayPos, SIGNAL(valueChanged(double)),
            this, SLOT(updateAngle(double)));
}

void WSpinny::paintEvent(QPaintEvent *e)
{
    QPainter p(this);

    p.drawPixmap(0, 0, *m_pBG);

    //To rotate the foreground pixmap around the center of the image,
    //we use the classic trick of translatin the coordinate system such that
    //the origin is at the center of the image. We then rotate the coordinate system,
    //and draw the pixmap at the corner.
    p.translate(width() / 2, height() / 2);
    p.save();
    p.rotate(m_fAngle);
    p.drawPixmap(-(width() / 2), -(height() / 2), *m_pFG);
    
    if (m_bGhostPlayback)
    {
        p.restore();
        p.save();
        p.rotate(m_fGhostAngle);
        p.drawPixmap(-(width() / 2), -(height() / 2), *m_pGhost);

        //Rotate back to the playback position (not the ghost positon), and draw the 
        //beat marks from there.
        p.restore();
        //Draw a dot in 4 beats
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
        }
    } else {
        //Have to do this
        p.restore(); 
    }
}

/* Convert between a normalized playback position (0.0 - 1.0) and an angle
   in our polar coordinate system.
   Returns an angle clamped between -180 and 180 degrees. */
double WSpinny::calculateAngle(double playpos)
{
    //Convert playpos to seconds.
    double t = playpos * m_pDuration->get();

    //33 RPM is approx. 0.5 rotations per second.
    //qDebug() << t;
    double angle = 180*t;
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
    //qDebug() << "poc:" << angle;

    Q_ASSERT(angle <= 180 && angle >= -180);
    return angle; 
}

//Inverse of calculateAngle()
double WSpinny::calculatePositionFromAngle(double angle)
{
    //33 RPM is approx. 0.5 rotations per second.
    double t = angle/180.; //time in seconds
    //qDebug() << t;

    //Convert t from seconds into a normalized playposition value.
    double playpos = t / m_pDuration->get();

    return playpos; 
}

void WSpinny::updateAngle(double playpos)
{
    m_fAngle = calculateAngle(playpos);
    update();
}

//Update the angle using the ghost playback position. 
void WSpinny::updateAngleForGhost()
{
    qint64 elapsed = m_time.elapsed();
    //qDebug() << "ghost elapsed:" << elapsed;
    double duration = m_pDuration->get();
    double newPlayPos = m_dPausedPosition + (((double)elapsed)/1000.)/duration;
    m_fGhostAngle = calculateAngle(newPlayPos);
    update();
}

void WSpinny::updatePitchFilter()
{
    //if (m_dPrevPosOffset != 0.0f)
   //     qDebug() << "upf" << m_dPrevPosOffset;

    m_pitchFilter.observation(m_dPrevPosOffset);
    //Only feed changes in position into the filter once.
   // m_dPrevPosOffset = 0.;

        //Step 2: Measure elapsed time since previous mouse movement or click
        qint64 elapsed = m_velocityTime.elapsed();
        
        m_dVelocity = 0;

        //Apply drift control
        double screenPos = calculatePositionFromAngle(m_fAngle);
        //double drift = screenPos - m_pPlayPos->get();
        //m_dVelocity -= drift*0.05;

        //Remember the drift is a change in relative position,
        //so we have to convert it to a duration and do something to get a velocity!
        //XXX(TODO)
        
        m_dVelocity += (m_dDeltaPosInSeconds*1000 / elapsed); //- drift*0.005; 
        //qDebug() << "v:" << m_dVelocity << "d:" << drift << 
        //            "pppo:" << m_fAngle << "pp:" << m_dPrevTheta;

        m_pScratch->slotSet(m_dVelocity);
        m_pScratchPos->slotSet(screenPos);

        //Reset the velocity
        m_dVelocity = 0.0f;
        m_dDeltaPosInSeconds = 0.;

        //Start timer again.
        m_velocityTime.start();
    
}

void WSpinny::mouseMoveEvent(QMouseEvent * e)
{
    int y = e->y();
    int x = e->x();
    
    double deltaY = m_dPrevY - y;
    m_dPrevY = y;
    //int dX = x-m_iStartMouseX;
    //int dY = y-m_iStartMouseY;
    int c_x = x - width()/2;  //Coordinates from center of widget
    int c_y = y - height()/2;
    double theta = (180./M_PI)*atan2(c_x, -c_y);
    //NOTE: There is a 90 degree rotation between the polar coords of the screen
    //      and the mouse, therefore the +90 in the above.
    //qDebug() << "c_x:" << c_x << "c_y:" << c_y << "dX:" << dX << "dY:" << dY;

    double deltaTheta = theta - m_dPrevTheta;
    qDebug() << "theta:" << theta; 
    qDebug() << "prev theta:" << m_dPrevTheta << "delta:" << deltaTheta;
    if (m_dPrevTheta > 100 && theta < 0)
        deltaTheta += 360;
    else if (m_dPrevTheta < -100 && theta > 0)
        deltaTheta -= 360;
   
    m_dPrevTheta = theta;

    if (e->buttons() & Qt::LeftButton)
    {
        //Convert deltaTheta into a percentage of song length.
        double posOffset = calculatePositionFromAngle(deltaTheta);
        
        //Step 1: Get abs position of movement (y).
        //double absPos = m_pPlayPos->get() + posOffset;

        //Calculate change in song time for the given change in angle.
        //m_dDeltaPosInSeconds = deltaTheta / 180.; //33 RPM

        //m_dPrevPosOffset is fed into the pitchFilter on a timer.
        //float velocity = m_pitchFilter.currentPitch()*100000.;
        //qDebug() << "velocity:" << velocity;
        //m_pScratch->slotSet(velocity);
        double absPosInSamples = (m_pPlayPos->get() + posOffset) * m_pTrackSamples->get();
        m_pScratchPos->slotSet(posOffset * m_pTrackSamples->get());

        qDebug() << "ap:" << posOffset << posOffset * m_pTrackSamples->get();

        //Save these values for next time.
        m_dPrevPosOffset = posOffset;
        m_dPrevVelocity = m_dVelocity;


    }
    else if (e->buttons() & Qt::MiddleButton)
    {
        //Absolute movement
        double posOffset = calculatePositionFromAngle(deltaTheta);
        //qDebug() << "dtheta:" << deltaTheta << "ppo:" << posOffset;
                    //"icr:" << m_iCompleteRotations;

        m_pPlayPos->slotSet(m_pPlayPos->get() + posOffset);
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
        //QApplication::setOverrideCursor( QCursor( Qt::BlankCursor ) );
        double initialPosInSamples = m_pVisualPlayPos->get() * m_pTrackSamples->get();
        m_pScratchPos->slotSet(initialPosInSamples);
        m_pScratchToggle->slotSet(1.0f);
        m_bScratchPlayback = true;
        //For Y-axis scratching only:
        //m_dPrevY = e->y();

        //Angular scratching:
        m_dPrevTheta = calculateAngle(m_dPausedPosition);
    }
    else if (e->button() == Qt::MiddleButton)
    {
        //Trigger a seek to this position.
        m_dPausedPosition = m_pPlayPos->get();
        m_dPrevTheta = calculateAngle(m_dPausedPosition);
        mouseMoveEvent(e);
    }
    else if (e->button() == Qt::RightButton)
    {
        qDebug() << "mousePressEvent"; 
        //Stop playback and start the timer for ghost playback
        m_time.start();
        m_dPausedPosition = m_pPlayPos->get();
        updateAngleForGhost(); //Need to recalc the ghost angle right away
        m_bGhostPlayback = true;
        m_ghostTimer.start(30);
        connect(&m_ghostTimer, SIGNAL(timeout()), 
                this, SLOT(updateAngleForGhost()));

        //TODO: Ramp down (brake) over a period of 1 beat instead? Would be sweet.
        m_pPlay->slotSet(0.0f);
    }
}

void WSpinny::mouseReleaseEvent(QMouseEvent * e)
{
    if (e->button() == Qt::LeftButton)
    {
        //QApplication::restoreOverrideCursor();
        m_pScratchToggle->slotSet(0.0f);
        m_bScratchPlayback = false;
        m_iCompleteRotations = 0;
        //m_dPrevTheta = 0; //XXX: Why would we set this to zero?
    }
    else if (e->button() == Qt::RightButton)
    {
        //Start playback by jumping forwards in the song as if playback was never
        //paused. (useful for bleeping or adding silence breaks)
        qint64 elapsed = m_time.elapsed();
        qDebug() << "elapsed:" << elapsed;
        m_ghostTimer.stop();
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
