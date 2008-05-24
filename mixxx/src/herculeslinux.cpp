/***************************************************************************
              herculeslinux.cpp  -  description
                 -------------------
begin                : Tue Feb 22 2005
copyright            : (C) 2005 by Tue Haste Andersen
email                : haste@diku.dk
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/*
 * =========Version History=============
 * Version 1.6.0 --- Dec 5 2007 --- Garth Dahlstrom <ironstorm@users.sf.net>
 * - Developed on Hercule DJ Console MK2 - MK1 may also work, untested
 * - Fixed control scale to be 0.0 to 4.0 on bass, mid, treb, etc
 * - Fixed pitch knob to be from -1.0 to 1.0
 * - Fixed volume sliders to be from 0.0 to 1.0
 * - Fixed crossfader to be -1.0 to 1.0
 * - Fixed Play buttons to send the NOT (!) value whatever the current playback status is
 * - Button LEDs light up when pressed
 * - Thanks to Thomas' patch we have a working headphone switch on the Mk2
 *
 * - REMOVED legacy dev/file based support used in Mixxx 1.5.0 (Alberts MP3 Control won't work unless Mel can get it going with libdjconsole)
 *	The last straw why this code has now been removed is with the recent engine changes to the meanings of all of the control values,
 *	this block of code would need a large update and a bit of testing to get everything working in the manner it used to...
 *
 * - TODO: fix the JogDials to do something like a scratch (they do a temporary pitch bend now :\)
 * - TODO: reset m_iPitchOffsetLeft or m_iPitchOffsetRight value to -9999 when mouse/keyboard adjusts pitch slider (see PitchChange method header)
 */

#define __THOMAS_HERC__
#include "herculeslinux.h"
#include <string.h>
#include <QtDebug>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include "controlobject.h"
#include "controlobjectthread.h"
#include "mathstuff.h"
#include "rotary.h"
//Added by qt3to4:
#include <Q3ValueList>                            // used by the old herc code
#include <QTimer>

#include <math.h>

//#define __HERCULES_STUB__ // Define __HERCULES_STUB__ disable Herc USB mode to be able to test Hercules in MIDI mode.
#ifndef __HERCULES_STUB__
  // If DJCONSOLE=1 -> use libDJConsole implementation
  // If DJCONSOLE_LEGACY_CODE -> use legacy devfile based implementation
  // Otherwise stub-out HerculesLinux object
  #ifndef __LIBDJCONSOLE__
    #ifdef __DJCONSOLE_LEGACY__
      // include devfile based djconsole legacy code
      #include "herculeslinuxlegacy.cpp"
    #else
      #define __HERCULES_STUB__  // stub is implied because neither libDJConsole nor Legacy code implementations are specified.
    #endif
  #endif
#endif

#ifdef __HERCULES_STUB__
/************** Stub ***********/
// Stub of HerculesLinux object.
HerculesLinux::HerculesLinux() : Hercules() {}
HerculesLinux::~HerculesLinux() {}
void HerculesLinux::closedev() {}
void HerculesLinux::run() {}
bool HerculesLinux::opendev() { return 1; }
int HerculesLinux::opendev(int iId) { return 1; }
void HerculesLinux::getNextEvent() {}
void HerculesLinux::led_write(int iLed, bool bOn) {}
void HerculesLinux::selectMapping(QString qMapping) {}
double HerculesLinux::PitchChange(const QString ControlSide, const int ev_value, int &m_iPitchPrevious, int &m_iPitchOffset) { return 0; }
/************** End Stub ***********/
#else //__HERCULES_STUB__

#ifdef __LIBDJCONSOLE__

#ifndef MAIN_VOL
// TODO: Move const block to libDJConsole start
// ------------ start libDJConsole block ----------------
const int MONITOR_DECK_A = 51;
const int MONITOR_DECK_B = 52;
const int MONITOR_MIX = 58;
const int MONITOR_SPLIT = 54;

// Stubs for Herc Rmx Controls
const int GAIN_A = 902;
const int GAIN_B = 903;
const int MAIN_VOL = 900;
const int BALANCE = 901;

// Stubs for Herc Rmx Buttons
const int SCRATCH = 601;
const int LEFT_KILL_HIGH = 1032;
const int LEFT_KILL_MID = 1064;
const int LEFT_KILL_BASS = 1128;
const int RIGHT_KILL_HIGH = 2032;
const int RIGHT_KILL_MID = 2064;
const int RIGHT_KILL_BASS = 2128;
const int LOAD_DECK_A = 302;
const int LOAD_DECK_B = 332;
const int LEFT_PITCH_RESET = 301; // TODO: Implement Left Pitch Reset
const int RIGHT_PITCH_RESET = 4128; // TODO: Implement Right Pitch Reset

const int UP = 602;
const int DOWN = 604;
const int LEFT = 608;
const int RIGHT = 616;

const int LEFT_STOP = 216;
const int RIGHT_STOP = 516;
const int LEFT_4 = 108;
const int LEFT_5 = 116;
const int LEFT_6 = 132;
const int RIGHT_4 = 408;
const int RIGHT_5 = 416;
const int RIGHT_6 = 432;
#endif

QHash<long, ControlObjectThread *> buttonMapping;
QHash<long, ControlObjectThread *> nonMIDIbuttonMapping;

// Check if the control is a button (used to filter out button released events)
// FIXME: This code will have to be rewritten to use something non-QT when moved to libDJConsole.
const QSet<int> non_button_controls = QSet<int>::QSet() << XFADER << BALANCE << MAIN_VOL << MONITOR_SPLIT << MONITOR_MIX \
  << MONITOR_DECK_A << GAIN_A << LEFT_VOL << LEFT_JOG << LEFT_BASS << LEFT_MID << LEFT_HIGH << LEFT_PITCH \
  << MONITOR_DECK_B << GAIN_B << RIGHT_VOL << RIGHT_JOG << RIGHT_BASS << RIGHT_MID << RIGHT_HIGH << RIGHT_PITCH;

static bool isButton(const int controlId) {
    return !non_button_controls.contains(controlId);
}

// -------------- end libDJConsole block ----------------

static void console_event(void * c, int code, int value) {
    HerculesLinux * f=(HerculesLinux *)c;
    f->consoleEvent(code, value);
}


HerculesLinux::HerculesLinux() : Hercules() {
    djc = 0;                                      // set to zero to force detection on the first run through.
    m_iPitchLeft = -1;
    m_iPitchRight = -1;

    qDebug() << "HerculesLinux: Constructor called";

    // m_iFd = -1; // still needed?
    m_iId = -1;
    m_iJogLeft = 0.;
    m_iJogRight = 0.;

    m_dJogLeftOld = -1;
    m_dJogRightOld = -1;

    jogLibraryScrolling = false;

    m_bHeadphoneLeft = false;
    m_bHeadphoneRight = false;

    #ifdef __THOMAS_HERC__
    m_iHerculesHeadphonesSelection = 1;
    #endif

    buttonMapping[LEFT_KILL_HIGH] = m_pControlObjectLeftKillHigh;
    buttonMapping[RIGHT_KILL_HIGH] = m_pControlObjectRightKillHigh;
    buttonMapping[LEFT_KILL_MID] = m_pControlObjectLeftKillMid;
    buttonMapping[RIGHT_KILL_MID] = m_pControlObjectRightKillMid;
    buttonMapping[LEFT_KILL_BASS] = m_pControlObjectLeftKillBass;
    buttonMapping[RIGHT_KILL_BASS] = m_pControlObjectRightKillBass;
    buttonMapping[LEFT_PLAY] = m_pControlObjectLeftBtnPlay;
    buttonMapping[RIGHT_PLAY] = m_pControlObjectRightBtnPlay;
    buttonMapping[LEFT_PITCH_DOWN] = m_pControlObjectLeftBtnPitchBendMinus;
    buttonMapping[LEFT_PITCH_UP] = m_pControlObjectLeftBtnPitchBendPlus;
    buttonMapping[RIGHT_PITCH_DOWN] = m_pControlObjectRightBtnPitchBendMinus;
    buttonMapping[RIGHT_PITCH_UP] = m_pControlObjectRightBtnPitchBendPlus;
    buttonMapping[LEFT_AUTO_BEAT] = m_pControlObjectLeftBtnAutobeat;
    buttonMapping[RIGHT_AUTO_BEAT] = m_pControlObjectRightBtnAutobeat;

    buttonMapping[LEFT_MONITOR] = m_pControlObjectLeftBtnHeadphone;
    buttonMapping[RIGHT_MONITOR] = m_pControlObjectRightBtnHeadphone;

    buttonMapping[LEFT_CUE] = m_pControlObjectLeftBtnCue;
    buttonMapping[RIGHT_CUE] = m_pControlObjectRightBtnCue;

    nonMIDIbuttonMapping[UP] = m_pControlObjectUp;
    nonMIDIbuttonMapping[DOWN] = m_pControlObjectDown;
    nonMIDIbuttonMapping[LEFT] = m_pControlObjectLeft;
    nonMIDIbuttonMapping[RIGHT] = m_pControlObjectRight;

    nonMIDIbuttonMapping[LOAD_DECK_A] = m_pControlObjectLoadDeckA;
    nonMIDIbuttonMapping[LOAD_DECK_B] = m_pControlObjectLoadDeckB;
    nonMIDIbuttonMapping[LEFT_SKIP_BACK] = m_pControlObjectLeftBtnTrackPrev;
    nonMIDIbuttonMapping[LEFT_SKIP_FORWARD] = m_pControlObjectLeftBtnTrackNext;
    nonMIDIbuttonMapping[RIGHT_SKIP_BACK] = m_pControlObjectRightBtnTrackPrev;
    nonMIDIbuttonMapping[RIGHT_SKIP_FORWARD] = m_pControlObjectRightBtnTrackNext;

}


HerculesLinux::~HerculesLinux() {}


void HerculesLinux::closedev() {}


void HerculesLinux::run() {
    #ifdef __THOMAS_HERC__
    double l;
    double r;
    bool leftJogProcessing = false;
    bool rightJogProcessing = false;
    m_pRotaryLeft->setFilterLength(4);
    m_pRotaryRight->setFilterLength(4);
    m_pRotaryLeft->setCalibration(64);
    m_pRotaryRight->setCalibration(64);
    djc->Leds.setBit(LEFT_FX, false);
    djc->Leds.setBit(LEFT_FX_CUE, false);
    djc->Leds.setBit(LEFT_LOOP, true);
    djc->Leds.setBit(RIGHT_FX, false);
    djc->Leds.setBit(RIGHT_FX_CUE, false);
    djc->Leds.setBit(RIGHT_LOOP, true);
    while( 1 ) {
        if (m_iJogLeft != 0) {
            l = m_pRotaryLeft->fillBuffer(m_iJogLeft);
            m_iJogLeft = 0;
            leftJogProcessing = true;
        } else {
            l = m_pRotaryLeft->filter(m_iJogLeft);
        }
        if (m_iJogRight != 0) {
            r = m_pRotaryRight->fillBuffer(m_iJogRight);
            m_iJogRight = 0;
            rightJogProcessing = true;
        } else {
            r = m_pRotaryRight->filter(m_iJogRight);
        }

        if (jogLibraryScrolling && (l != 0 || r != 0)) {
            if (l+r > 0) {
               sendButtonEvent(1, m_pControlObjectDown);
            } else if (l+r < 0) {
               sendButtonEvent(1, m_pControlObjectUp);
            }
            l = 0; r = 0;
            leftJogProcessing = false; rightJogProcessing = false;
        }

        if ( l != 0 || leftJogProcessing) {
            //qDebug() << "sendEvent(" << l << ", m_pControlObjectLeftJog)";
            if (scratchMode) {
                sendEvent(l, m_pControlObjectLeftScratch);
            } else {
                sendEvent(l, m_pControlObjectLeftJog);
            }
            if ( l == 0 ) leftJogProcessing = false;
        }
        if ( r != 0 || rightJogProcessing) {
            //qDebug() << "sendEvent(" << r << ", m_pControlObjectRightJog)";
            if (scratchMode) {
                sendEvent(r, m_pControlObjectRightScratch);
            } else {
                sendEvent(r, m_pControlObjectRightJog);
            }
            if ( r == 0 ) rightJogProcessing = false;
        }
        msleep (64);
    }
    #endif                                        // __THOMAS_HERC__
}


bool HerculesLinux::opendev() {
    qDebug() << "Starting Hercules DJ Console detection";
    if (djc == 0) {
        djc = new DJConsole();
        if (djc == 0 || !djc->detected()) {
            qDebug() << "Sorry, no love.";
            return 0;
        }

        qDebug() << "A Hercules DJ Console was detected.";

        djc->loadData();

        // All non-RMX controllers do scratching by default.
        isRMX = djc->product() >= 0xb101;
        scratchMode = (isRMX? scratchMode : true);
        qDebug() << "isRMX = " << isRMX;

        start();
        djc->setCallback(console_event, this);

        return djc->ready();

    }
    else {
        qDebug() << "Already completed detection.";
        return 1;
    }
}


int HerculesLinux::opendev(int iId) {
    return opendev();
}


static bool buttonStateLookup(int channel, QString key) {
    QString q = "[Channel" + QString::number(channel) + "]";
    // qDebug() << "q is" << q << "-- key is"<< key<< "-- value is "<< ControlObject::getControl(ConfigKey(q,key))->get();
    return ControlObject::getControl(ConfigKey(q,key))->get()==1;
}


static bool trackIsLoaded(int channel) {
    return ControlObject::getControl(ConfigKey("[Channel" + QString::number(channel) + "]","duration")) != NULL && ControlObject::getControl(ConfigKey("[Channel" + QString::number(channel) + "]","duration"))->get() > 0;
}


void HerculesLinux::consoleEvent(int first, int second) {
    // qDebug() << __FILE__ << ":" << __LINE__<<"1st: "<< first <<"2nd: "<< second;

    bool buttonPressed = isButton(first) && second != 0;

    if (isButton(first)) {
        if (buttonMapping.contains(first)) {
            sendButtonEvent(buttonPressed, buttonMapping[first]->getControlObject());
        } else if (nonMIDIbuttonMapping.contains(first)) {
            qDebug() << "nonMIDIbuttonMapping:"<< first;
            sendButtonEvent(buttonPressed, nonMIDIbuttonMapping[first]);
        }
        // Buttons - Special Cases
        if (buttonPressed) {
            switch(first) {
                case UP:
                case DOWN: jogLibraryScrolling = true; break;
                case SCRATCH: //TODO: move this into "Master" controlObject
                    scratchMode = !scratchMode;
                    qDebug() << "scratchMode = " << scratchMode;
                    break;
                    //	case LEFT: ; break;
                    //	case RIGHT: ; break;
		case LEFT_STOP: if (buttonStateLookup(1, buttonMapping[LEFT_PLAY]->getControlObject()->getKey().item)) sendButtonEvent(buttonPressed, buttonMapping[LEFT_PLAY]->getControlObject()); break;
		case RIGHT_STOP: if (buttonStateLookup(2, buttonMapping[RIGHT_PLAY]->getControlObject()->getKey().item)) sendButtonEvent(buttonPressed, buttonMapping[RIGHT_PLAY]->getControlObject()); break;
                case LEFT_1: if (!isRMX) m_pRotaryLeft->setCalibration(512); break;
                case LEFT_2: if (!isRMX) m_pRotaryLeft->setCalibration(256); break;
                case LEFT_3: if (!isRMX) m_pRotaryLeft->setCalibration(64); break;
                case RIGHT_1: if (!isRMX) m_pRotaryRight->setCalibration(512);
                case RIGHT_2: if (!isRMX) m_pRotaryRight->setCalibration(256); break;
                case RIGHT_3: if (!isRMX) m_pRotaryRight->setCalibration(64); break;
                //                case RIGHT_MONITOR: sendButtonEvent(m_bHeadphoneLeft = false, m_pControlObjectLeftBtnHeadphone); break; // m_bHeadphoneRight = !m_bHeadphoneRight; break;
                //                case LEFT_MONITOR: sendButtonEvent(m_bHeadphoneRight = false, m_pControlObjectRightBtnHeadphone); break; // m_bHeadphoneLeft = !m_bHeadphoneLeft; break;
            }
        } else {
            switch(first) {
                case UP:
                case DOWN:
                case LEFT:
                case RIGHT: jogLibraryScrolling = false; break;
            }
        }
    } else { // Not a button
        int iDiff = 0;

        // GED's magic formula -- no longer used.
        // double v = ((second+1)/(4.- ((second>((7/8.)*256))*((second-((7/8.)*256))*1/16.))));

        // Albert's http://zunzun.com/ site saves the day by solving our data points to this new magical formula...
        double magic = (0.733835252488 * tan((0.00863901501308 * second) - 4.00513109039)) + 0.887988233294;

        double divisor = 256.;
        double d1 = divisor-1;
        double d2 = (divisor/2)-1;

        switch (first) {
            case BALANCE: sendEvent((second-d2)/d2, m_pControlObjectBalance); break;
            case GAIN_A: sendEvent(magic, m_pControlObjectGainA); break;
            case GAIN_B: sendEvent(magic, m_pControlObjectGainB); break;
            case MAIN_VOL: sendEvent(magic, m_pControlObjectMainVolume); break;
            case LEFT_VOL: sendEvent(second/d1, m_pControlObjectLeftVolume); break;
            case RIGHT_VOL: sendEvent(second/d1, m_pControlObjectRightVolume); break;
            case XFADER: sendEvent((second-d2)/d2, m_pControlObjectCrossfade); break;
            case RIGHT_HIGH: sendEvent(magic, m_pControlObjectRightTreble); break;
            case RIGHT_MID: sendEvent(magic, m_pControlObjectRightMiddle); break;
            case RIGHT_BASS: sendEvent(magic, m_pControlObjectRightBass); break;
            case LEFT_HIGH: sendEvent(magic, m_pControlObjectLeftTreble); break;
            case LEFT_MID: sendEvent(magic, m_pControlObjectLeftMiddle); break;
            case LEFT_BASS: sendEvent(magic, m_pControlObjectLeftBass); break;
            case LEFT_MASTER_TEMPO: sendEvent(0, m_pControlObjectLeftBtnMasterTempo); break;
            case RIGHT_MASTER_TEMPO: sendEvent(0, m_pControlObjectRightBtnMasterTempo); break;
            case LEFT_JOG:
                iDiff = 0;
                if (m_dJogLeftOld>=0) {
                    iDiff = second-m_dJogLeftOld;
                }
                if (iDiff<-200) {
                    iDiff += 256;
                } else if (iDiff>200) {
                    iDiff -= 256;
                }
                m_dJogLeftOld = second;
                m_iJogLeft += (double) iDiff;     // here goes the magic
                break;
            case RIGHT_JOG:
                iDiff = 0;
                if (m_dJogRightOld>=0) {
                    iDiff = second-m_dJogRightOld;
                }
                if (iDiff<-200) {
                    iDiff += 256;
                } else if (iDiff>200) {
                    iDiff -= 256;
                }
                m_dJogRightOld = second;
                m_iJogRight += (double) iDiff;
                break;
            case LEFT_PITCH: sendEvent(PitchChange("Left", second, m_iPitchLeft, m_iPitchOffsetLeft), m_pControlObjectLeftPitch); break;
            case RIGHT_PITCH: sendEvent(PitchChange("Right", second, m_iPitchRight, m_iPitchOffsetRight), m_pControlObjectRightPitch); break;
            /* for the headphone select if have measured something like this on my hercules mk2
             *
             *	from state	to state	value(s)
             *	split		mix		first=102, second=8
             *	mix		split		first=103, second=4 most significant
             *	mix		split		first=100, second=1
             *	mix		split		first=101, second=2
             *	mix		deck b		first=101, second=2
             *	deck b		mix		first=102, second=8
             *	deck b		deck a		first=100, second=1
             *	deck a		deck b		first=101, second=2
             *
             *	you will see only one unique value: first=103,second=4
             *	so lets try what we learned about: (sorry, we really need a var for tracking this)
             */
            case MONITOR_SPLIT:
                if (second != 0) {
                  qDebug() << "Deck Split (mute both)";
                  m_iHerculesHeadphonesSelection = kiHerculesHeadphoneSplit;
                  sendButtonEvent(m_bHeadphoneRight = false, m_pControlObjectRightBtnHeadphone);
                  sendButtonEvent(m_bHeadphoneLeft = false, m_pControlObjectLeftBtnHeadphone);
                }
                break;
            case MONITOR_MIX:
                if (second != 0) {
                  qDebug() << "Deck MIX";
                  m_iHerculesHeadphonesSelection = kiHerculesHeadphoneMix;
                  sendButtonEvent(m_bHeadphoneRight = true, m_pControlObjectRightBtnHeadphone);
                  sendButtonEvent(m_bHeadphoneLeft = true, m_pControlObjectLeftBtnHeadphone);
                } 
                break;
            case MONITOR_DECK_B:
                if (second != 0 && m_iHerculesHeadphonesSelection != kiHerculesHeadphoneSplit) {
                  qDebug() << "Deck B";
                  m_iHerculesHeadphonesSelection = kiHerculesHeadphoneDeckB;
                  sendButtonEvent(m_bHeadphoneRight = true, m_pControlObjectRightBtnHeadphone);
                  sendButtonEvent(m_bHeadphoneLeft = false, m_pControlObjectLeftBtnHeadphone);
                }
                break;
            case MONITOR_DECK_A:
                if (second != 0 && m_iHerculesHeadphonesSelection == kiHerculesHeadphoneDeckB) {
                  qDebug() << "Deck A";
                  m_iHerculesHeadphonesSelection = kiHerculesHeadphoneDeckA;
                  sendButtonEvent(m_bHeadphoneRight = false, m_pControlObjectRightBtnHeadphone);
                  sendButtonEvent(m_bHeadphoneLeft = true, m_pControlObjectLeftBtnHeadphone);
                }
                break;
            default:
                qDebug() << "Unmapped control " << first << " = " << second;
                break;
        }

    }

    // Update LED states
    // TODO: Move Update LED states to seperate method, hook some call backs from playing engine to it.
    QHash<long, bool> leds;
    leds[SCRATCH] = scratchMode;
    leds[LEFT_CUE] = false;                       // trackIsLoaded(1) && buttonStateLookup(1, buttonMapping[LEFT_CUE]->getControlObject()->getKey().item);
    leds[RIGHT_CUE] = false;                      // trackIsLoaded(2) && buttonStateLookup(2, buttonMapping[RIGHT_CUE]->getControlObject()->getKey().item);

    leds[LEFT_PLAY] = trackIsLoaded(1) && buttonStateLookup(1, buttonMapping[LEFT_PLAY]->getControlObject()->getKey().item);
    leds[RIGHT_PLAY] = trackIsLoaded(2) && buttonStateLookup(2, buttonMapping[RIGHT_PLAY]->getControlObject()->getKey().item);
    leds[LEFT_MONITOR] = buttonStateLookup(1, buttonMapping[LEFT_MONITOR]->getControlObject()->getKey().item);
    leds[RIGHT_MONITOR] = buttonStateLookup(2, buttonMapping[RIGHT_MONITOR]->getControlObject()->getKey().item);

    leds[LEFT_MASTER_TEMPO] = false;              // TODO: LEFT BEATLOCK LEDs ... buttonStateLookup(1, "master_tempo");
    leds[RIGHT_MASTER_TEMPO] = false;             // TODO: RIGHT BEATLOCK LEDs ...  buttonStateLookup(1, "master_tempo");

    leds[LEFT_FX] = m_pRotaryLeft->getCalibration() == 512;
    leds[LEFT_FX_CUE] = m_pRotaryLeft->getCalibration() == 256;
    leds[LEFT_LOOP] = m_pRotaryLeft->getCalibration() == 64;
    leds[RIGHT_FX] = m_pRotaryRight->getCalibration() == 512;
    leds[RIGHT_FX_CUE] = m_pRotaryRight->getCalibration() == 256;
    leds[RIGHT_LOOP] = m_pRotaryRight->getCalibration() == 64;

    QHashIterator<long, bool> led(leds);
    while (led.hasNext()) {
        led.next();
        // qDebug() << "led:" << led.key() << "second:"<< second << "buttonPressed:" << buttonPressed << "led.value" << led.value();
        led_write(led.key(), (buttonPressed && (first == led.key())) || led.value());
    }
}

void HerculesLinux::getNextEvent() {}

void HerculesLinux::led_write(int iLed, bool bOn) {
  djc->Leds.setBit(iLed, bOn);
}

void HerculesLinux::selectMapping(QString qMapping) {}

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
    }
    else if (pitchAdjustStep > 0) {
        m_iPitchPrevious = 127;
    }
    else if (pitchAdjustStep < 0) {
        m_iPitchPrevious = 0;
    }

    // qDebug() << "PitchChange [" << ControlSide << "] PitchAdjust" << pitchAdjustStep <<"-> new Pitch:" << m_iPitchPrevious << " NewRangeAdjustedPitch:" <<  QString::number(((m_iPitchPrevious+1) - 64)/64.);

    // old range was 0..127
    // new range is -1.0 to 1.0
    return ((m_iPitchPrevious+1) - 64)/64.;
}
#endif
#endif //__HERCULES_STUB__
