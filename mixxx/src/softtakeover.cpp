/***************************************************************************
                          softtakeover.cpp  -  description
                          ----------------
    begin                : Sat Mar 26 2011
    copyright            : (C) 2011 by Sean M. Pappalardo
    email                : spappalardo@mixxx.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "softtakeover.h"
#include <math.h>  // for fabs()
#include <qdatetime.h>

// HACK: remove this after Control 2.0 is here
#include "controlpotmeter.h"

// qint64 currentTimeMsecs() {
uint SoftTakeover::currentTimeMsecs() {
    QDateTime currDT = QDateTime::currentDateTime();

    // toMSecsSinceEpoch() is preferred since it's only one QDateTime call but
    //  it requires Qt 4.7. Use it instead when something more substantial
    //  elsewhere in Mixxx also requires Qt 4.7.
    //qint64 t = dt.toMSecsSinceEpoch();  // Requires Qt 4.7
    uint t = currDT.toTime_t()*1000+currDT.toString("zzz").toUInt();
    return t;
}

// For legacy Controls
void SoftTakeover::enable(QString group, QString name) {
    MixxxControl mixxxControl = MixxxControl(group,name);
    enable(mixxxControl);
}

void SoftTakeover::enable(MixxxControl control) {
    if (!m_times.contains(control)) m_times.insert(control,currentTimeMsecs());
}

// For legacy Controls
void SoftTakeover::disable(QString group, QString name) {
    MixxxControl mixxxControl = MixxxControl(group,name);
    disable(mixxxControl);
}

void SoftTakeover::disable(MixxxControl control) {
    m_times.remove(control);
}

// For legacy Controls
bool SoftTakeover::ignore(QString group, QString name, float newValue) {
    MixxxControl mixxxControl = MixxxControl(group,name);
    return ignore(mixxxControl,newValue);
}

bool SoftTakeover::ignore(MixxxControl mc, float newValue, bool midiVal) {
    bool ignore = false;
    QString message;
    if (m_times.contains(mc)) {
        // We only want to ignore the MIDI controller when all of the following are true:
        //  - its new value is far away from the MixxxControl
        //  - it's been awhile since the last MIDI message for this control affected it
        
        float threshold = 3;
        
        ControlObject* temp = ControlObject::getControl(
            ConfigKey(mc.getControlObjectGroup(), mc.getControlObjectValue()));
        
        double maxValue, minValue;
        
        if (!midiVal) {
            // HACK until we have Control 2.0. It can't come soon enough...
            ControlPotmeter* cpo = dynamic_cast<ControlPotmeter*>(temp);    // for getMax/getMin
            if (cpo != NULL) {
                maxValue = cpo->getMax();
                minValue = cpo->getMin();
            }
            // End hack
            
            double scaleFactor = (fabs(maxValue)-fabs(minValue))/128;
            threshold = scaleFactor*3;
        }
        
        double oldValue;
        if (midiVal) oldValue = temp->GetMidiValue();
        else oldValue = temp->get();
        double difference = oldValue - newValue;
        
        uint currentTime = currentTimeMsecs();
        if (fabs(difference)>threshold
            && (currentTime - m_times.value(mc)) > 50) {
            ignore = true;
        }
        if (!ignore) {
            //  Update the time only if the value is not ignored
            m_times.insert(mc,currentTime); // Replaces any previous value for this MixxxControl
        }
    }
    return ignore;
}
