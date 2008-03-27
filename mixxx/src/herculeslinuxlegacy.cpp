// Below this line is the original non-libdjconsole implementation.

#ifndef MSC_PULSELED
// this may not have made its way into the kernel headers yet ...
  #define MSC_PULSELED 0x01
#endif

Q3ValueList <int> HerculesLinux::sqlOpenDevs;

HerculesLinux::HerculesLinux() : Hercules()
{
    m_iFd = -1;
    m_iId = -1;
    m_iJogLeft = -1;
    m_iJogRight = -1;

    m_dJogLeftOld = 0.;
    m_dJogRightOld = 0.;
    m_iPitchOffsetLeft=-9999;
    m_iPitchOffsetRight=-9999;
    m_iPitchLeft = -1;
    m_iPitchRight = -1;
    m_bClearLeftPitchPlus=false;
    m_bClearLeftPitchMinus=false;
    m_bClearRightPitchPlus=false;
    m_bClearRightPitchMinus=false;
}

HerculesLinux::~HerculesLinux()
{
}

void HerculesLinux::run()
{
	clear_leds();
    while (1)
    {
    
        getNextEvent();

        if (m_pControlObjectLeftBtnPlayProxy->get()!=m_bPlayLeft)
        {
            m_bPlayLeft=!m_bPlayLeft;
            led_write(kiHerculesLedLeftCueBtn, m_bPlayLeft);
        }
        if (m_pControlObjectRightBtnPlayProxy->get()!=m_bPlayRight)
        {
            m_bPlayRight=!m_bPlayRight;
            led_write(kiHerculesLedRightCueBtn, m_bPlayRight);
        }
        if (m_pControlObjectLeftBtnLoopProxy->get()!=m_bLoopLeft)
        {
            m_bLoopLeft=!m_bLoopLeft;
            //led_write(kiHerculesLedLeftCueBtn, m_bLoopLeft);
        }
        if (m_pControlObjectRightBtnLoopProxy->get()!=m_bLoopRight)
        {
            m_bLoopRight=!m_bLoopRight;
            //led_write(kiHerculesLedRightCueBtn, m_bLoopRight);
        }
        if (m_pControlObjectLeftBtnHeadphoneProxy->get() != m_bHeadphoneLeft) {
	        m_bHeadphoneLeft=!m_bHeadphoneLeft;
	        led_write(kiHerculesLedLeftHeadphone, m_bHeadphoneLeft);
	    }
        if (m_pControlObjectRightBtnHeadphoneProxy->get() != m_bHeadphoneRight) {
	        m_bHeadphoneRight=!m_bHeadphoneRight;
	        led_write(kiHerculesLedRightHeadphone, m_bHeadphoneRight);
	    }
	    int leftVU = (int)(m_pControlObjectLeftVuMeter->get()*4.0);
	    if (leftVU != m_iLeftVU) {
	        m_iLeftVU = leftVU;
	        led_write(kiHerculesLedLeftFx, leftVU == 3);
	        led_write(kiHerculesLedLeftCueLamp, leftVU > 1);
	        led_write(kiHerculesLedLeftLoop, leftVU > 0);
	    }
	    int rightVU = (int)(m_pControlObjectRightVuMeter->get()*4.0);
	    if (rightVU != m_iRightVU) {
	        m_iRightVU = rightVU;
	        led_write(kiHerculesLedRightFx, rightVU == 3);
	        led_write(kiHerculesLedRightCueLamp, rightVU > 1);
	        led_write(kiHerculesLedRightLoop, rightVU > 0);
	    }
	    // We wait as long as possible to do this, otherwise we reset too
        // quick, and the faked press is never picked up.
        // (note: this currently isn't used, as no matter where I put it
        // it's too fast. A better solution would be to find/make a relative
        // shifter interface)
        if (m_bClearLeftPitchPlus) {
            m_bClearLeftPitchPlus = false;
            sendButtonEvent(0,m_pControlObjectLeftBtnPitchBendPlus);
        }
        if (m_bClearLeftPitchMinus) {
            m_bClearLeftPitchMinus = false;
            sendButtonEvent(0,m_pControlObjectLeftBtnPitchBendMinus);
        }
        if (m_bClearRightPitchPlus) {
            m_bClearRightPitchPlus = false;
            sendButtonEvent(0,m_pControlObjectRightBtnPitchBendPlus);
        }
        if (m_bClearRightPitchMinus) {
            m_bClearRightPitchMinus = false;
            sendButtonEvent(0,m_pControlObjectRightBtnPitchBendMinus);
        }

    }
}

bool HerculesLinux::opendev()
{
    for(int i=0; i<kiHerculesNumEventDevices; i++)
    {
        if (sqlOpenDevs.find(i)==sqlOpenDevs.end())
        {
//            qDebug() << "Looking for a Hercules DJ Console on /dev/input/event" << i << " ...";
            m_iFd = opendev(i);
            if(m_iFd >= 0)
                break;
        }
    }
    if (m_iFd>0)
    {
        qDebug() << "Hercules device @ " << m_iFd;
        // Start thread
        start();

        return true;
    }
    else
        qDebug() << "Hercules device (" << m_iFd << ") not found!";
    return false;
}

void HerculesLinux::closedev()
{
    if (m_iFd>0)
    {
        close(m_iFd);

        // Remove id from list
        Q3ValueList<int>::iterator it = sqlOpenDevs.find(m_iId);
        if (it!=sqlOpenDevs.end())
            sqlOpenDevs.remove(it);
    }
    m_iFd = -1;
    m_iId = -1;
}

int HerculesLinux::opendev(int iId)
{
    char rgcDevName[256];
    sprintf(rgcDevName, "/dev/input/event%d", iId);
    int iFd = open(rgcDevName, O_RDWR|O_NONBLOCK);
    int i;
    char rgcName[255];

    if(iFd < 0) {
//        qDebug() << "Could not open Hercules at /dev/input/event" << iId << " [" << strerror(errno) << "]";
        if (errno==13) {
            qDebug() << "If you have a Hercules device plugged into USB, you'll need to either execute 'sudo chmod o+rw- /dev/input/event?' or run mixxx as root.";
        }
        return -1;
    }
    if(ioctl(iFd, EVIOCGNAME(sizeof(rgcName)), rgcName) < 0)
    {
        qDebug() << "EVIOCGNAME got negative size at /dev/input/event" << iId;
        close(iFd);
        return -1;
    }
    // it's the correct device if the prefix matches what we expect it to be:
    for(i=0; i<kiHerculesNumValidPrefixes; i++) {
        if (kqHerculesValidPrefix[i]==rgcName)
        {
            m_iId = iId;
            m_iInstNo = sqlOpenDevs.count();

            // Add id to list of open devices
            sqlOpenDevs.append(iId);

            qDebug() << "pm id " << iId;

            return iFd;
        }
        qDebug() << "  " << i << ". rgcName = [" << (const char *)rgcName << "]";
        qDebug() << "  " << i << ". kqHerculesValidPrefix[i] = [" << kqHerculesValidPrefix[i] << "]";
    }

    close(iFd);
    return -1;
}

void HerculesLinux::getNextEvent()
{
    FD_ZERO(&fdset);
    FD_SET(m_iFd, &fdset);
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 10000;
    int v = select(m_iFd+1, &fdset, 0, 0, &tv);

    if (v<=0)
    {
        double r;

        r = m_pRotaryLeft->filter(0.);
        if (r!=0. || r!=m_dLeftVolumeOld)
            sendEvent(r, m_pControlObjectLeftJog);
        m_dLeftVolumeOld = r;

        r = m_pRotaryRight->filter(0.);
        if (r!=0. || r!=m_dRightVolumeOld)
            sendEvent(r, m_pControlObjectRightJog);
        m_dRightVolumeOld = r;

        return;
    }

//     qDebug() << "v " << v << ", usec " << tv.tv_usec << ", isset " << FD_ISSET(m_iFd;

    struct input_event ev;
    int iR = read(m_iFd, &ev, sizeof(struct input_event));
    if (iR == sizeof(struct input_event))
    {
        if (ev.type == EV_SYN) { // avoid unnecessary calculations
            return;
        }

        // double v = ((ev.value+1)/(4.- ((ev.value>((7/8.)*256))*((ev.value-((7/8.)*256))*1/16.)))); // GED's formula, might need some work
        // Albert's http://zunzun.com/ site saves the day by solving our data points to this new magical formula...
        double magic = (0.733835252488 * tan((0.00863901501308 * ev.value) - 4.00513109039)) + 0.887988233294;

        double divisor = 256.;
        double d1 = divisor-1;
        double d2 = (divisor/2)-1;

        // qDebug() << "ev.type:" << ev.type << "ev.code:" << ev.code << "ev.value:" << ev.value;
        //qDebug() << "type " << ev.type << ", code " << ev.code << ", value " << ev.value;
        //qDebug() << "type " << ev.type << ", code " << ev.code << ", value " << ev.value << ", v is " << v;

        switch(ev.type)
        {
        case EV_ABS:
            //qDebug() << "code " << ev.code;
            int iDiff;
            double dDiff;
            int pitchValue;
            
            switch (ev.code)
            {
            case kiHerculesLeftTreble:
                sendEvent(magic, m_pControlObjectLeftTreble);
                break;
            case kiHerculesLeftMiddle:
                sendEvent(magic, m_pControlObjectLeftMiddle);
                break;
            case kiHerculesLeftBass:
                sendEvent(magic, m_pControlObjectLeftBass);
                break;
            case kiHerculesLeftVolume:
                m_dLeftVolumeOld = ev.value/d1;
                sendEvent(ev.value/d1, m_pControlObjectLeftVolume);
                break;
            case kiHerculesLeftPitch:
                pitchValue = PitchChangeOrdinal("Left", ev.value, m_iPitchLeft, m_iPitchOffsetLeft);
                if (pitchValue > 0) {
                    for (int i=0; i<pitchValue; i++)
                        sendButtonEvent(1, m_pControlObjectLeftBtnPitchBendPlus);
                } else {
                    for (int i=0; i>pitchValue; i--)
                        sendButtonEvent(1, m_pControlObjectLeftBtnPitchBendMinus);
                }
//                sendEvent(PitchChange("Left", ev.value, m_iPitchLeft, m_iPitchOffsetLeft), m_pControlObjectLeftPitch);
                break;
            case kiHerculesLeftJog:
                iDiff = 0;
                if (m_iJogLeft>=0)
                    iDiff = ev.value-m_iJogLeft;
                if (iDiff<-200)
                    iDiff += 256;
                else if (iDiff>200)
                    iDiff -= 256;
                dDiff = m_pRotaryLeft->filter((double)iDiff/16.);
                //qDebug() << "Left Jog - ev.value " << ev.value << ", m_iJogLeft " << m_iJogLeft << ", idiff " << iDiff << ", dDiff " << dDiff;
                m_iJogLeft = ev.value;
                sendEvent(dDiff, m_pControlObjectLeftJog);
                break;
            case kiHerculesRightTreble:
                sendEvent(magic, m_pControlObjectRightTreble);
                break;
            case kiHerculesRightMiddle:
                sendEvent(magic, m_pControlObjectRightMiddle);
                break;
            case kiHerculesRightBass:
                sendEvent(magic, m_pControlObjectRightBass);
                break;
            case kiHerculesRightVolume:
                m_dRightVolumeOld = ev.value/d1;
                //qDebug() << "R Volume " << ev.value/d1;
                sendEvent(ev.value/d1, m_pControlObjectRightVolume);
                break;
            case kiHerculesRightPitch:
                //qDebug() << "";
                pitchValue = PitchChangeOrdinal("Right", ev.value, m_iPitchRight, m_iPitchOffsetRight);
                if (pitchValue > 0) {
                    for (int i=0; i<pitchValue; i++) {
                        sendButtonEvent(1, m_pControlObjectRightBtnPitchBendPlus);
                    }
                } else {
                    for (int i=0; i>pitchValue; i--) {
                        sendButtonEvent(1, m_pControlObjectRightBtnPitchBendMinus);
                    }
                }
//                sendEvent(PitchChange("Right", ev.value, m_iPitchRight, m_iPitchOffsetRight), m_pControlObjectRightPitch);
                break;
            case kiHerculesRightJog:
                iDiff = 0;
                if (m_iJogRight>=0)
                    iDiff = ev.value-m_iJogRight;
                if (iDiff<-200)
                    iDiff += 256;
                else if (iDiff>200)
                    iDiff -= 256;
                dDiff = m_pRotaryRight->filter((double)iDiff/16.);
//                    qDebug() << "Right Jog - ev.value " << ev.value << ", m_iJogRight " << m_iJogRight << ", idiff " << iDiff << ", dDiff " << dDiff;
                m_iJogRight = ev.value;
                sendEvent(dDiff, m_pControlObjectRightJog);
                break;
            case kiHerculesCrossfade:
                //qDebug() << "(ev.value+1)/2.0f: " << (ev.value+1)/2.0f;
                sendEvent((ev.value-d2)/d2, m_pControlObjectCrossfade);
                break;
//              default:
//                  sendEvent(0., m_pControlObjectLeftJog);
//                  sendEvent(0., m_pControlObjectRightJog);
            }
            break;
        case EV_KEY:
                switch (ev.code)
                {
                case kiHerculesLeftBtnPitchBendMinus:
                    sendButtonEvent(ev.value, m_pControlObjectLeftBtnPitchBendMinus);
                    break;
                case kiHerculesRightBtnPitchBendMinus:
                    sendButtonEvent(ev.value, m_pControlObjectRightBtnPitchBendMinus);
                    break;

                case kiHerculesLeftBtnPitchBendPlus:
                    sendButtonEvent(ev.value, m_pControlObjectLeftBtnPitchBendPlus);
                    break;
                case kiHerculesRightBtnPitchBendPlus:
                    sendButtonEvent(ev.value, m_pControlObjectRightBtnPitchBendPlus);
                    break;

                case kiHerculesLeftBtnTrackNext:
		    sendButtonEvent(ev.value, m_pControlObjectLeftBtnTrackNext);
                    break;
                case kiHerculesRightBtnTrackNext:
                    sendButtonEvent(ev.value, m_pControlObjectRightBtnTrackNext);
                    break;

                case kiHerculesLeftBtnTrackPrev:
                    sendButtonEvent(ev.value, m_pControlObjectLeftBtnTrackPrev);
                    break;
                case kiHerculesRightBtnTrackPrev:
                    sendButtonEvent(ev.value, m_pControlObjectRightBtnTrackPrev);
                    break;

                case kiHerculesLeftBtnCue:
                      //m_bCueLeft = !m_bCueLeft;
                      sendButtonEvent(ev.value, m_pControlObjectLeftBtnCue);
                      //led_write(kiHerculesLedLeftCueBtn, m_bCueLeft);
                    break;
                case kiHerculesRightBtnCue:
                    //m_bCueRight = !m_bCueRight;
                    sendButtonEvent(ev.value, m_pControlObjectRightBtnCue);
                    //led_write(kiHerculesLedRightCueBtn, m_bCueRight);
                    break;

                case kiHerculesLeftBtnPlay:
                    if (ev.value) {
//                    m_bPlayLeft = !m_pControlObjectLeftBtnPlay->get();
                      sendButtonEvent(!m_pControlObjectLeftBtnPlay->get(), m_pControlObjectLeftBtnPlay);
//                    led_write(kiHerculesLedLeftPlay, m_bPlayLeft);
                    }
                    break;
                case kiHerculesRightBtnPlay:
                    if (ev.value) {
//                     m_bPlayRight = !m_pControlObjectRightBtnPlay->get();
                      sendButtonEvent(!m_pControlObjectRightBtnPlay->get(), m_pControlObjectRightBtnPlay);
//                     led_write(kiHerculesLedRightPlay, m_bPlayRight);
                    }
                    break;

                case kiHerculesLeftBtnHeadphone: 
                    if (ev.value) {
                      //led_write(kiHerculesLedLeftHeadphone, !m_pControlObjectLeftBtnHeadphone->get());
                      sendButtonEvent(!m_bHeadphoneLeft, m_pControlObjectLeftBtnHeadphone);
                    }
                    break;
                case kiHerculesRightBtnHeadphone:
                    if (ev.value) {
                      //led_write(kiHerculesLedRightHeadphone, !m_pControlObjectRightBtnHeadphone->get());
                      sendButtonEvent(!m_bHeadphoneRight, m_pControlObjectRightBtnHeadphone);
                    }
                    break;

                case kiHerculesLeftBtnAutobeat:
                    sendButtonEvent(ev.value, m_pControlObjectLeftBtnAutobeat);
                    m_bSyncLeft = !m_bSyncLeft;
//                     led_write(kiHerculesLedLeftSync, m_bSyncLeft);
                    break;
                case kiHerculesRightBtnAutobeat:
                    sendButtonEvent(ev.value, m_pControlObjectRightBtnAutobeat);
                    m_bSyncRight = !m_bSyncRight;
//                     led_write(kiHerculesLedRightSync, m_bSyncRight);
                    break;

                case kiHerculesLeftBtnMasterTempo:
//                     sendEvent(0, m_pControlObjectLeftBtnMasterTempo);
//                     m_bMasterTempoLeft = !m_bMasterTempoLeft;
//                     led_write(kiHerculesLedLeftMasterTempo, m_bMasterTempoLeft);
                    break;
                case kiHerculesRightBtnMasterTempo:
//                     sendEvent(1., m_pControlObjectRightBtnMasterTempo);
//                     m_bMasterTempoRight = !m_bMasterTempoRight;
                    //led_write(kiHerculesLedRightMasterTempo, m_bMasterTempoRight);
                    break;

                case kiHerculesLeftBtnFx:
                    sendButtonEvent(true, m_pControlObjectLeftBtnFx);
/*
                    m_iLeftFxMode = (m_iLeftFxMode+1)%3;
                    qDebug() << "left fx " << m_iLeftFxMode==0 << "," << m_iLeftFxMode==1 << "," << m_iLeftFxMode==2;
                    changeJogMode(m_iLeftFxMode,m_iRightFxMode);
                    led_write(kiHerculesLedLeftFx, m_iLeftFxMode==0);
                    led_write(kiHerculesLedLeftCueLamp, m_iLeftFxMode==1);
                    led_write(kiHerculesLedLeftLoop, m_iLeftFxMode==2);
 */
                    break;
                case kiHerculesRightBtnFx:
                    sendButtonEvent(true, m_pControlObjectRightBtnFx);
/*
                    m_iRightFxMode = (m_iRightFxMode+1)%3;
                    changeJogMode(m_iLeftFxMode,m_iRightFxMode);

   //                     if (m_iRightFxMode==0)
                    {
                        led_write(kiHerculesLedRightCueLamp, false);
                        led_write(kiHerculesLedRightLoop, false);
                        led_write(kiHerculesLedRightFx, false);
                        led_write(kiHerculesLedRightCueLamp, false);
                        led_write(kiHerculesLedRightLoop, false);
                        led_write(kiHerculesLedRightFx, false);
                    }
                    if (m_iRightFxMode==1)
                        led_write(kiHerculesLedRightCueLamp, true);
                    if (m_iRightFxMode==2)
                        led_write(kiHerculesLedRightLoop, true);
 */
                    break;


                case kiHerculesLeftBtn1:
                case kiHerculesLeftBtn2:
                case kiHerculesLeftBtn3:
		    if (ev.value) {
                      m_iLeftFxMode = ev.code - kiHerculesLeftBtn1;
                      changeJogMode(m_iLeftFxMode, m_iRightFxMode);
                      sendButtonEvent(true, m_pControlObjectLeftBtn123[m_iLeftFxMode]);
                    }
                    break;
                case kiHerculesRightBtn1:
                case kiHerculesRightBtn2:
                case kiHerculesRightBtn3:
		    if (ev.value) {
                      m_iRightFxMode = ev.code - kiHerculesRightBtn1;
                      changeJogMode(m_iLeftFxMode, m_iRightFxMode);
                      sendButtonEvent(true, m_pControlObjectRightBtn123[m_iRightFxMode]);
                    }
                    break;

                }
            break;
            }

         }

    //
    // Check if led queue is empty
    //

    // Check if we have to turn on led
    //if (m_qRequestLed.available()==0)
//     {
    //  m_qRequestLed--;
//         led_write(ki);

//         msleep(5);

//         led_write(0, 0, 0, 0, 0);
//     }
    //else if (iR != sizeof(struct input_event))
    //    msleep(5);
}

void HerculesLinux::led_write(int iLed, bool bOn)
{
//     if (bOn) qDebug() << "true";
//     else qDebug() << "false";

    struct input_event ev;
    memset(&ev, 0, sizeof(struct input_event));

    ev.type = EV_LED;
    //ev.type = 0x0000;
    ev.code = iLed;
    if (bOn)
        ev.value = 1;
    else
        ev.value = 0;

    //qDebug() << "Hercules: led_write(iLed=" << iLed << ", value=" << ev.value << ")";

    if (write(m_iFd, &ev, sizeof(struct input_event)) != sizeof(struct input_event))
        qDebug() << "Hercules: write(): " << strerror(errno);
}

void HerculesLinux::selectMapping(QString qMapping)
{
    Hercules::selectMapping(qMapping);

    if (qMapping==kqInputMappingHerculesInBeat)
    {
        led_write(kiHerculesLedLeftSync, true);
        led_write(kiHerculesLedRightSync, true);
    }
    else
    {
        //led_write(kiHerculesLedLeftSync, false);
        //led_write(kiHerculesLedRightSync, false);
    }
}

/*double HerculesLinux::PitchChange(const QString ControlSide, const int ev_value, int &m_iPitchPrevious, int &m_iPitchOffset) {
    // Handle the initial event from the Hercules and set pitch to default of 0% change
    if (m_iPitchPrevious < 0) {
        m_iPitchOffset = ev_value;
        m_iPitchPrevious = 64;
        return ((m_iPitchPrevious+1)-64)/64.;
    }

    int delta = ev_value - m_iPitchOffset;
    if (delta >= 240) {
        delta = (255 - delta) * -1;
    }
    if (delta <= -240) {
        delta = 255 + delta;
    }
    m_iPitchOffset = ev_value;

#ifdef __THOMAS_HERC__
    int pitchAdjustStep = delta; // * 3;
#else
    int pitchAdjustStep = delta * 3;
#endif

    if ((pitchAdjustStep > 0 && m_iPitchPrevious+pitchAdjustStep < 128) || (pitchAdjustStep < 0 && m_iPitchPrevious+pitchAdjustStep > 0)) {
        m_iPitchPrevious = m_iPitchPrevious+pitchAdjustStep;
    } else if (pitchAdjustStep > 0) {
        m_iPitchPrevious = 127;
    } else if (pitchAdjustStep < 0) {
        m_iPitchPrevious = 0;
    }

    // qDebug() << "PitchChange [" << ControlSide << "] PitchAdjust" << pitchAdjustStep <<"-> new Pitch:" << m_iPitchPrevious << " NewRangeAdjustedPitch:" <<  QString::number(((m_iPitchPrevious+1)$

    // old range was 0..127
    // new range is -1.0 to 1.0
    return ((m_iPitchPrevious+1) - 64)/64.;
}*/

/*
    The old version of this sets the absolute pitch, but the herc doesn't have
    an absolute pitch slider, so this treats it like a pitch bend. This function
    returns the number of fake button presses that should be performed.
*/
int HerculesLinux::PitchChangeOrdinal(const QString ControlSide, const int ev_value, int &m_iPitchPrevious, int &m_iPitchOffset) {
    // We have to ignore the first value we get here, as that lets us calibrate.
    // We don't know which way they've turned it. A better option would be
    // either to query it on start, or reset the device. I don't know how to
    // do either through the event interface.
    if (m_iPitchPrevious < 0)
        m_iPitchPrevious = ev_value;
    int delta = ev_value - m_iPitchPrevious;
    if (delta >= 240) {
        delta = (255 - delta) * -1;
    } else if (delta <= -240) {
        delta = 255 + delta;
    }
    m_iPitchPrevious = ev_value;
    return delta;
}

/*
 * Cutesy little LED initialisation show
 */
void HerculesLinux::clear_leds() {
	const int tenth_sec = 100000;
	led_write(kiHerculesLedRightSync, true);
	led_write(kiHerculesLedLeftSync, true);
	led_write(kiHerculesLedLeftLoop, true);
	led_write(kiHerculesLedRightLoop, true);
	led_write(kiHerculesLedLeftMasterTempo, true);
	led_write(kiHerculesLedRightMasterTempo, true);
	led_write(kiHerculesLedLeftFx, true);
	led_write(kiHerculesLedRightFx, true);
	led_write(kiHerculesLedRightCueLamp, true);
	led_write(kiHerculesLedRightCueBtn, true);
	led_write(kiHerculesLedLeftCueLamp, true);
	led_write(kiHerculesLedLeftHeadphone, true);
	led_write(kiHerculesLedRightHeadphone, true);
	led_write(kiHerculesLedLeftCueBtn, true);

	usleep(2 * tenth_sec);
	led_write(kiHerculesLedLeftFx, false);
	led_write(kiHerculesLedRightFx, false);
	
	usleep(1 * tenth_sec);
	led_write(kiHerculesLedLeftCueLamp, false);
	led_write(kiHerculesLedRightCueLamp, false);
	
	usleep(1 * tenth_sec);
	led_write(kiHerculesLedLeftLoop, false);
	led_write(kiHerculesLedRightLoop, false);
	
	usleep(1 * tenth_sec);
	led_write(kiHerculesLedLeftMasterTempo, false);
	led_write(kiHerculesLedRightMasterTempo, false);
	led_write(kiHerculesLedLeftHeadphone, false);
	led_write(kiHerculesLedRightHeadphone, false);

	usleep(1 * tenth_sec);
	led_write(kiHerculesLedLeftSync, false);
	led_write(kiHerculesLedRightSync, false);

	usleep(1 * tenth_sec);
	led_write(kiHerculesLedLeftCueBtn, false);
	led_write(kiHerculesLedRightCueBtn, false);

}
