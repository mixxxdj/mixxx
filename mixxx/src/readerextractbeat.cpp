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
#include "visual/visualbuffer.h"
#include "readerevent.h"

#ifdef __GNUPLOT__
    // For sleep:
    #include <unistd.h>
#endif

ReaderExtractBeat::ReaderExtractBeat(ReaderExtract *input, int frameSize, int frameStep, int _histSize) : ReaderExtract(input, "marks")
{
    frameNo = input->getBufferSize(); ///frameStep;
    framePerChunk = frameNo/READCHUNK_NO;
    framePerFrameSize = frameSize/frameStep;
    
    // Initialize histogram
    histSize = _histSize;
    hist = new CSAMPLE[histSize];
    int i;
    for (i=0; i<histSize; i++)
        hist[i]=0.;

    // Initialize beat interval vector
    beatIntVector = new CSAMPLE[histSize];

    // Initialize beat and bpm buffer
    beatBuffer = new float[getBufferSize()];
    bpmBuffer = new CSAMPLE[getBufferSize()];
    for (i=0; i<getBufferSize(); i++)
    {
        beatBuffer[i] = 0;
        bpmBuffer[i] = -1.;
    }
    beatBufferLastIdx = 0;
    
    // These should be updated whenever a new track is loaded
    histMinInterval = 60.f/histMaxBPM;
    histMaxInterval = 60.f/histMinBPM;              
    histInterval = (histMaxInterval-histMinInterval)/(CSAMPLE)(histSize-1.f);
    histMaxIdx = -1;
    
    qDebug("min %f, max %f, interval %f",histMinInterval, histMaxInterval, histInterval);
                
    // Get pointer to HFC buffer
    hfc = (CSAMPLE *)input->getBasePtr();

    // Confidence is calculated for each new beat mark
    confidence = 0.;
    
#ifdef __GNUPLOT__
    // Initialize gnuplot interface
    gnuplot_hfc = openPlot("HFC");
    gnuplot_hist = openPlot("Histogram");
    gnuplot_beat = openPlot("BeatIntVector");
//    gnuplot_bpm  = openPlot("BPM");
#endif

//    textout.setName("bpm.txt");
//    textout.open( IO_WriteOnly );
}

ReaderExtractBeat::~ReaderExtractBeat()
{
    delete [] hist;
    delete [] beatIntVector;
    delete [] beatBuffer;
    delete [] bpmBuffer;
}

void ReaderExtractBeat::reset()
{
    softreset();
    
    for (int i=0; i<histSize; i++)
        hist[i]=0.;
}

void ReaderExtractBeat::softreset()
{
    peaks.clear();
    for (int i=0; i<getBufferSize(); i++)
    {
        beatBuffer[i] = 0;
        bpmBuffer[i] = -1.;
    }
    confidence = 0.;

#ifdef __VISUALS__
    // Update vertex buffer by sending an event containing indexes of where to update.
    if (m_pVisualBuffer != 0)
        for (int i=0; i<getBufferSize(); i+=getBufferSize()/READCHUNK_NO)
            QApplication::postEvent(m_pVisualBuffer, new ReaderEvent(i, getBufferSize()/READCHUNK_NO));
#endif

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

void *ReaderExtractBeat::processChunk(const int _idx, const int start_idx, const int _end_idx, bool)
{
//    QTextStream stream( &textout );

    int end_idx = _end_idx;
    int idx = _idx;
    int frameFrom, frameTo;

    // Adjust range (circular buffer)
    if (start_idx>=_end_idx)
        end_idx += READCHUNK_NO;
    if (start_idx>_idx)
        idx += READCHUNK_NO;

    // From frame...
    if (idx>start_idx)
        frameFrom = ((((idx%READCHUNK_NO)*framePerChunk)-framePerFrameSize+1)-2+frameNo)%frameNo;
    else
        frameFrom = (((idx%READCHUNK_NO)*framePerChunk)+1)%frameNo;

    // To frame...
    if (idx<end_idx-1)
        frameTo = ((((idx+1)%READCHUNK_NO)*framePerChunk)+1)%frameNo;
    else
        frameTo = (((((idx+1)%READCHUNK_NO)*framePerChunk)-framePerFrameSize)-2+frameNo)%frameNo;

    int frameAdd = 0;
    if (frameFrom>frameTo)
        frameAdd = frameNo;
        
    idx = idx%READCHUNK_NO;
    end_idx = end_idx%READCHUNK_NO;
                
    // Delete beat markings in beat buffer covered by chunk idx
    int i;
    //qDebug("Deleting beat marks %i-%i (bufsize: %i)",frameFrom,frameTo+frameAdd,getBufferSize());
    for (i=frameFrom; i<=frameTo+frameAdd; i++)
        beatBuffer[i%frameNo] = 0;

    // Delete peaks in range covered by chunk idx, from the peak list
    Tpeaks::iterator it = peaks.begin();
    while (it!=peaks.end())
    {
        if (circularValidIndex((*it), frameFrom, frameTo, frameNo))
            it = peaks.remove(it);
        else
            ++it;
    }

    // Find start position in peak list
    Tpeaks::iterator itStart = peaks.end();
    it = itStart;
    while (it!=peaks.begin() && (*it)>frameTo)
        --it;

    // Add new peaks to the peak list
    bool foundPeak = false;                   
    for (i=frameTo+frameAdd; i>=frameFrom; i--)
    {
        if (hfc[i%frameNo]>hfc[(i-1)%frameNo] && hfc[i%frameNo]>hfc[(i+1)%frameNo] /*&& hfc[i%frameNo]>threshold*/)
        {
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

            if (!foundPeak)
            {
                itStart = it;
                foundPeak = true;
            }
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
    float x = frameFrom%frameNo;
    replotxy(&x, &y, 1, gnuplot_hfc);
    x = frameTo%frameNo;
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
	int i;
    for (i=0; i<getBufferSize(); i++)
    {
        if (beatBuffer[i]==1)
            py[i] = hfc[i];
        else
            py[i] = 0;
    }
    replotData(py, getBufferSize(), gnuplot_hfc, plotFloats);
    
//    savePlot(gnuplot_hfc, "hfc.png", "png");

    delete [] px;
    delete [] py;

//    sleep(1);
#endif

    // Will be true if beats was set in the current buffer
    bool beatset = false;
        
    // Perform updates to histogram if peaks was found
    if (foundPeak)
    {
//        qDebug("(*it): %i",(*it));

        it = itStart;
        int count = 0;
        while (circularValidIndex((*it), frameFrom, frameTo, frameNo))
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

            // Interval between current peak (it) and last marked beat
            CSAMPLE cur  = (CSAMPLE)(*it)/(CSAMPLE)getRate();
            CSAMPLE last = (CSAMPLE)beatBufferLastIdx/(CSAMPLE)getRate();
            CSAMPLE markedbeatinterval;
            if (last>cur)
                markedbeatinterval = cur+((CSAMPLE)getBufferSize()/(CSAMPLE)getRate())-last;
            else
                markedbeatinterval = cur-last;
            
            // Interval corresponding to max in histogram
            CSAMPLE bestinterval = ((CSAMPLE)histMaxIdx*histInterval)+histMinInterval;
                        
            // Interval in seconds between current beat (it) and a previous peak (it2)
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
//                    qDebug("interval %f, ok",interval);
//                    qDebug("update hist");
                    // Histogram is updated with a gauss function centered at the found interval
                    int center  = (interval-histMinInterval)/histInterval;
                    int j_start = -min(gaussWidth, center);
                    int j_end   =  min(gaussWidth, (histSize-1)-center);
                    for (int j=j_start; j<j_end; j++)
                    {
                        hist[center+j] += exp((-0.5*j*j)/(0.5*gaussWidth))*hfc[(*it)]*hfc[(*it2)];
                        if (hist[center+j]>hist[histMaxIdx])
                            histMaxIdx = center+j;
                    }
                    
                    // beatIntVector is updated
                    if (abs(interval-markedbeatinterval)<(beatPrecision*(1.-confidence)))
                        beatIntVector[center] = hfc[(*it)]*hfc[(*it2)]*2.;
                    else
                        beatIntVector[center] = hfc[(*it)]*hfc[(*it2)];
                }
                else
                {
                    // The inteval is less than histMinInterval. The histogram is not updated, but the
                    // beatIntVector shold be so. Everything below histMinInterval goes to index 0 of
                    // beatIntVector.
//                    qDebug("interval %f, too small",interval);
                    if (beatIntVector[1]<hfc[(*it)]*hfc[(*it2)])
                        beatIntVector[1] = hfc[(*it)]*hfc[(*it2)];
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
            
//            qDebug("best interval %f (in hfc plot)",(((CSAMPLE)maxidx*histInterval)+histMinInterval)*86.);
                        
            if (histMaxIdx>-1)
            {
                // Check if maximum interval is max in beatIntVector
                // Multiply everything in a small range around maxidx with a constant
                const int RANGE = 5;
                int i;
                for (i=max(0,histMaxIdx-RANGE); i<min(histSize,histMaxIdx+RANGE); i++)
                    beatIntVector[i] *= 1.5;

/*                                    
#ifdef __GNUPLOT__
            //
            // Plot beatIntVector
            //
            setLineType(gnuplot_beat,"lines");
            plotData(beatIntVector,histSize,gnuplot_beat,plotFloats);

            int i1=max(0,histMaxIdx-5);
            int i2=min(histMaxIdx+5,histSize);
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

                // Search for max in a small range around maxidx in beatIntVector
                int beatIntMaxIdx = -1;
                for (i=max(0,histMaxIdx-RANGE); i<min(histSize,histMaxIdx+RANGE); i++)
                    if ((beatIntMaxIdx>-1 && beatIntVector[i]>beatIntVector[beatIntMaxIdx]) || (beatIntVector[i]>0.))
                        beatIntMaxIdx = i;
                
                // Set beat point if no greater value is found in beatIntVector before the index beatIntMaxIdx
                CSAMPLE beat = 1;
                if (beatIntMaxIdx>-1)
                {
                    for (int i=0; i<max(0,histMaxIdx-RANGE); i++)
                    {
                        if (beatIntVector[i]>beatIntVector[beatIntMaxIdx])
                        {
                            beat = 0;
                            break;
                        }
                    }
                                                                     
                    if (beat==1)
                    {
                        // Mark beat if long enough distance to last beat
                        CSAMPLE histint = (((CSAMPLE)histMaxIdx*histInterval)+histMinInterval);
                        CSAMPLE cur  = (CSAMPLE)(*it)/(CSAMPLE)getRate();
                        CSAMPLE last = (CSAMPLE)beatBufferLastIdx/(CSAMPLE)getRate();
                        CSAMPLE dist;
                        if (last>cur)
                            dist = cur+((CSAMPLE)getBufferSize()/(CSAMPLE)getRate())-last;
                        else
                            dist = cur-last;
                                                        
//                        qDebug("dist %f, interval %f",dist,histint);
                        // Check if the distance to last marked beat is ok
                        if (dist >= histint*(1.-(beatPrecision*(1.-confidence))) && dist < histint*(1.+(beatPrecision*(1.-confidence))))
                        {                            
                            // Update confidence
                            updateConfidence((*it),beatBufferLastIdx);
                            
                            qDebug("Set beat mark at peak, dist %f",dist);
                            beatBuffer[(*it)] = beat;
                            beatBufferLastIdx = (*it);
                            beatset=true;
                        }
                    }
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
            if (it == itStart)
                break;
        }    
    }


    // If no peaks was found, check if distance to last marked beat and force a beat mark
    if (!beatset && histMaxIdx>-1)
    {
        // Find ideal distance in seconds between beat marks
        CSAMPLE histint = (((CSAMPLE)histMaxIdx*histInterval)+histMinInterval); //(1.+(beatPrecision/3.));
        CSAMPLE cur  = (CSAMPLE)(frameTo)/(CSAMPLE)getRate();
        CSAMPLE last = (CSAMPLE)beatBufferLastIdx/(CSAMPLE)getRate();
        CSAMPLE dist;
        if (last>cur)
            dist = cur+((CSAMPLE)getBufferSize()/(CSAMPLE)getRate())-last;
        else                     
            dist = cur-last;

        // If interval to last marked beat is less than the current block, then mark a beat.
        if (histint<dist)
        {
            int i = (int)((last+histint)*getRate())%frameNo;

            // Update confidence
            updateConfidence(i,beatBufferLastIdx);

            beatBuffer[i] = 1;
            beatBufferLastIdx = i;
            qDebug("Force beat mark at no peak, dist %f",histint);
        }
    }

/*
    // If no peaks was found, check distance to last marked beat and check that no big peak is in between here and
    // last marked peak, then force a beat mark
    if (!beatset && histMaxIdx>-1)
    {
        // Find ideal distance in seconds between beat marks
        CSAMPLE histint = (((CSAMPLE)histMaxIdx*histInterval)+histMinInterval); //(1.+(beatPrecision/3.));
        CSAMPLE cur  = (CSAMPLE)(frameTo)/(CSAMPLE)getRate();
        CSAMPLE last = (CSAMPLE)beatBufferLastIdx/(CSAMPLE)getRate();
        CSAMPLE dist;
        if (last>cur)
            dist = cur+((CSAMPLE)getBufferSize()/(CSAMPLE)getRate())-last;
        else
            dist = cur-last;

        // If interval to last marked beat is less than the current block, then mark a beat.
        if (histint<dist)
        {
            int i = (int)((last+histint)*getRate());
            int jend = last*getRate();
            while (jend>i)
                i+=frameNo;                
            float maxhfc = 0.;
            for (int j=i-1; j<jend; j--)
                if (maxhfc<hfc[j%frameNo])
                    maxhfc=hfc[j%frameNo];

            if (hfc[i]>=maxhfc)
            {
                beatBuffer[i] = 1;
                beatBufferLastIdx = i;
                qDebug("Force beat mark at no peak, dist %f",histint);
            }
        }
    }
*/
                                                
    // Update bpmBuffer
    CSAMPLE bpm;
    if (histMaxIdx>-1)
        bpm = 60./(((CSAMPLE)histMaxIdx*histInterval)+histMinInterval);
    else
        bpm = -1.;
    for (i=frameFrom; i<frameTo+frameAdd; ++i)
    {
        bpmBuffer[i%frameNo] = bpm;
//        stream << bpm << "\n";
    }
//    textout.flush();

    // Down-write histogram
    for (i=0; i<histSize; i++)
        hist[i] *= histDownWrite;
    
#ifdef __GNUPLOT__
    //
    // Plot Histogram
    //
    setLineType(gnuplot_hist,"lines");
    plotData(hist, histSize, gnuplot_hist, plotFloats);

    setLineType(gnuplot_hist,"points");
    float _maxidx = (float)histMaxIdx;
    replotxy(&_maxidx, &hist[histMaxIdx], 1, gnuplot_hist);

    //savePlot(gnuplot_hist, "hist.png", "png");
#endif

#ifdef __VISUALS__
    // Update vertex buffer by sending an event containing indexes of where to update.
    if (m_pVisualBuffer != 0)
        QApplication::postEvent(m_pVisualBuffer, new ReaderEvent(frameFrom, frameTo+frameAdd-frameFrom));
#endif

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

void ReaderExtractBeat::updateConfidence(int curBeatIdx, int lastBeatIdx)
{
    CSAMPLE min = 0.4f;
    if (confidence>=0.7)
        min = 0.71f;
        
    int i=curBeatIdx-1+frameNo;
    while (i<lastBeatIdx)
        i+=frameNo;
        
    CSAMPLE max = 0.00001f;
    for (i; i%frameNo!=lastBeatIdx%frameNo; --i)
        if (hfc[i%frameNo]>max && hfc[i%frameNo]>hfc[(i-1+frameNo)%frameNo] && hfc[i%frameNo]>hfc[(i+1)%frameNo])
            max = hfc[i%frameNo];

    CSAMPLE tmp = hfc[curBeatIdx%frameNo]/max;
    if (tmp<=0.)
        tmp = 0.00001f;
    confidence = confidence*0.99+(0.01*(log(tmp)+2.));

    if (confidence<min)
        confidence = min;
    if (confidence>0.9)
        confidence = 0.9f;

//    confidence=0.9;    
    //qDebug("confidence :%f, beat %f, max %f, tmp %f",confidence,hfc[curBeatIdx%frameNo], max, tmp);
}



