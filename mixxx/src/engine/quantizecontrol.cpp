// QuantizeControl.cpp
// Created on Sat 5, 2011
// Author: pwhelan

#include <QtDebug>
#include <QObject>

#include "controlobject.h"
#include "configobject.h"
#include "controlpushbutton.h"
#include "cachingreader.h"
#include "engine/quantizecontrol.h"
#include "engine/enginecontrol.h"
#include "mathstuff.h"

QuantizeControl::QuantizeControl(const char * _group,
                               ConfigObject<ConfigValue> * _config,
                               CachingReader *reader)
        : EngineControl(_group, _config) {

    m_dQuantizePrevBeat= -1;

    m_pReader = reader;
    connect(m_pReader, SIGNAL(trackLoaded(TrackPointer, int, int)),
            this, SLOT(slotTrackLoaded(TrackPointer, int, int)),
            Qt::DirectConnection);

    m_pCOQuantizeEnabled = new ControlPushButton(ConfigKey(_group, "quantize"));
    m_pCOQuantizeEnabled->set(0.0f);
    m_pCOQuantizeBeat = new ControlObject(ConfigKey(_group, "quantize_beat"));
    m_pCOQuantizeBeat->set(0.0f);
}

QuantizeControl::~QuantizeControl() {
}

void QuantizeControl::slotTrackLoaded(TrackPointer tio, 
                            int iTrackSampleRate, int iTrackNumSamples)
{
    m_pTrack = tio;
    m_pBeats = m_pTrack->getBeats();
    connect(m_pTrack.data(), SIGNAL(beatsUpdated()), this, SLOT(slotBeatsUpdated()));
}

void QuantizeControl::slotBeatsUpdated()
{
    m_pBeats = m_pTrack->getBeats();
}

double QuantizeControl::process(const double dRate,
                               const double currentSample,
                               const double totalSamples,
                               const int iBufferSize) {

    m_iCurrentSample = currentSample;
    if (!even(m_iCurrentSample))
        m_iCurrentSample--;

    if ( m_pCOQuantizeEnabled->get() ) {
        	if ((m_iCurrentSample > m_pCOQuantizeBeat->get()) || (m_iCurrentSample <= m_dQuantizePrevBeat)) {
            double newpos;
            
        	    if ( ! m_pBeats ) {
        	        qDebug() << "No Beats to Quantize With";
        	        return kNoTrigger;
        	    }

        	    newpos = floorf(m_pBeats->findNextBeat(m_iCurrentSample));
        	    if ( !even(newpos))
        	        newpos--;
        	    
        	    m_pCOQuantizeBeat->set(newpos);
        	    m_dQuantizePrevBeat = m_pBeats->findPrevBeat(m_iCurrentSample);
        	}
    }
    	
    return kNoTrigger;
}

