#ifndef TONAL_TONALANALYSER_H
#define TONAL_TONALANALYSER_H

#include "analyser.h"
#include "ChordExtractor.hxx"

class TonalAnalyser : public Analyser {

public:
	TonalAnalyser();
	void initialise(TrackInfoObject* tio);
	void process(const CSAMPLE *pIn, const int iLen);
	void finalise(TrackInfoObject* tio);

private:
	float m_time;

	Simac::ChordExtractor m_ce;
};

#endif
