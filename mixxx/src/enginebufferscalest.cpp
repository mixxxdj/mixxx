/***************************************************************************
enginebufferscalest.cpp  -  description
-------------------
begin                : November 2004
copyright            : (C) 2004 by Tue & Ken Haste Andersen
email                : haste@diku.dk
adaptation           : Mads Holm
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "readerextractwave.h"
#include "mathstuff.h"
#include "engineobject.h"
#include "enginebufferscalest.h"
#include "SoundTouch.h"

using namespace soundtouch;

EngineBufferScaleST::EngineBufferScaleST(ReaderExtractWave *wave) : EngineBufferScale(wave)
{
    m_bPitchIndpTimeStretch = false;

    pSoundTouch = new soundtouch::SoundTouch(); 
    qDebug("Using SoundTouch version: %s",pSoundTouch->getVersionString());
    pSoundTouch->setChannels(2);
    pSoundTouch->setTempo(1);
    rateUsed  = 1;
    tempoUsed = 1;
    
    
    uint PlaySRate = getPlaySrate();

    pSoundTouch->setSampleRate(PlaySRate);

    pSoundTouch->setRate(rateUsed);

    oldSampleRate = PlaySRate;

    buffer_back = new CSAMPLE[ReadAheadSize*2];
    
    // Initialize variables used in fast mode
    m_bFastMode = false;
    m_iScaleFastSamplesRemainInChunk = 0;
    buffer_fast = new CSAMPLE[ReadAheadSize*2];

    pBlocks= new ScaleBufferDescriptor();
    backwards = false;
    old_backwards=false;
    _clear_pending=true;
}

EngineBufferScaleST::~EngineBufferScaleST()
{
    delete pBlocks;
    delete pSoundTouch;
    delete [] buffer_back;
}

void EngineBufferScaleST::setFastMode(bool bMode)
{
    //m_bFastMode = bMode;
}

void EngineBufferScaleST::setPitchIndpTimeStretch(bool b)
{
    m_bPitchIndpTimeStretch = b;
    if (m_bPitchIndpTimeStretch)
        pSoundTouch->setRate(1.);
    else
        pSoundTouch->setTempo(1.);
}

double EngineBufferScaleST::setRate(double _rate)
{
    /* This is a temporary wrapper to compensate for the fact that 
       the engine calls the scaler with a combination of baserate
       and tempo adjustment. The call to setRate should be replaced
       by a call to setTempo. */

    double sampleRate = wave->getRate();
    double PlaySRate=getPlaySrate(); 
    double baserate=sampleRate/PlaySRate;
    
    return setTempo(_rate/baserate)*baserate;
}

void EngineBufferScaleST::clear()
{
	_clear_pending=true;
}

void EngineBufferScaleST::reset(double _playposition,bool _backwards)
{
	pSoundTouch->clear();
	pBlocks->clear();
	old_backwards=_backwards;
	_clear_pending=false;
    readAheadPos=_playposition;
}


double EngineBufferScaleST::setTempo(double _tempo)
{
	double tempoOld = tempoUsed;
	if (_tempo>0)
	{
		tempoUsed=_tempo;
		backwards=false;
	}
	else
	{
		tempoUsed= - _tempo;
		backwards=true;
	}
	
	
    // Ensure valid range of rate
    if (tempoUsed==0.)
        return 0.;
    else
    {
        if (tempoUsed>4.)
            tempoUsed = 4.;
        else if (tempoUsed<1./2.)
            tempoUsed = 1./2.;
    }
    
	//rate= 1.0 + (rate - 1)*2;
	if (tempoOld != tempoUsed)
	{
            if (m_bPitchIndpTimeStretch)
            {    
                pSoundTouch->setTempo(tempoUsed);
            }
            else
            {
                pSoundTouch->setRate(tempoUsed);
            }
            qDebug("setTempo(%f -> %f)",_tempo,tempoUsed);
	}
    if (backwards)
        return -tempoUsed ;
    else
        return tempoUsed ;
}

CSAMPLE *EngineBufferScaleST::scale(double playpos, int buf_size, float *pBase, int iBaseLength)
{
    if (!pBase)
    {
        pBase = wavebuffer;
        iBaseLength = READBUFFERSIZE;
    }

    CSAMPLE *pFastBuffer;
    if (m_bFastMode)
        return scaleFast(playpos, buf_size, pBase, iBaseLength);
        
	
	double ScaledSamples=0;
	double UsedSamples=0;
	
	float	*data_out         = 0;
	long	input_frames      = 0;
	long    output_frames     = 0;
	long	output_frames_gen = 0;
	
	if (_clear_pending)
		reset(playpos,backwards);
	
	
	unsigned int sampleRate = wave->getRate();
	if (sampleRate != oldSampleRate)
	{
		uint PlaySRate=getPlaySrate();
		rateUsed=(double)sampleRate/(double)PlaySRate;
		pSoundTouch->setRate(rateUsed);
		oldSampleRate= sampleRate;
	}
	
	
	
    data_out = buffer;
    output_frames = buf_size/2;
	
	double factor=rateUsed*tempoUsed;
	if (factor!=0.0)
	{
		
		// Invert wavebuffer on backwards playback
		if (backwards)
		{
			while (pSoundTouch->numSamples()<(unsigned)buf_size)
			{
				for (int i=0; i<ReadAheadSize; ++i)
				{
					readAheadPos = 
						((int)readAheadPos + iBaseLength - 1) % iBaseLength;
					buffer_back[i*2+1] = pBase[(int)readAheadPos];
					
					readAheadPos = 
						((int)readAheadPos + iBaseLength - 1) % iBaseLength;
					buffer_back[i*2] = pBase[(int)readAheadPos];
				}
				// Perform conversion
				pSoundTouch->putSamples((const SAMPLETYPE *)(buffer_back),(unsigned int)ReadAheadSize);
				UsedSamples += ReadAheadSize;
				ScaledSamples += (double)input_frames / factor;
			}
		}
		else
		{
			// Perform conversion
			while (pSoundTouch->numSamples()<(unsigned)buf_size)
			{
				input_frames = 
					min( (iBaseLength - (int)readAheadPos)/2 ,ReadAheadSize);
				pSoundTouch->putSamples((const SAMPLETYPE *)(&pBase[(int)readAheadPos]),(unsigned int)input_frames);
				UsedSamples += input_frames;
				ScaledSamples += (double)input_frames / factor;
				readAheadPos=((int)readAheadPos+input_frames*2) % iBaseLength;
			}
			
		}
		
		
		if (UsedSamples>0)
		{
			pBlocks->append(ScaledSamples,UsedSamples);
		}
		
		output_frames_gen=(long int)pSoundTouch->receiveSamples((SAMPLETYPE *)buffer,(unsigned int)output_frames);
		
		UsedSamples=ScaledSamples=0;
		ScaleBufferBlock *hd=pBlocks->getHead();
		while (ScaledSamples + hd->ScaledSamples 
			<= output_frames_gen && !pBlocks->empty())
		{
			UsedSamples += hd->UsedSamples;
			ScaledSamples += hd->ScaledSamples;
			hd->UsedSamples=hd->ScaledSamples=0;
			if (!pBlocks->empty())
				pBlocks->pop();
			hd=pBlocks->getHead();
		}
		if (ScaledSamples<output_frames_gen){
			double lastFragment;
			double lastUsed;
			double lastScaled;
			
			if (hd->ScaledSamples>0)
			{
				lastFragment=
					((double)output_frames_gen-ScaledSamples)/hd->ScaledSamples;
				lastUsed=floor(lastFragment*hd->UsedSamples);
				lastScaled=lastFragment*hd->ScaledSamples;
			}
			else
			{
				lastUsed=hd->UsedSamples;
				lastScaled=hd->ScaledSamples;
			}
			lastUsed=max(lastUsed,0);
			lastScaled=max(lastScaled,0);
			hd->UsedSamples -= lastUsed;
			UsedSamples += lastUsed;
			hd->ScaledSamples -= lastScaled;
			ScaledSamples += lastScaled;
		}
		
		//pBlocks->showDebug();
	}
	else
	{
		qFatal("Factor is 0= rate * tempo = %f * %f ",rateUsed,tempoUsed);
	}
	
    // Calculate new playpos
    if (backwards)
        new_playpos = playpos - (double)UsedSamples*2.;
    else
        new_playpos = playpos + (double)UsedSamples*2.;
	
    if (m_bFastMode)
        return pFastBuffer;
    else
        return buffer;
}

ScaleBufferDescriptor::ScaleBufferDescriptor()
{
	pScaleBlocks = new ScaleBufferBlock[MAXBUFFERBLOCKS];
	Head=Tail=0;
}

ScaleBufferDescriptor::~ScaleBufferDescriptor()
{
	delete [] pScaleBlocks;
}
void ScaleBufferDescriptor::append(double ScaledSamples, double UsedSamples)
{
	pScaleBlocks[Tail].ScaledSamples=ScaledSamples;
	pScaleBlocks[Tail].UsedSamples=UsedSamples;
	Tail = (Tail + 1) % MAXBUFFERBLOCKS;
	if (Tail==Head)
		qFatal("ScaleBlockBuffer full");
}

bool ScaleBufferDescriptor::empty()
{
	return Tail==Head;
}

void ScaleBufferDescriptor::pop()
{
	if (Tail==Head)
		qFatal("ScaleBlockBuffer already empty");
	Head = (Head + 1) % MAXBUFFERBLOCKS;
}

ScaleBufferBlock *ScaleBufferDescriptor::getHead()
{
	if (Tail==Head) // Ensure zero values on queue empty
		pScaleBlocks[Head].ScaledSamples=pScaleBlocks[Head].UsedSamples=0;
	return &pScaleBlocks[Head];
}

void ScaleBufferDescriptor::showDebug()
{
	int i;
	double ScaledSamples=0;
	double UsedSamples=0;
	for (i=0;i<(MAXBUFFERBLOCKS + Tail-Head) % MAXBUFFERBLOCKS;i++)
	{
		ScaleBufferBlock b=pScaleBlocks[(Head+i)% MAXBUFFERBLOCKS];
		UsedSamples += b.UsedSamples;
		ScaledSamples += b.ScaledSamples;
	}
	qDebug("Scb Blockcount=%d usedTotal=%f scaledTotal=%f",
		(MAXBUFFERBLOCKS + Tail-Head) % MAXBUFFERBLOCKS,
		UsedSamples,ScaledSamples);
	
}

void ScaleBufferDescriptor::clear()
{
	Head=Tail=0;
}

CSAMPLE *EngineBufferScaleST::scaleFast(double playpos, int buf_size, float *pBase, int iBaseLength)
{
    
    const float kfChunkLenSec = 0.1;
    
    int iChunkSize = kfChunkLenSec*getPlaySrate();
//     int iFileBufferLen = buf_size*tempoUsed;
//     int iChunkNo = buf_size/iChunkSize;
    
    // Fast scaling (sounds like fast forward on ordinary CD players)
    int no = 0;
    int iBase = 0;
    int iOut = 0;
    float *pBaseNew = &pBase[(int)playpos];
     
    if (m_iScaleFastSamplesRemainInChunk==0)
        m_iScaleFastSamplesRemainInChunk = iChunkSize;
    
    while (iOut<buf_size)
    {
        buffer[iOut] = pBaseNew[(iBase+iBaseLength*10)%iBaseLength];
    
        iBase++;
        iOut++;
        m_iScaleFastSamplesRemainInChunk--;
        
        if (m_iScaleFastSamplesRemainInChunk<=0)
        {
            m_iScaleFastSamplesRemainInChunk = iChunkSize;
            
            if (backwards)
                iBase -= iChunkSize*(tempoUsed);
            else
                iBase += iChunkSize*(tempoUsed);
        }
    }

    // Update new play position
    new_playpos = playpos + iBase;

//      qDebug("tempo %f",tempoUsed);
    
    //qDebug("got %f, new %f, diff %f",playpos, new_playpos, new_playpos-playpos);
            
    return buffer;
}

