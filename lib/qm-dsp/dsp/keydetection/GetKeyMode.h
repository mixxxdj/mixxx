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


#include "dsp/rateconversion/Decimator.h"
#include "dsp/chromagram/Chromagram.h"


class GetKeyMode  
{
public:
	GetKeyMode( int sampleRate, float tuningFrequency,
		    double hpcpAverage, double medianAverage );

	virtual ~GetKeyMode();

	int process( double* PCMData );

	double krumCorr( double* pData1, double* pData2, unsigned int length );

	unsigned int getBlockSize() { return m_ChromaFrameSize*m_DecimationFactor; }
	unsigned int getHopSize() { return m_ChromaHopSize*m_DecimationFactor; }

	double* getChroma() { return m_ChrPointer; }
	unsigned int getChromaSize() { return m_BPO; }

	double* getMeanHPCP() { return m_MeanHPCP; }

	double *getKeyStrengths() { return m_keyStrengths; }

	bool isModeMinor( int key ); 

protected:

	double m_hpcpAverage;
	double m_medianAverage;
	unsigned int m_DecimationFactor;

	//Decimator (fixed)
	Decimator* m_Decimator;

	//chroma configuration
	ChromaConfig m_ChromaConfig;

	//Chromagram object
	Chromagram* m_Chroma;

	//Chromagram output pointer
	double* m_ChrPointer;

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
	

	double* m_DecimatedBuffer;
	double* m_ChromaBuffer;
	double* m_MeanHPCP;

	double* m_MajCorr;
	double* m_MinCorr;
	double* m_Keys;
	int* m_MedianFilterBuffer;
	int* m_SortedBuffer;

	double *m_keyStrengths;
};

#endif // !defined GETKEYMODE_H
