/***************************************************************************
bpmreceiver.h  -  The base class for objects that need to receive
                  notifications from the bpm detection.
-------------------
begin                : Sat, Aug 17, 2007
copyright            : (C) 2007 by Micah Lee
email                : snipexv@gmail.com
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef BPMRECEIVER_H
#define BPMRECEIVER_H

class TrackInfoObject;

/**
  * Base class for recieving BPM detection information
  *
  *@author Micah Lee
  */

class BpmReceiver 
{
public:
    BpmReceiver(){}
    virtual ~BpmReceiver(){}
    /** Sent from the bpm detection when the detection progress has changed for a 
        particular TrackInfoObject. */
    virtual void setProgress(TrackInfoObject *tio, double progress) = 0;

    /** Sent from the bpm detection when processing of this TrackInfoObject is complete.
        Also notifies whether or not the detection was succesful. */

    /** NOTE: The TrackInfoObject will already have the new BPM and confirm status before
              this method is called.*/
    virtual void setComplete(TrackInfoObject *tio, bool failed) = 0;
};

#endif
