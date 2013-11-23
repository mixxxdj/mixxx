#include "tonalanalyser.h"
#include "DiscontinuousSegmentation.hxx"
#include "../segmentation.h"
#include "../trackinfoobject.h"

#include <QDebug>
#include <QString>

TonalAnalyser::TonalAnalyser() :
		m_ce() {

    m_bCanRun = true;
	m_ce.filterInertia( 0.7 );
	m_ce.enableTunning( true );
	m_ce.enablePeakWindowing( true );
	m_ce.hopRatio( 8.0 );
	m_ce.segmentationMethod( 1 );
}

void TonalAnalyser::finalise(TrackInfoObject* tio) {
    if(!m_bCanRun)
        return;

	CLAM::DiscontinuousSegmentation segmentation = m_ce.segmentation();

	Segmentation<QString> segs;

	for (unsigned i=0; i<segmentation.onsets().size(); i++)
	{
		unsigned chordIndex = m_ce.chordIndexes()[i];
		std::string chordName = m_ce.root(chordIndex) + " " + m_ce.mode(chordIndex);
		//segmentation.setLabel(i,chordName);
		segs.addSeg(segmentation.onsets()[i], segmentation.offsets()[i], chordName.c_str());
		//qDebug() << "Got chord " << chordName.c_str() << " at " << segmentation.onsets()[i] << " until " << segmentation.offsets()[i];
	}

	m_ce.clear();

	tio->setChordData(segs);

}

void TonalAnalyser::initialise(TrackInfoObject* tio, int sampleRate, int totalSamples) {
	m_time = 0;
    if(tio->getChannels() == 1) {
        m_bCanRun = false;
    } else
        m_bCanRun = true;
}

void TonalAnalyser::process(const CSAMPLE *pIn, const int iLen) {

    if(!m_bCanRun)
        return;
    
	m_time += (iLen/2)/44100.0f;

	CSAMPLE* mono = new CSAMPLE[32768];

	for (int i = 0; i < iLen/2; i++) {
		mono[i] = 0.5f*(pIn[i*2] + pIn[i*2+1]);
	}

	m_ce.doIt(mono, m_time);

	CLAM::DiscontinuousSegmentation segmentation = m_ce.segmentation();	
	segmentation.dragOffset(segmentation.onsets().size()-1, m_time);

	// ??
	//_currentTime += _implementation->hop()/44100.0;

}
