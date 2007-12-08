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
    m_iPitchLeft = 127;
    m_iPitchRight = 127;
}

HerculesLinux::~HerculesLinux()
{
}

void HerculesLinux::run()
{
    while (1)
    {
        getNextEvent();

        if (m_pControlObjectLeftBtnPlayProxy->get()!=m_bPlayLeft)
        {
            m_bPlayLeft=!m_bPlayLeft;
            led_write(kiHerculesLedLeftPlay, m_bPlayLeft);
        }
        if (m_pControlObjectRightBtnPlayProxy->get()!=m_bPlayRight)
        {
            m_bPlayRight=!m_bPlayRight;
            led_write(kiHerculesLedRightPlay, m_bPlayRight);
        }
        if (m_pControlObjectLeftBtnLoopProxy->get()!=m_bLoopLeft)
        {
            m_bLoopLeft=!m_bLoopLeft;
            led_write(kiHerculesLedLeftCueBtn, m_bLoopLeft);
        }
        if (m_pControlObjectRightBtnLoopProxy->get()!=m_bLoopRight)
        {
            m_bLoopRight=!m_bLoopRight;
            led_write(kiHerculesLedRightCueBtn, m_bLoopRight);
        }
	m_bHeadphoneLeft=m_pControlObjectLeftBtnHeadphoneProxy->get();
	m_bHeadphoneRight=m_pControlObjectRightBtnHeadphoneProxy->get();
    }
}

bool HerculesLinux::opendev()
{
    for(int i=0; i<kiHerculesNumEventDevices; i++)
    {
        if (sqlOpenDevs.find(i)==sqlOpenDevs.end())
        {
//            qDebug("Looking for a Hercules DJ Console on /dev/input/event%d ...", i);
            m_iFd = opendev(i);
            if(m_iFd >= 0)
                break;
        }
    }
    if (m_iFd>0)
    {
        qDebug("Hercules device @ %d", m_iFd);
        // Start thread
        start();

        // Turn off led
        led_write(kiHerculesLedLeftCueBtn, false);
        led_write(kiHerculesLedRightCueBtn, false);
        led_write(kiHerculesLedLeftPlay, false);
        led_write(kiHerculesLedRightPlay, false);
        led_write(kiHerculesLedLeftSync, false);
        led_write(kiHerculesLedRightSync, false);
        led_write(kiHerculesLedLeftHeadphone, false);
        led_write(kiHerculesLedRightHeadphone, false);

        return true;
    }
    else
        qDebug("Hercules device (%d) not found!", m_iFd);
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
//        qDebug("Could not open Hercules at /dev/input/event%d [%s]",iId, strerror(errno));
        if (errno==13) {
            qDebug("If you have a Hercules device plugged into USB, you'll need to either execute 'sudo chmod o+rw- /dev/input/event?' or run mixxx as root.");
        }
        return -1;
    }
    if(ioctl(iFd, EVIOCGNAME(sizeof(rgcName)), rgcName) < 0)
    {
        qDebug("EVIOCGNAME got negative size at /dev/input/event%d",iId);
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

            qDebug("pm id %i",iId);

            return iFd;
        }
        qDebug("  %d. rgcName = [%s]",i,(const char *)rgcName);
        qDebug("  %d. kqHerculesValidPrefix[i] = [%s]",i, kqHerculesValidPrefix[i].data());
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

//     qDebug("v %i, usec %i, isset %i",v,tv.tv_usec,FD_ISSET(m_iFd, &fdset));

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
        //qDebug("type %i, code %i, value %i",ev.type,ev.code,ev.value);
        //qDebug("type %i, code %i, value %i, v is %5.3f",ev.type,ev.code,ev.value,v);

        switch(ev.type)
        {
        case EV_ABS:
            //qDebug("code %i",ev.code);
            int iDiff;
            double dDiff;

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
                //qDebug("");
                sendEvent(PitchChange("Left", ev.value, m_iPitchLeft, m_iPitchOffsetLeft), m_pControlObjectLeftPitch);
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
                //qDebug("Left Jog - ev.value %i, m_iJogLeft %i, idiff %i, dDiff %5.3f",ev.value, m_iJogLeft, iDiff, dDiff);
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
                //qDebug("R Volume %5.3f",ev.value/d1);
                sendEvent(ev.value/d1, m_pControlObjectRightVolume);
                break;
            case kiHerculesRightPitch:
                //qDebug("");
                sendEvent(PitchChange("Right", ev.value, m_iPitchRight, m_iPitchOffsetRight), m_pControlObjectRightPitch);
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
//                    qDebug("Right Jog - ev.value %i, m_iJogRight %i, idiff %i, dDiff %5.3f",ev.value, m_iJogRight, iDiff, dDiff);
                m_iJogRight = ev.value;
                sendEvent(dDiff, m_pControlObjectRightJog);
                break;
            case kiHerculesCrossfade:
                //qDebug("(ev.value+1)/2.0f: %f", (ev.value+1)/2.0f);
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
                    sendButtonEvent(!m_pControlObjectLeftBtnPitchBendMinus->get(), m_pControlObjectLeftBtnPitchBendMinus);
                    break;
                case kiHerculesRightBtnPitchBendMinus:
                    sendButtonEvent(true, m_pControlObjectRightBtnPitchBendMinus);
                    break;

                case kiHerculesLeftBtnPitchBendPlus:
                    sendButtonEvent(!m_pControlObjectLeftBtnPitchBendPlus->get(), m_pControlObjectLeftBtnPitchBendPlus);
                    break;
                case kiHerculesRightBtnPitchBendPlus:
                    sendButtonEvent(true, m_pControlObjectRightBtnPitchBendPlus);
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
                      led_write(kiHerculesLedLeftHeadphone, !m_pControlObjectLeftBtnHeadphone->get());
                      sendButtonEvent(!m_bHeadphoneLeft, m_pControlObjectLeftBtnHeadphone);
                    }
                    break;
                case kiHerculesRightBtnHeadphone:
                    if (ev.value) {
                      led_write(kiHerculesLedRightHeadphone, !m_pControlObjectRightBtnHeadphone->get());
                      sendButtonEvent(!m_bHeadphoneRight, m_pControlObjectRightBtnHeadphone);
                    }
                    break;

                case kiHerculesLeftBtnAutobeat:
                    sendButtonEvent(true, m_pControlObjectLeftBtnAutobeat);
                    m_bSyncLeft = !m_bSyncLeft;
//                     led_write(kiHerculesLedLeftSync, m_bSyncLeft);
                    break;
                case kiHerculesRightBtnAutobeat:
                    sendButtonEvent(true, m_pControlObjectRightBtnAutobeat);
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
                    qDebug("left fx %i,%i,%i",m_iLeftFxMode==0,m_iLeftFxMode==1,m_iLeftFxMode==2);
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
//     if (bOn) qDebug("true");
//     else qDebug("false");

    struct input_event ev;
    memset(&ev, 0, sizeof(struct input_event));

    //ev.type = EV_LED;
    ev.type = 0x0000;
    ev.code = iLed;
    if (bOn)
        ev.value = 3;
    else
        ev.value = 0;

    //qDebug("Hercules: led_write(iLed=%d, bOn=%d)", iLed, bOn);

    if (write(m_iFd, &ev, sizeof(struct input_event)) != sizeof(struct input_event))
        qDebug("Hercules: write(): %s", strerror(errno));
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
        led_write(kiHerculesLedLeftSync, false);
        led_write(kiHerculesLedRightSync, false);
    }
}

double HerculesLinux::PitchChange(const QString ControlSide, const int ev_value, int &m_iPitchPrevious, int &m_iPitchOffset) {
    // Handle the initial event from the Hercules and set pitch to default of 0% change
    if (m_iPitchPrevious < 0) {
        m_iPitchOffset = ev_value;
        m_iPitchPrevious = 64;
        return m_iPitchPrevious;
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
}

