#include "controlrotary.h"

const char ControlRotary::graycodetable[256] =  {128, 56, 40, 55, 24, 128, 39, 52, 8, 57, 128, 128, 23, 128, 36, 13, 120, 128, 41, 54, 128, 128, 128, 53, 7, 128, 128, 128, 20, 19, 125, 18, 104, 105, 128, 128, 25, 106, 38, 128, 128, 58, 128, 128, 128, 128, 37, 14, 119, 118, 128, 128, 128, 107, 128, 128, 4, 128, 3, 128, 109, 108, 2, 1, 88, 128, 89, 128, 128, 128, 128, 51, 9, 10, 90, 128, 22, 11, 128, 12, 128, 128, 42, 43, 128, 128, 128, 128, 128, 128, 128, 128, 21, 128, 126, 127, 103, 128, 102, 128, 128, 128, 128, 128, 128, 128, 91, 128, 128, 128, 128, 128, 116, 117, 128, 128, 115, 128, 128, 128, 93, 94, 92, 128, 114, 95, 113, 0, 72, 71, 128, 68, 73, 128, 128, 29, 128, 70, 128, 69, 128, 128, 35, 34, 121, 128, 122, 128, 74, 128, 128, 30, 6, 128, 123, 128, 128, 128, 124, 17, 128, 128, 128, 67, 26, 128, 27, 28, 128, 59, 128, 128, 128, 128, 128, 15, 128, 128, 128, 128, 128, 128, 128, 128, 5, 128, 128, 128, 110, 128, 111, 16, 87, 84, 128, 45, 86, 85, 128, 50, 128, 128, 128, 46, 128, 128, 128, 33, 128, 83, 128, 44, 75, 128, 128, 31, 128, 128, 128, 128, 128, 128, 128, 32, 100, 61, 101, 66, 128, 62, 128, 49, 99, 60, 128, 47, 128, 128, 128, 48, 77, 82, 78, 65, 76, 63, 128, 64, 98, 81, 79, 80, 97, 96, 112, 128};

/* -------- ------------------------------------------------------
   Purpose: Creates a new rotary encoder
   Input:   n - name
            midino - number of the midi controller.
            midicontroller - pointer to the midi controller.
   -------- ------------------------------------------------------ */
ControlRotary::ControlRotary(ConfigKey key, ControlPushButton *playbutton) : ControlPotmeter(key,0.,1.) // ????
{
    play = playbutton;
    direction = 1; // arbitrary
    gettimeofday(&oldtime,0);
    value = 0;
    counter = 0.;
    emit valueChanged(value);
}

/* -------- ------------------------------------------------------
   Purpose: Sets the position of the encoder. Called from midi
            and given the gray code as input. Calculates the
            real position from the gray code.
   Input:   the gray code.
   -------- ------------------------------------------------------ */
// This member is not properly overloading the one in controlobject. Therefore it is 
// never called, and the graycode translation has been moved to slotSetPosition.
void ControlRotary::slotSetPositionMidi(int _newpos)
{
    qDebug("rot");
    //int newpos = graycodetable[(int)(unsigned char)_newpos];
    slotSetPosition(_newpos);
    //emit updateGUI(newpos);
}

void ControlRotary::slotSetPosition(int newpos)
{
    // get position from gray code
    newpos = graycodetable[(int)(unsigned char)newpos];

    if ((newpos != -128) && (newpos != position))
    {
        short change = newpos-position;
        // Check for passing through 0 and 127:
        if (change > 100)
            change = 128-change;
        else if (change < -100)
            change += 126;

        // Check for a change in direction:
        short newdirection = sign(change);
        if ((newdirection==direction) || (abs(change)>1))
        {
            direction = newdirection;
            // Get the time:
            timeval newtime;
            gettimeofday(&newtime,0);
            long deltasec = newtime.tv_sec - oldtime.tv_sec;
            long deltamsec = (newtime.tv_usec - oldtime.tv_usec)/1000;
            if (deltasec > 2)
                value = 0.;
            else if (deltasec*1000+deltamsec > 0)
                value = (FLOAT_TYPE)change / (FLOAT_TYPE)(deltasec*1000+deltamsec);

//            cout << "Wheel: new position " << (int)newpos << " to " <<(int)position << ", velocity " <<
//            setw(8) << value <<"\n.";
//            cout << "deltat " << deltasec << ", " << deltamillisec << "\n";
//            cout << "millitime " << newtime.millitm << " old " << oldtime.millitm << "\n";

            oldtime = newtime;
            position = newpos;
            counter = 4.*(FLOAT_TYPE)(deltasec*1000+deltamsec);

            if (play->getValue()==off)
                value *= 4.;
            emit valueChanged(value);
        }
    }
}

void ControlRotary::updatecounter(int samples, int SRATE)
{
    if (counter > 0.)
    {
        counter -= (FLOAT_TYPE)samples/(FLOAT_TYPE)(SRATE/1000);
        if (counter <= 0.)
        {
            //cout << "Stopping wheel.\n";
            value = 0.;
            emit valueChanged(value);
        }
    }
}

short ControlRotary::sign(short x)
{
    if (x < 0)
        return -1;
    else
        if (x==0)
            return 0;
        else
            return 1;
}

void ControlRotary::slotSetValue(int newvalue)
{
    FLOAT_TYPE d = 400.;
    if (play->getValue()==off)
        d = 100.;

    setValue(((FLOAT_TYPE)newvalue-49.)/d);
}
