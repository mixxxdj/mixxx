/*
    Copyright (c) 2005 Centre for Digital Music ( C4DM )
                       Queen Mary Univesrity of London

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
 */

#ifndef GETKEYMODE_H
#define GETKEYMODE_H


#include "maths/MathAliases.h"
#include "dsp/rateconversion/Decimator.h"
#include "dsp/chromagram/Chromagram.h"


class GetKeyMode  
{
public:
	GetKeyMode( int sampleRate, float tuningFrequency,
		    fl_t hpcpAverage, fl_t medianAverage );

	virtual ~GetKeyMode();

	int process( fl_t* PCMData );

	fl_t krumCorr( fl_t* pData1, fl_t* pData2, unsigned int length );

	unsigned int getBlockSize() { return m_ChromaFrameSize*m_DecimationFactor; }
	unsigned int getHopSize() { return m_ChromaHopSize*m_DecimationFactor; }

	fl_t* getChroma() { return m_ChrPointer; }
	unsigned int getChromaSize() { return m_BPO; }

	fl_t* getMeanHPCP() { return m_MeanHPCP; }

	fl_t *getKeyStrengths() { return m_keyStrengths; }

	bool isModeMinor( int key ); 

protected:

	fl_t m_hpcpAverage;
	fl_t m_medianAverage;
	unsigned int m_DecimationFactor;

	//Decimator (fixed)
	Decimator* m_Decimator;

	//chroma configuration
	ChromaConfig m_ChromaConfig;

	//Chromagram object
	Chromagram* m_Chroma;

	//Chromagram output pointer
	fl_t* m_ChrPointer;

	//Framesize
	unsigned int m_ChromaFrameSize;
	//Hop
	unsigned int m_ChromaHopSize;
	//Bins per octave
	unsigned int m_BPO;


	unsigned int m_ChromaBuffersize;
	unsigned int m_MedianWinsize;
	
	unsigned int m_bufferindex;
	unsigned int m_ChromaBufferFilling;
	unsigned int m_MedianBufferFilling;
	

	fl_t* m_DecimatedBuffer;
	fl_t* m_ChromaBuffer;
	fl_t* m_MeanHPCP;

	fl_t* m_MajCorr;
	fl_t* m_MinCorr;
	fl_t* m_Keys;
	int* m_MedianFilterBuffer;
	int* m_SortedBuffer;

	fl_t *m_keyStrengths;
};

#endif // !defined GETKEYMODE_H
