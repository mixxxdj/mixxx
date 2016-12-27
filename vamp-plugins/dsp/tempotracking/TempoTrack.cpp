/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    QM DSP Library

    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2005-2006 Christian Landone.and Matthew Davies.

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#include "TempoTrack.h"

#include "maths/MathAliases.h"
#include "maths/MathUtilities.h"

#include <iostream>

#include <cassert>

//#define DEBUG_TEMPO_TRACK 1


#define RAY43VAL

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

TempoTrack::TempoTrack( TTParams Params )
{
    m_tempoScratch = NULL;
    m_rawDFFrame = NULL;
    m_smoothDFFrame = NULL;
    m_frameACF = NULL;
	m_smoothRCF = NULL;

    m_dataLength = 0;
    m_winLength = 0;
    m_lagLength = 0;

    m_rayparam = 0;
    m_sigma = 0;
    m_DFWVNnorm = 0;

    initialise( Params );
}

TempoTrack::~TempoTrack()
{
    deInitialise();
}

void TempoTrack::initialise( TTParams Params )
{	
    m_winLength = Params.winLength;
    m_lagLength = Params.lagLength;

    m_rayparam	 = 43.0;
    m_sigma = sqrt(3.9017);
    m_DFWVNnorm = exp( ( log( 2.0 ) / m_rayparam ) * ( m_winLength + 2 ) );

    m_rawDFFrame = new fl_t[ m_winLength ];
    m_smoothDFFrame = new fl_t[ m_winLength ];
    m_frameACF = new fl_t[ m_winLength ];
    m_tempoScratch = new fl_t[ m_lagLength ];
	m_smoothRCF = new fl_t[ m_lagLength ];


    unsigned int winPre = Params.WinT.pre;
    unsigned int winPost = Params.WinT.post;

    m_DFFramer.configure( m_winLength, m_lagLength );
	
    m_DFPParams.length = m_winLength;
    m_DFPParams.AlphaNormParam = Params.alpha;
    m_DFPParams.LPOrd = Params.LPOrd;
    m_DFPParams.LPACoeffs = Params.LPACoeffs;
    m_DFPParams.LPBCoeffs = Params.LPBCoeffs;
    m_DFPParams.winPre = Params.WinT.pre;
    m_DFPParams.winPost = Params.WinT.post;
    m_DFPParams.isMedianPositive = true;
	
    m_DFConditioning = new DFProcess( m_DFPParams );


	// these are parameters for smoothing m_tempoScratch
    m_RCFPParams.length = m_lagLength;
    m_RCFPParams.AlphaNormParam = Params.alpha;
    m_RCFPParams.LPOrd = Params.LPOrd;
    m_RCFPParams.LPACoeffs = Params.LPACoeffs;
    m_RCFPParams.LPBCoeffs = Params.LPBCoeffs;
    m_RCFPParams.winPre = Params.WinT.pre;
    m_RCFPParams.winPost = Params.WinT.post;
    m_RCFPParams.isMedianPositive = true;

    m_RCFConditioning = new DFProcess( m_RCFPParams );

}

void TempoTrack::deInitialise()
{	
    delete [] m_rawDFFrame;
	
    delete [] m_smoothDFFrame;

	delete [] m_smoothRCF;	
	
    delete [] m_frameACF;

    delete [] m_tempoScratch;

    delete m_DFConditioning;
	
	delete m_RCFConditioning;

}

void TempoTrack::createCombFilter(fl_t* Filter, unsigned int winLength, unsigned int TSig, fl_t beatLag)
{
    unsigned int i;

    if( beatLag == 0 )
    {
	for( i = 0; i < winLength; i++ )
	{    
	    Filter[ i ] = ( ( i + 1 ) / pow( m_rayparam, 2.0) ) * exp( ( -pow(( i + 1 ),2.0 ) / ( 2.0 * pow( m_rayparam, 2.0))));
	}
    }
    else
    {	
	m_sigma = beatLag/4;
	for( i = 0; i < winLength; i++ )
	{
	    fl_t dlag = (fl_t)(i+1) - beatLag;
	    Filter[ i ] =  exp(-0.5 * pow(( dlag / m_sigma), 2.0) ) / (sqrt( 2 * PI) * m_sigma);
	}
    }
}

fl_t TempoTrack::tempoMM(fl_t* ACF, fl_t* weight, int tsig)
{

    fl_t period = 0;
    fl_t maxValRCF = 0.0;
    unsigned int maxIndexRCF = 0;

    fl_t* pdPeaks;

    unsigned int maxIndexTemp;
    fl_t	maxValTemp;
    unsigned int count; 
	
    unsigned int numelem,i,j;
    int a, b;

    for( i = 0; i < m_lagLength; i++ )
	m_tempoScratch[ i ] = 0.0;

    if( tsig == 0 ) 
    {
	//if time sig is unknown, use metrically unbiased version of Filterbank
	numelem = 4;
    }
    else
    {
	numelem = tsig;
    }

#ifdef DEBUG_TEMPO_TRACK
    std::cerr << "tempoMM: m_winLength = " << m_winLength << ", m_lagLength = " << m_lagLength << ", numelem = " << numelem << std::endl;
#endif

    for(i=1;i<m_lagLength-1;i++)
    {
	//first and last output values are left intentionally as zero
	for (a=1;a<=numelem;a++)
	{
	    for(b=(1-a);b<a;b++)
	    {
		if( tsig == 0 )
		{					
		    m_tempoScratch[i] += ACF[a*(i+1)+b-1] * (1.0 / (2.0 * (fl_t)a-1)) * weight[i];
		}
		else
		{
		    m_tempoScratch[i] += ACF[a*(i+1)+b-1] * 1 * weight[i];
		}
	    }
	}
    }


	//////////////////////////////////////////////////
	// MODIFIED BEAT PERIOD EXTRACTION //////////////
	/////////////////////////////////////////////////

	// find smoothed version of RCF ( as applied to Detection Function)
	m_RCFConditioning->process( m_tempoScratch, m_smoothRCF);

	if (tsig != 0) // i.e. in context dependent state
	{	
//     NOW FIND MAX INDEX OF ACFOUT
            for( i = 0; i < m_lagLength; i++)
            {
                if( m_tempoScratch[ i ] > maxValRCF)
                {
                    maxValRCF = m_tempoScratch[ i ];
                    maxIndexRCF = i;
                }
            }
	}
	else // using rayleigh weighting
	{
		vector <vector<fl_t> > rcfMat;
	
		fl_t sumRcf = 0.;
	
		fl_t maxVal = 0.;
		// now find the two values which minimise rcfMat
		fl_t minVal = 0.;
		int p_i = 1; // periodicity for row i;
		int p_j = 1; //periodicity for column j;
	
	
		for ( i=0; i<m_lagLength; i++)
		{
			m_tempoScratch[i] =m_smoothRCF[i];
		}	

		// normalise m_tempoScratch so that it sums to zero.
		for ( i=0; i<m_lagLength; i++)
		{
			sumRcf += m_tempoScratch[i];
		}	
	
		for( i=0; i<m_lagLength; i++)
		{
			m_tempoScratch[i] /= sumRcf;
		}	
	
		// create a matrix to store m_tempoScratchValues modified by log2 ratio
		for ( i=0; i<m_lagLength; i++)
		{
			rcfMat.push_back  ( vector<fl_t>() ); // adds a new row...
		}
	
		for (i=0; i<m_lagLength; i++)
		{
			for (j=0; j<m_lagLength; j++)
			{
				rcfMat[i].push_back (0.);
			}
		}
	
		// the 'i' and 'j' indices deliberately start from '1' and not '0'
		for ( i=1; i<m_lagLength; i++)
		{
			for (j=1; j<m_lagLength; j++)
			{
				fl_t log2PeriodRatio = log( static_cast<fl_t>(i)/static_cast<fl_t>(j) ) / log(2.0);
				rcfMat[i][j] = ( abs(1.0-abs(log2PeriodRatio)) );
				rcfMat[i][j] += ( 0.01*( 1./(m_tempoScratch[i]+m_tempoScratch[j]) ) );
			}
		}
		
		// set diagonal equal to maximum value in rcfMat 
		// we don't want to pick one strong middle peak - we need a combination of two peaks.
	
		for ( i=1; i<m_lagLength; i++)
		{
			for (j=1; j<m_lagLength; j++)
			{
				if (rcfMat[i][j] > maxVal)
				{	
					maxVal = rcfMat[i][j];
				}
			}
		}
	
		for ( i=1; i<m_lagLength; i++)
		{
			rcfMat[i][i] = maxVal;
		}
	
		// now find the row and column number which minimise rcfMat
		minVal = maxVal;
		
		for ( i=1; i<m_lagLength; i++)
		{
			for ( j=1; j<m_lagLength; j++)
			{
				if (rcfMat[i][j] < minVal)
				{	
					minVal = rcfMat[i][j];
					p_i = i;
					p_j = j;
				}
			}
		}
	
	
		// initially choose p_j (arbitrary) - saves on an else statement
		int beatPeriod = p_j;
		if (m_tempoScratch[p_i] > m_tempoScratch[p_j])
		{
			beatPeriod = p_i;
		}
		
		// now write the output
		maxIndexRCF = static_cast<int>(beatPeriod);
	}


    fl_t locked = 5168.f / maxIndexRCF;
    if (locked >= 30 && locked <= 180) {
        m_lockedTempo = locked;
    }

#ifdef DEBUG_TEMPO_TRACK
    std::cerr << "tempoMM: locked tempo = " << m_lockedTempo << std::endl;
#endif

    if( tsig == 0 )
	tsig = 4;


#ifdef DEBUG_TEMPO_TRACK
std::cerr << "tempoMM: maxIndexRCF = " << maxIndexRCF << std::endl;
#endif
	
    if( tsig == 4 )
    {
#ifdef DEBUG_TEMPO_TRACK
        std::cerr << "tsig == 4" << std::endl;
#endif

	pdPeaks = new fl_t[ 4 ];
	for( i = 0; i < 4; i++ ){ pdPeaks[ i ] = 0.0;}

	pdPeaks[ 0 ] = ( fl_t )maxIndexRCF + 1;

	maxIndexTemp = 0;
	maxValTemp = 0.0;
	count = 0;

	for( i = (2 * maxIndexRCF + 1) - 1; i < (2 * maxIndexRCF + 1) + 2; i++ )
	{
	    if( ACF[ i ] > maxValTemp )
	    {
		maxValTemp = ACF[ i ];
		maxIndexTemp = count;
	    }
	    count++;
	}
	pdPeaks[ 1 ] = (fl_t)( maxIndexTemp + 1 + ( (2 * maxIndexRCF + 1 ) - 2 ) + 1 )/2;

	maxIndexTemp = 0;
	maxValTemp = 0.0;
	count = 0;

	for( i = (3 * maxIndexRCF + 2 ) - 2; i < (3 * maxIndexRCF + 2 ) + 3; i++ )
	{
	    if( ACF[ i ] > maxValTemp )
	    {
		maxValTemp = ACF[ i ];
		maxIndexTemp = count;
	    }
	    count++;
	}
	pdPeaks[ 2 ] = (fl_t)( maxIndexTemp + 1 + ( (3 * maxIndexRCF + 2) - 4 ) + 1 )/3;

	maxIndexTemp = 0;
	maxValTemp = 0.0;
	count = 0;

	for( i = ( 4 * maxIndexRCF + 3) - 3; i < ( 4 * maxIndexRCF + 3) + 4; i++ )
	{
	    if( ACF[ i ] > maxValTemp )
	    {
		maxValTemp = ACF[ i ];
		maxIndexTemp = count;
	    }
	    count++;
	}
	pdPeaks[ 3 ] = (fl_t)( maxIndexTemp + 1 + ( (4 * maxIndexRCF + 3) - 9 ) + 1 )/4 ;


	period = MathUtilities::mean( pdPeaks, 4 );
    }
    else
    { 
#ifdef DEBUG_TEMPO_TRACK
       std::cerr << "tsig != 4" << std::endl;
#endif

	pdPeaks = new fl_t[ 3 ];
	for( i = 0; i < 3; i++ ){ pdPeaks[ i ] = 0.0;}

	pdPeaks[ 0 ] = ( fl_t )maxIndexRCF + 1;

	maxIndexTemp = 0;
	maxValTemp = 0.0;
	count = 0;

	for( i = (2 * maxIndexRCF + 1) - 1; i < (2 * maxIndexRCF + 1) + 2; i++ )
	{
	    if( ACF[ i ] > maxValTemp )
	    {
		maxValTemp = ACF[ i ];
		maxIndexTemp = count;
	    }
	    count++;
	}
	pdPeaks[ 1 ] = (fl_t)( maxIndexTemp + 1 + ( (2 * maxIndexRCF + 1 ) - 2 ) + 1 )/2;

	maxIndexTemp = 0;
	maxValTemp = 0.0;
	count = 0;

	for( i = (3 * maxIndexRCF + 2 ) - 2; i < (3 * maxIndexRCF + 2 ) + 3; i++ )
	{
	    if( ACF[ i ] > maxValTemp )
	    {
		maxValTemp = ACF[ i ];
		maxIndexTemp = count;
	    }
	    count++;
	}
	pdPeaks[ 2 ] = (fl_t)( maxIndexTemp + 1 + ( (3 * maxIndexRCF + 2) - 4 ) + 1 )/3;


	period = MathUtilities::mean( pdPeaks, 3 );
    }

    delete [] pdPeaks;

    return period;
}

void TempoTrack::stepDetect( fl_t* periodP, fl_t* periodG, int currentIdx, int* flag )
{
    fl_t stepthresh = 1 * 3.9017;

    if( *flag )
    {
	if(abs(periodG[ currentIdx ] - periodP[ currentIdx ]) > stepthresh)
	{
	    // do nuffin'
	}
    }
    else
    {
	if(fabs(periodG[ currentIdx ]-periodP[ currentIdx ]) > stepthresh)
	{
	    *flag = 3;
	}
    }
}

void TempoTrack::constDetect( fl_t* periodP, int currentIdx, int* flag )
{
    fl_t constthresh = 2 * 3.9017;

    if( fabs( 2 * periodP[ currentIdx ] - periodP[ currentIdx - 1] - periodP[ currentIdx - 2] ) < constthresh)
    {
	*flag = 1;
    }
    else
    {
	*flag = 0;
    }
}

int TempoTrack::findMeter(fl_t *ACF, unsigned int len, fl_t period)
{
    int i;
    int p = (int)MathUtilities::round( period );
    int tsig;

    fl_t Energy_3 = 0.0;
    fl_t Energy_4 = 0.0;

    fl_t temp3A = 0.0;
    fl_t temp3B = 0.0;
    fl_t temp4A = 0.0;
    fl_t temp4B = 0.0;

    fl_t* dbf = new fl_t[ len ]; int t = 0;
    for( unsigned int u = 0; u < len; u++ ){ dbf[ u ] = 0.0; }

    if( (fl_t)len < 6 * p + 2 )
    {
	for( i = ( 3 * p - 2 ); i < ( 3 * p + 2 ) + 1; i++ )
	{
	    temp3A += ACF[ i ];
	    dbf[ t++ ] = ACF[ i ];
	}
	
	for( i = ( 4 * p - 2 ); i < ( 4 * p + 2 ) + 1; i++ )
	{
	    temp4A += ACF[ i ];
	}

	Energy_3 = temp3A;
	Energy_4 = temp4A;
    }
    else
    {
	for( i = ( 3 * p - 2 ); i < ( 3 * p + 2 ) + 1; i++ )
	{
	    temp3A += ACF[ i ];
	}
	
	for( i = ( 4 * p - 2 ); i < ( 4 * p + 2 ) + 1; i++ )
	{
	    temp4A += ACF[ i ];
	}

	for( i = ( 6 * p - 2 ); i < ( 6 * p + 2 ) + 1; i++ )
	{
	    temp3B += ACF[ i ];
	}
	
	for( i = ( 2 * p - 2 ); i < ( 2 * p + 2 ) + 1; i++ )
	{
	    temp4B += ACF[ i ];
	}

	Energy_3 = temp3A + temp3B;
	Energy_4 = temp4A + temp4B;
    }

    if (Energy_3 > Energy_4)
    {
	tsig = 3;
    }
    else
    {
	tsig = 4;
    }


    return tsig;
}

void TempoTrack::createPhaseExtractor(fl_t *Filter, unsigned int winLength, fl_t period, unsigned int fsp, unsigned int lastBeat)
{	
    int p = (int)MathUtilities::round( period );
    int predictedOffset = 0;

#ifdef DEBUG_TEMPO_TRACK
    std::cerr << "TempoTrack::createPhaseExtractor: period = " << period << ", p = " << p << std::endl;
#endif

    if (p > 10000) {
        std::cerr << "TempoTrack::createPhaseExtractor: WARNING! Highly implausible period value " << p << "!" << std::endl;
        period = 5168 / 120;
    }

    fl_t* phaseScratch = new fl_t[ p*2 + 2 ];
    for (int i = 0; i < p*2 + 2; ++i) phaseScratch[i] = 0.0;

	
    if( lastBeat != 0 )
    {
	lastBeat = (int)MathUtilities::round((fl_t)lastBeat );///(fl_t)winLength);

        predictedOffset = lastBeat + p - fsp;

        if (predictedOffset < 0) 
        {
            lastBeat = 0;
        }
    }

    if( lastBeat != 0 )
    {
	int mu = p;
	fl_t sigma = (fl_t)p/8;
	fl_t PhaseMin = 0.0;
	fl_t PhaseMax = 0.0;
	unsigned int scratchLength = p*2;
	fl_t temp = 0.0;

	for(  int i = 0; i < scratchLength; i++ )
	{
	    phaseScratch[ i ] = exp( -0.5 * pow( ( i - mu ) / sigma, 2 ) ) / ( sqrt( 2*PI ) *sigma );
	}

	MathUtilities::getFrameMinMax( phaseScratch, scratchLength, &PhaseMin, &PhaseMax );
			
	for(int i = 0; i < scratchLength; i ++)
	{
	    temp = phaseScratch[ i ];
	    phaseScratch[ i ] = (temp - PhaseMin)/PhaseMax;
	}

#ifdef DEBUG_TEMPO_TRACK
        std::cerr << "predictedOffset = " << predictedOffset << std::endl;
#endif

	unsigned int index = 0;
	for (int i = p - ( predictedOffset - 1); i < p + ( p - predictedOffset) + 1; i++)
	{
#ifdef DEBUG_TEMPO_TRACK
            std::cerr << "assigning to filter index " << index << " (size = " << p*2 << ")" << " value " << phaseScratch[i] << " from scratch index " << i << std::endl;
#endif
	    Filter[ index++ ] = phaseScratch[ i ];
	}
    }
    else
    {
	for( int i = 0; i < p; i ++)
	{
	    Filter[ i ] = 1;
	}
    }
	
    delete [] phaseScratch;
}

int TempoTrack::phaseMM(fl_t *DF, fl_t *weighting, unsigned int winLength, fl_t period)
{
    int alignment = 0;
    int p = (int)MathUtilities::round( period );

    fl_t temp = 0.0;

    fl_t* y = new fl_t[ winLength ];
    fl_t* align = new fl_t[ p ];

    for( int i = 0; i < winLength; i++ )
    {	
	y[ i ] = (fl_t)( -i + winLength  )/(fl_t)winLength;
	y[ i ] = pow(y [i ],2.0); // raise to power 2.
    }

    for( int o = 0; o < p; o++ )
    { 
	temp = 0.0;
	for(int i = 1 + (o - 1); i< winLength; i += (p + 1))
	{
	    temp = temp + DF[ i ] * y[ i ]; 
	}
	align[ o ] = temp * weighting[ o ];       
    }


    fl_t valTemp = 0.0;
    for(int i = 0; i < p; i++)
    {
	if( align[ i ] > valTemp )
	{
	    valTemp = align[ i ];
	    alignment = i;
	}
    }

    delete [] y;
    delete [] align;

    return alignment;
}

int TempoTrack::beatPredict(unsigned int FSP0, fl_t alignment, fl_t period, unsigned int step )
{
    int beat = 0;

    int p = (int)MathUtilities::round( period );
    int align = (int)MathUtilities::round( alignment );
    int FSP = (int)MathUtilities::round( FSP0 );

    int FEP = FSP + ( step );

    beat = FSP + align;

    m_beats.push_back( beat );

    while( beat + p < FEP )
    {
	beat += p;
		
	m_beats.push_back( beat );
    }

    return beat;
}



vector<int> TempoTrack::process( vector <fl_t> DF,
                                 vector <fl_t> *tempoReturn )
{
    m_dataLength = DF.size();
	
    m_lockedTempo = 0.0;

    fl_t	period = 0.0;
    int stepFlag = 0;
    int constFlag = 0;
    int FSP = 0;
    int tsig = 0;
    int lastBeat = 0;

    vector <fl_t> causalDF;

    causalDF = DF;

    //Prepare Causal Extension DFData
    unsigned int DFCLength = m_dataLength + m_winLength;
	
    for( unsigned int j = 0; j < m_winLength; j++ )
    {
	causalDF.push_back( 0 );
    }
	
	
    fl_t* RW = new fl_t[ m_lagLength ];
    for( unsigned int clear = 0; clear < m_lagLength; clear++){ RW[ clear ] = 0.0;}

    fl_t* GW = new fl_t[ m_lagLength ];
    for(unsigned int clear = 0; clear < m_lagLength; clear++){ GW[ clear ] = 0.0;}

    fl_t* PW = new fl_t[ m_lagLength ];
    for(unsigned clear = 0; clear < m_lagLength; clear++){ PW[ clear ] = 0.0;}

    m_DFFramer.setSource( &causalDF[0], m_dataLength );

    unsigned int TTFrames = m_DFFramer.getMaxNoFrames();

#ifdef DEBUG_TEMPO_TRACK
    std::cerr << "TTFrames = " << TTFrames << std::endl;
#endif
	
    fl_t* periodP = new fl_t[ TTFrames ];
    for(unsigned clear = 0; clear < TTFrames; clear++){ periodP[ clear ] = 0.0;}
	
    fl_t* periodG = new fl_t[ TTFrames ];
    for(unsigned clear = 0; clear < TTFrames; clear++){ periodG[ clear ] = 0.0;}
	
    fl_t* alignment = new fl_t[ TTFrames ];
    for(unsigned clear = 0; clear < TTFrames; clear++){ alignment[ clear ] = 0.0;}

    m_beats.clear();

    createCombFilter( RW, m_lagLength, 0, 0 );

    int TTLoopIndex = 0;

    for( unsigned int i = 0; i < TTFrames; i++ )
    {
	m_DFFramer.getFrame( m_rawDFFrame );

	m_DFConditioning->process( m_rawDFFrame, m_smoothDFFrame );

	m_correlator.doAutoUnBiased( m_smoothDFFrame, m_frameACF, m_winLength );
		
	periodP[ TTLoopIndex ] = tempoMM( m_frameACF, RW, 0 );

	if( GW[ 0 ] != 0 )
	{
	    periodG[ TTLoopIndex ] = tempoMM( m_frameACF, GW, tsig );
	}
	else
	{
	    periodG[ TTLoopIndex ] = 0.0;
	}

	stepDetect( periodP, periodG, TTLoopIndex, &stepFlag );

	if( stepFlag == 1)
	{
	    constDetect( periodP, TTLoopIndex, &constFlag );
	    stepFlag = 0;
	}
	else
	{
	    stepFlag -= 1;
	}

	if( stepFlag < 0 )
	{
	    stepFlag = 0;
	}

	if( constFlag != 0)
	{
	    tsig = findMeter( m_frameACF, m_winLength, periodP[ TTLoopIndex ] );
	
	    createCombFilter( GW, m_lagLength, tsig, periodP[ TTLoopIndex ] );
			
	    periodG[ TTLoopIndex ] = tempoMM( m_frameACF, GW, tsig ); 

	    period = periodG[ TTLoopIndex ];

#ifdef DEBUG_TEMPO_TRACK
            std::cerr << "TempoTrack::process: constFlag == " << constFlag << ", TTLoopIndex = " << TTLoopIndex << ", period from periodG = " << period << std::endl;
#endif

	    createPhaseExtractor( PW, m_winLength, period, FSP, 0 ); 

	    constFlag = 0;

	}
	else
	{
	    if( GW[ 0 ] != 0 )
	    {
		period = periodG[ TTLoopIndex ];

#ifdef DEBUG_TEMPO_TRACK
                std::cerr << "TempoTrack::process: GW[0] == " << GW[0] << ", TTLoopIndex = " << TTLoopIndex << ", period from periodG = " << period << std::endl;
#endif

                if (period > 10000) {
                    std::cerr << "TempoTrack::process: WARNING!  Highly implausible period value " << period << "!" << std::endl;
                    std::cerr << "periodG contains (of " << TTFrames << " frames): " << std::endl;
                    for (int i = 0; i < TTLoopIndex + 3 && i < TTFrames; ++i) {
                        std::cerr << i << " -> " << periodG[i] << std::endl;
                    }
                    std::cerr << "periodP contains (of " << TTFrames << " frames): " << std::endl;
                    for (int i = 0; i < TTLoopIndex + 3 && i < TTFrames; ++i) {
                        std::cerr << i << " -> " << periodP[i] << std::endl;
                    }
                    period = 5168 / 120;
                }

		createPhaseExtractor( PW, m_winLength, period, FSP, lastBeat ); 

	    }
	    else
	    {
		period = periodP[ TTLoopIndex ];

#ifdef DEBUG_TEMPO_TRACK
                std::cerr << "TempoTrack::process: GW[0] == " << GW[0] << ", TTLoopIndex = " << TTLoopIndex << ", period from periodP = " << period << std::endl;
#endif

		createPhaseExtractor( PW, m_winLength, period, FSP, 0 ); 
	    }
	}

	alignment[ TTLoopIndex ] = phaseMM( m_rawDFFrame, PW, m_winLength, period ); 

	lastBeat = beatPredict(FSP, alignment[ TTLoopIndex ], period, m_lagLength );

	FSP += (m_lagLength);

        if (tempoReturn) tempoReturn->push_back(m_lockedTempo);

	TTLoopIndex++;
    }


    delete [] periodP;
    delete [] periodG;
    delete [] alignment;

    delete [] RW;
    delete [] GW;
    delete [] PW;

    return m_beats;
}

