/***************************************************************************
                          readerextractbeat.cpp  -  description
                             -------------------
    begin                : Tue Mar 18 2003
    copyright            : (C) 2003 by Tue & Ken Haste Andersen
    email                : haste@diku.dk
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "readerextractbeat.h"

ReaderExtractBeat::ReaderExtractBeat(ReaderExtract *input, int frameSize, int frameStep, int _histSize) : ReaderExtract(input)
{
    frameNo = input->getBufferSize(); ///frameStep;
    framePerChunk = frameNo/READCHUNK_NO;
    
    // Initialize histogram
    histSize = _histSize;
    hist = new CSAMPLE[histSize];
    for (int i=0; i<histSize; i++)
        hist[i]=0.;

    // Initialize beat interval vector
    beatIntVector = new CSAMPLE[histSize];

    // Initialize beat and bpm buffer
    beatBuffer = new bool[getBufferSize()];
    bpmBuffer = new CSAMPLE[getBufferSize()];
    for (i=0; i<getBufferSize(); i++)
    {
        beatBuffer[i] = false;
        bpmBuffer[i] = -1.;
    }
    
    // These should be updated whenever a new track is loaded
    histMinInterval = 60./histMaxBPM;
    histMaxInterval = 60./histMinBPM;              
    histInterval = (histMaxInterval-histMinInterval)/(CSAMPLE)(histSize-1.);

    qDebug("min %f, max %f, interval %f",histMinInterval, histMaxInterval, histInterval);
                
    // Initialize peak chunk array
    peakIt = new Tpeaks::iterator[READCHUNK_NO];
    for (i=0; i<READCHUNK_NO; i++)
        peakIt[i] = 0;
        
    hfc = (CSAMPLE *)input->getBasePtr();
}

ReaderExtractBeat::~ReaderExtractBeat()
{
    delete [] hist;
    delete [] peakIt;
}

void ReaderExtractBeat::reset()
{
}

void *ReaderExtractBeat::getBasePtr()
{
    return (void *)beatBuffer;
}

CSAMPLE *ReaderExtractBeat::getBpmPtr()
{
    return bpmBuffer;
}

int ReaderExtractBeat::getRate()
{
    return input->getRate();
}

int ReaderExtractBeat::getChannels()
{
    return input->getChannels();
}

int ReaderExtractBeat::getBufferSize()
{
    return input->getBufferSize();
}

void *ReaderExtractBeat::processChunk(const int idx, const int start_idx, const int end_idx)
{
    // Determine peak search range
    int chunkStart, chunkEnd;
    if (idx==start_idx)
        chunkStart = idx*framePerChunk+1;
    else
        chunkStart = idx*framePerChunk;
    if (idx==end_idx)
        chunkEnd = (idx+1)*framePerChunk-1;
    else
        chunkEnd = (idx+1)*framePerChunk;
//    int chunkStart = idx*framePerChunk;
//    int chunkEnd   = (idx+1)*framePerChunk-1;

    qDebug("chunk %i-%i, max: %i",chunkStart,chunkEnd,frameNo);
    
    // Delete beat markings in beat buffer covered by chunk idx
    for (int i=chunkStart; i<chunkEnd; i++)
        beatBuffer[i] = false;

    // Delete peaks in range covered by chunk idx, from the peak list
    Tpeaks::iterator it = peakIt[idx];
//    qDebug("removing (idx %i) from %i to %i",idx, chunkStart, chunkEnd);
    while (it!=0 && (*it)>=chunkStart && (*it)<=chunkEnd)
    {
//        qDebug("removing %i",(*it));
        it = peaks.remove(it);
        if (it==peaks.end())
            it = 0;
    }
    peakIt[idx] = 0;

//    qDebug("idx: %i, start_idx: %i, end_idx: %i, frameStart: %i, frameEnd: %i",idx, start_idx, end_idx, frameStart, frameEnd);

    // If position in peak list is not given from previous step,
    // find position in peaks list to insert new elements at
    if (it==0 && peaks.count()!=0)
    {
        int i = idx+1;
        if (end_idx<=start_idx)
            while (i<READCHUNK_NO && peakIt[i]==0)
                i++;
        else
            while (i<end_idx && i<READCHUNK_NO && peakIt[i]==0)
                i++;
        if (i<READCHUNK_NO)
            it = peakIt[i];
//        qDebug("i %i",i);
    }
//    qDebug("it %p",it);

    // Add new peaks to the peak list
    bool foundPeak = false;                   
    for (i=chunkEnd-1; i>chunkStart+1; i--)
        if (hfc[i]>hfc[i-1] && hfc[i]>hfc[i+1] && hfc[i]>threshold)
        {
            foundPeak = true;


            /*
            // Perform second order interpolation
            CSAMPLE t, t1, t2, corr;
            t  = log(hfc[i]);
            t1 = log(hfc[i-1]);
            t2 = log(hfc[i-2]);

            if ((t1-2.0*t+t2) != 0)
                corr = (0.5*(t1-t2))/(t1-2*t+t2); // Correction for Gaussian window?!
            else
                corr = 0.;

            curr->amp =  afactor*exp(t-0.25*corr*(t1-t2));
            curr->freq = ((CSAMPLE) power_idx+corr)*SRATE/l;
            */
            
            // Insert peak in peaks list
            if (it!=0)
            {
//                qDebug("insert peak, %i",i);
                it = peaks.insert(it, i);
            }
            else
            {
//                qDebug("append peak %i",i);
                it = peaks.append(i);
            }
        }

    // If any peaks was found, update peakIt iterator array
    if (foundPeak)
        peakIt[idx] = it;
    
    // Print list of peaks    
    Tpeaks::iterator ii;
    i = 0;
    for (ii=peaks.begin(); ii!=peaks.end(); ++ii)
    {
//        qDebug("list element %i: %i",i,(*ii));
        i++;
    }

        
    // Perform updates to histogram if peaks was found
    if (foundPeak)
    {
        int count = 0;
        while ((*it)>=chunkStart && (*it)<=chunkEnd)
        {
            // Consider distance to previous peaks in the range between histMinBPM and histMaxBPM
            int i = (*it);
            Tpeaks::iterator it2 = it;
            if (it2==peaks.begin())
                it2 = peaks.end();
            --it2;

/*            // Reset beatIntVector
            for (int i=0; i<histSize; i++)
                beatIntVector[i] = 0.;
*/
            // Current interval in seconds
            CSAMPLE interval = (CSAMPLE)(i-(*it2))/(CSAMPLE)input->getRate();
            while(interval>0. && interval<=histMaxInterval && it2!=it)
            {
//              qDebug("interval %f",interval);
                //qDebug("val: %i, i %i, (*it) %i, it %p",i-(*it2),i,(*it2), it2);
                if (interval>=histMinInterval)
                {
                    // Histogram is updated with a gauss function centered at the found interval
                    int center  = (interval-histMinInterval)/histInterval;
                    int j_start = -min(gaussWidth, center);
                    int j_end   =  min(gaussWidth, (histSize-1)-center);
                    for (int j=j_start; j<j_end; j++)
                        hist[center+j] += exp((-0.5*j*j)/(0.5*gaussWidth))*hfc[(*it)]*hfc[(*it2)];

                    // beatIntVector is updated
                    beatIntVector[center] = hfc[(*it)]*hfc[(*it2)];
                }

                if (it2==peaks.begin())
                    it2 = peaks.end();
                --it2;

                interval = (CSAMPLE)(i-(*it2))/(CSAMPLE)input->getRate();
            }

            //
            // If a beat is found, update beatBuffer and bpmBuffer
            //
            
            // Find maximum in histogram
            int maxidx = -1;
            for (i=1; i<histSize-1; i++)
                if (hist[i]>hist[i-1] && hist[i]>hist[i+1])
                    if ((maxidx>-1 && hist[i]>hist[maxidx]) || maxidx==-1)
                        maxidx = i;

            // Update bpmBuffer
            if (maxidx>-1)
            {
                CSAMPLE bpm = 60./(((CSAMPLE)maxidx*histInterval)+histMinInterval);
                Tpeaks::iterator it3 = it;
                ++it3;
                int start;
                if (count==0)
                    start = chunkStart;
                else
                    start = (*it);
                for (int i=start; i<(*it3); i++)
                    bpmBuffer[i] = bpm;
//                qDebug("update from %i - %i",(*it),(*(it3)));
            }
                        
/*
            // Check if maximum interval is max in beatIntVector
            if (maxidx>-1)
            {
                bool beat = true;
                for (int i=0; i<histSize; i++)
                    if (beatIntVector[i]>beatIntVector[maxidx])
                    {
                        beat = false;
                        break;
                    }
                beatBuffer[(*it)] = beat;

//                if (beat)
//                    qDebug("beat at %f",(CSAMPLE)(*it)/(CSAMPLE)getRate());
            }
*/

            ++it;
            if (it == peaks.end())
                it = peaks.begin();
            if (it == peakIt[idx])
                break;
        }    
    }

    // Down-write histogram
    for (i=0; i<histSize; i++)
        hist[i] *= 0.8;
    
    // Find and print maximum
    int maxidx = -1;
    for (i=1; i<histSize-1; i++)
    {
//        std::cout << hist[i] << " ";   
        if (hist[i]>hist[i-1] && hist[i]>hist[i+1])
            if ((maxidx>-1 && hist[i]>hist[maxidx]) || maxidx==-1)
                maxidx = i;
    }
    //std::cout << "\n";        
    
    // Update remaining part of bpmBuffer
/*
    CSAMPLE bpm = 60./(((CSAMPLE)maxidx*histInterval)+histMinInterval);
    Tpeaks::iterator it3 = peaks.end(); --it3;
    for (int i=(*it3); i<chunkEnd; i++)
        bpmBuffer[i] = bpm;
*/

    // Fill beat information based on result from histogram
    if (maxidx!=-1)
    {
        // Find previous beat in beatBuffer
        int beatIdx = -1;

        int i=chunkStart;
        if (i<=chunkStart)
            i+=getBufferSize();
        while (i>chunkStart)
        {
            i--;
            if (beatBuffer[i%getBufferSize()])
            {
                beatIdx = i%getBufferSize();
                break;
            }
        }
        if (beatIdx==-1)
            beatIdx = 0;
        
        // Fill current chunk of beatBuffer
        int interval = (((CSAMPLE)maxidx*histInterval)+histMinInterval)*getRate();

        if (chunkStart<beatIdx)
            chunkStart += getBufferSize();
        while (chunkEnd<chunkStart)
            chunkEnd += getBufferSize();
        while (beatIdx < chunkStart)
            beatIdx += interval;
        while (beatIdx < chunkEnd)
        {
            beatBuffer[beatIdx%getBufferSize()]=true;
//            qDebug("interval %i, beatIdx %i, frameSize %i",interval,beatIdx,getBufferSize()/READCHUNK_NO);

            beatIdx += interval;
        }

        //qDebug("beat(89) = %i",beatBuffer[89]);
        // Print beat buffer
/*
        std::cout << "idx: " << idx << "\n";
        for (int i=chunkStart; i<chunkEnd; i++)
            if (beatBuffer[i%getBufferSize()])
                std::cout << "(" << i << "," << beatBuffer[i%getBufferSize()] << ") ";
        std::cout << "\n";
*/        
/*        
        while (beatIdx < chunkStart && beatIdx<getBufferSize())
            beatIdx += interval;
        if (beatIdx>=getBufferSize())
        {
            beatIdx -= getBufferSize();
            while (beatIdx

            beatIdx += interval;
            


        beatIdx = (beatIdx+getBufferSize())%get


        /READCHUNK_NO))-(getBufferSize()/READCHUNK_NO);

        while (beatIdx<0)
            beatIdx += interval;
        beatIdx += chunkStart;
        if (chunkEnd<chunkStart)
            chunkEnd += getBufferSize();
        while (beatIdx < chunkEnd)
        {
            beatBuffer[beatIdx%getBufferSize()] = true;
            beatIdx += interval;
        }
*/
    }



    // Print beat intervals
/*
    int b1 = -1;
    int b2 = -1;
    for (int i=chunkStart; i<chunkEnd; i++)
    {
        if (beatBuffer[i])
        {
            b2=b1;
            b1=i;

            if (b2>-1)
                qDebug("beat interval %f ms (equals %f BPM)",1000.*(CSAMPLE)(b1-b2)/(CSAMPLE)getRate(),60.*((CSAMPLE)getRate()/(CSAMPLE)(b1-b2)));
        }
    }
*/            
    
    if (maxidx>-1)
        qDebug("BPM: %f, maxidx: %i, maxval %f, srate: %i",60.*(1./(((CSAMPLE)maxidx*histInterval)+histMinInterval)),maxidx, hist[maxidx], input->getRate());
            
    return (void *)&hist[idx];
}
