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

#ifdef __GNUPLOT__
    // For sleep:
    #include <unistd.h>
#endif

ReaderExtractBeat::ReaderExtractBeat(ReaderExtract *input, int frameSize, int frameStep, int _histSize) : ReaderExtract(input)
{
    frameNo = input->getBufferSize(); ///frameStep;
    framePerChunk = frameNo/READCHUNK_NO;
    
    // Initialize histogram
    histSize = _histSize;
    hist = new CSAMPLE[histSize];
    int i;
    for (i=0; i<histSize; i++)
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


#ifdef __GNUPLOT__
    // Initialize gnuplot interface
    gnuplot_hfc = openPlot("HFC");
//    gnuplot_hist = openPlot("Histogram");
    gnuplot_beat = openPlot("BeatIntVector");
//    gnuplot_bpm  = openPlot("BPM");
#endif
}

ReaderExtractBeat::~ReaderExtractBeat()
{
    delete [] hist;
    delete [] peakIt;
}

void ReaderExtractBeat::reset()
{
    peaks.clear();
    int i;
    for (i=0; i<READCHUNK_NO; i++)
        peakIt[i] = 0;    
    for (i=0; i<getBufferSize(); i++)
    {
        beatBuffer[i] = false;
        bpmBuffer[i] = -1.;
    }
    for (i=0; i<histSize; i++)
        hist[i]=0.;
    
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

void *ReaderExtractBeat::processChunk(const int idx, const int start_idx, const int end_idx, bool backwards)
{
    // Determine peak search range
    int chunkStart, chunkEnd;
    int chunkAdd = 0;
    if (backwards)
    {
        chunkStart = idx*framePerChunk+1;
        chunkEnd = (idx+1)*framePerChunk;
    }
    else
    {
        chunkStart = idx*framePerChunk-1;
        chunkEnd = (idx+1)*framePerChunk-2;
    }
    if (chunkStart<0)
        chunkStart += frameNo;
    if (chunkStart>chunkEnd)
    {
//        chunkEnd += frameNo;
        chunkAdd = frameNo;
    }

//    qDebug("chunk %i-%i, frameNo: %i, framePerChunk %i",chunkStart,chunkEnd,frameNo,framePerChunk);
    
    // Delete beat markings in beat buffer covered by chunk idx
    int i;
    for (i=chunkStart; i<=chunkEnd+chunkAdd; i++)
        beatBuffer[i%frameNo] = false;

    // Delete peaks in range covered by chunk idx, from the peak list
    Tpeaks::iterator it = peakIt[idx];
//    qDebug("removing (idx %i) from %i to %i",idx, chunkStart, chunkEnd);



    while (it!=0 && circularValidIndex((*it), chunkStart, chunkEnd, frameNo))
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
    for (i=chunkEnd+chunkAdd; i>=chunkStart; i--)
    {
        if (hfc[i%frameNo]>hfc[(i-1)%frameNo] && hfc[i%frameNo]>hfc[(i+1)%frameNo] /*&& hfc[i%frameNo]>threshold*/)
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
                it = peaks.insert(it, i%frameNo);
            else
                it = peaks.append(i%frameNo);
        }
    }

#ifdef __GNUPLOT__
    //
    // Plot HFC
    //

    // HFC
    setLineType(gnuplot_hfc,"lines");
    plotData(hfc,getBufferSize(),gnuplot_hfc,plotFloats);

    // Mark current region
    setLineType(gnuplot_hfc,"impulses");
    float y = 1000000000;
    float x = chunkStart%frameNo;
    replotxy(&x, &y, 1, gnuplot_hfc);
    x = chunkEnd%frameNo;
    replotxy(&x,   &y, 1, gnuplot_hfc);

    // Peaks
    setLineType(gnuplot_hfc,"points");
    CSAMPLE *px = new CSAMPLE[getBufferSize()];
    CSAMPLE *py = new CSAMPLE[getBufferSize()];
    Tpeaks::iterator itt = peaks.begin();
    int j=0;
    while (itt!=peaks.end())
    {
        if ((*itt)>0.)
        {                     
            px[j] = (float)(*itt);
            py[j] = hfc[(*itt)];
        }
        ++itt;
        j++;
    }
    replotxy(px, py, j, gnuplot_hfc);

    // Beat marks
    for (int i=0; i<getBufferSize(); i++)
    {
        if (beatBuffer[i])
            py[i] = hfc[i];
        else
            py[i] = 0;
    }
    replotData(py, getBufferSize(), gnuplot_hfc, plotFloats);
    
    delete [] px;
    delete [] py;

//    sleep(1);
#endif



    // If any peaks was found, update peakIt iterator array
    if (foundPeak)
        peakIt[idx] = it;
    
    // Perform updates to histogram if peaks was found
    if (foundPeak)
    {
//        qDebug("(*it): %i",(*it));

        int count = 0;
        while (circularValidIndex((*it), chunkStart, chunkEnd, frameNo))
        {
//            qDebug("peak %i",(*it));
            // Consider distance to previous peaks in the range between histMinBPM and histMaxBPM
            Tpeaks::iterator it2 = it;
            if (it2==peaks.begin())
                it2 = peaks.end();
            --it2;

            // Reset beatIntVector
            for (i=0; i<histSize; i++)
                beatIntVector[i] = 0.;

            // Current interval in seconds
            CSAMPLE interval;
            if ((*it2)>(*it))
                interval = (CSAMPLE)(((*it)+frameNo)-(*it2))/(CSAMPLE)input->getRate();
            else
                interval = (CSAMPLE)((*it)-(*it2))/(CSAMPLE)input->getRate();
//            qDebug("interval %f, (range %f-%f), peak count %i",interval,histMinInterval,histMaxInterval,peaks.count());

                        
            while(interval>0. && interval<=histMaxInterval && it2!=it)
            {
                //qDebug("val: %i, i %i, (*it) %i, it %p",i-(*it2),i,(*it2), it2);
                if (interval>=histMinInterval)
                {
//                    qDebug("update hist");
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

                if ((*it2)>(*it))
                    interval = (CSAMPLE)(((*it)+frameNo)-(*it2))/(CSAMPLE)input->getRate();
                else
                    interval = (CSAMPLE)((*it)-(*it2))/(CSAMPLE)input->getRate();
//                qDebug("interval %f, (range %f-%f), peak count %i",interval,histMinInterval,histMaxInterval,peaks.count());
            }
            
            //
            // If a beat is found, update beatBuffer and bpmBuffer
            //
            
            // Find maximum in histogram
            int maxidx = -1;
            for (int i=1; i<histSize-1; i++)
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
#ifdef __GNUPLOT__
            //
            // Plot beatIntVector
            //
            setLineType(gnuplot_beat,"lines");
            plotData(beatIntVector,histSize,gnuplot_beat,plotFloats);

            int i1=max(0,maxidx-5);
            int i2=min(maxidx+5,histSize);
            float *x = new float[i2-i1];
            float *y = new float[i2-i1];
            for (int i=0; i<i2-i1; i++)
            {
                x[i] = (float)i1+i;
                y[i] = beatIntVector[i1+i];
            }
            setLineType(gnuplot_beat,"points");
            replotxy(x, y, i2-i1, gnuplot_beat);
            setLineType(gnuplot_beat,"impulses");
            replotxy(x, y, i2-i1, gnuplot_beat);
            sleep(1);
            delete [] x;
            delete [] y;
#endif
*/
            // Check if maximum interval is max in beatIntVector
            if (maxidx>-1)
            {
                // Search for max in a small range around maxidx in beatIntVector
                int beatIntMaxIdx = -1;
                const int RANGE = 5;
                for (int i=max(0,maxidx-RANGE); i<min(histSize,maxidx+RANGE); i++)
                    if ((beatIntMaxIdx>-1 && beatIntVector[i]>beatIntVector[beatIntMaxIdx]) || (beatIntVector[i]>0.))
                        beatIntMaxIdx = i;
                
                // Set beat point if no greater value is found in beatIntVector before the index beatIntMaxIdx
                bool beat = true;
                if (beatIntMaxIdx>-1)
                {

                    for (int i=0; i<max(0,maxidx-RANGE); i++)
                    {
                        if (beatIntVector[i]>beatIntVector[beatIntMaxIdx])
                        {
                            beat = false;
                            break;
                        }
                    }
                    beatBuffer[(*it)] = beat;
//                    if (beat)
//                        qDebug("beat %i",(*it));

                }
            }

/*
#ifdef __GNUPLOT__
    setLineType(gnuplot_bpm,"lines");

    CSAMPLE *x = new CSAMPLE[getBufferSize()];
    for (int i=0; i<getBufferSize(); i++)
    {
        if (beatBuffer[i])
            x[i] = bpmBuffer[i];
        else
            x[i] = 0.;
    }

    setLineType(gnuplot_bpm,"points");
    plotData(x, getBufferSize(), gnuplot_bpm, plotFloats);
//            sleep(1);
#endif
*/


/*                qDebug("maxidx: %i", maxidx);
                      qDebug("beat at %i, beatIntMax: %f, maxidx %i",(*it),beatIntMax,maxidx);

                      for (i=0; i<histSize; i++)
                        if (beatIntVector[i]>0.)
                          std::cout << "(" << i << "," << beatIntVector[i] << ") ";
                      std::cout << "\n";
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
        hist[i] *= histDownWrite;
    
    // Find maximum
    int maxidx = -1;
    for (i=1; i<histSize-1; i++)
    {
        if (hist[i]>hist[i-1] && hist[i]>hist[i+1])
            if ((maxidx>-1 && hist[i]>hist[maxidx]) || maxidx==-1)
                maxidx = i;
    }
    //std::cout << "\n";        


/*
#ifdef __GNUPLOT__
    //
    // Plot Histogram
    //
    setLineType(gnuplot_hist,"lines");
    plotData(hist, histSize, gnuplot_hist, plotFloats);

    setLineType(gnuplot_hist,"points");
    float _maxidx = (float)maxidx;
    replotxy(&_maxidx, &hist[maxidx], 1, gnuplot_hist);
#endif
*/
            
    // Update remaining part of bpmBuffer
/*
    CSAMPLE bpm = 60./(((CSAMPLE)maxidx*histInterval)+histMinInterval);
    Tpeaks::iterator it3 = peaks.end(); --it3;
    for (int i=(*it3); i<chunkEnd; i++)
        bpmBuffer[i] = bpm;
*/

/*
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
        //maxidx=10; //HACK!!!
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
*/

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
    }
*/



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


bool ReaderExtractBeat::circularValidIndex(int idx, int start, int end, int len)
{
    if (start<end)
    {
        if (idx<=end && idx>=start)
            return true;
    }
    else if (start>end)
    {
        if (!(idx>end && idx<start))
            return true;
    }
    else if (start==end)
        return true;

    return false;
}





