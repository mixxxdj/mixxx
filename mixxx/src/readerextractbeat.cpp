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
#include "mathstuff.h"

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
    //beatIntVector = new CSAMPLE[histSize];

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
    histMaxCorr = 0.f;
    
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
}

ReaderExtractBeat::~ReaderExtractBeat()
{
    delete [] hist;
//    delete [] beatIntVector;
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
        QApplication::postEvent(m_pVisualBuffer, new ReaderEvent(0, getBufferSize()));
#endif
}

void ReaderExtractBeat::newsource(QString qFilename)
{
    textbpm.close();
    textbpm.setName(QString(qFilename).append(".bpm"));
    textbpm.open(IO_WriteOnly);

    texthist.close();
    texthist.setName(QString(qFilename).append(".hist"));
    texthist.open(IO_WriteOnly);

    textbeat.close();
    textbeat.setName(QString(qFilename).append(".beat"));
    textbeat.open(IO_WriteOnly);
    
    textconf.close();
    textconf.setName(QString(qFilename).append(".conf"));
    textconf.open(IO_WriteOnly);

    texthfc.close();
    texthfc.setName(QString(qFilename).append(".hfc"));
    texthfc.open(IO_WriteOnly);
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
    QTextStream streambpm(&textbpm);
    QTextStream streamhist(&texthist);
    QTextStream streambeat(&textbeat);
    QTextStream streamhfc(&texthfc);


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
        
//    idx = idx%READCHUNK_NO;
//    end_idx = end_idx%READCHUNK_NO;

//    qDebug("from %i-%i",frameFrom,frameTo);
                                
    // Delete beat markings in beat buffer covered by chunk idx
    int i;
    //qDebug("Deleting beat marks %i-%i (bufsize: %i)",frameFrom,frameTo+frameAdd,getBufferSize());
    for (i=frameFrom; i<=frameTo+frameAdd; i++)
        beatBuffer[i%frameNo] = 0;

    // Delete peaks in range covered by chunk idx, from the peak list
    Lpeaks::iterator it = peaks.begin();
    while (it!=peaks.end())
    {
        if (circularValidIndex((*it).i, frameFrom, frameTo, frameNo))
            it = peaks.remove(it);
        else
            ++it;
    }

    // Find start position in peak list
    Lpeaks::iterator itStart = peaks.end();
    it = itStart;
    while (it!=peaks.begin() && (*it).i>frameTo)
        --it;

    // Add new peaks to the peak list
    bool foundPeak = false;                   
    for (i=frameTo+frameAdd; i>=frameFrom; i--)
    {
        if (hfc[i%frameNo]>hfc[(i-1+frameNo)%frameNo] && hfc[i%frameNo]>hfc[(i+1)%frameNo])
        {
            Tpeak p;
            p.i = i%frameNo;
//            while (p.i>frameNo)
//                p.i -= frameNo;

            // Perform second order interpolation of current hfc peak
            CSAMPLE t=0., t1=0., t2=0.;
//            qDebug("%i %i %i %i",p.i%frameNo,(p.i+1)%frameNo,(p.i-1+frameNo)%frameNo,frameNo);
            t  = hfc[i%frameNo];
            t1 = hfc[(i-1+frameNo)%frameNo];
            t2 = hfc[(i+1)%frameNo];

            if ((t1-2.0*t+t2) != 0.)
                p.corr = (0.5*(t1-t2))/(t1-2.*t+t2);
            else
                p.corr = 0.;

            // Insert peak in peaks list
            if (it!=0)
                it = peaks.insert(it, p);
            else
                it = peaks.append(p);

            if (!foundPeak)
            {
                itStart = it;
                foundPeak = true;
            }
        }
    }

/*
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
    Lpeaks::iterator itt = peaks.begin();
    int j=0;
    while (itt!=peaks.end())
    {
        if ((*itt).i>0.)
        {                     
            px[j] = (float)(*itt).i;
            py[j] = hfc[(*itt).i];
        }
        ++itt;
        j++;
    }
    replotxy(px, py, j, gnuplot_hfc);

    // Beat marks
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
*/

    // For each sample in the current chunk...
    it = itStart;
    for (i=frameFrom; i<=frameTo+frameAdd; ++i)
    {
        // Is this sample a peak?
        if ((i%frameNo)==(*it).i)
        {
            //
            // Peak
            //

            //
            // Consider distance to previous peaks (it2) in the range between histMinBPM and histMaxBPM
            Lpeaks::iterator it2 = it;
            if (it2==peaks.begin())
                it2 = peaks.end();
            --it2;

            // Reset beatIntVector
//            for (i=0; i<histSize; i++)
//                beatIntVector[i] = 0.;

            // Interval in seconds between current peak (it) and a previous peak (it2)
            CSAMPLE interval;
            if ((*it2).i>(*it).i)
                interval = (CSAMPLE)(((*it).i+(*it).corr+frameNo)-((*it2).i+(*it2).corr))/(CSAMPLE)input->getRate();
            else
                interval = (CSAMPLE)(((*it).i+(*it).corr)-((*it2).i+(*it2).corr))/(CSAMPLE)input->getRate();
            
            // This variable is set to true, if there exists a peak which is 1.5 times larger than the current
            // peak from this peak and histMaxIdx back in time.
            bool maxPeakInHistMaxInterval = false;

            //
            // For each previous peak in histMaxInterval range, update the histogram, and the beatIntVector
            while(interval>0. && interval<=histMaxInterval && it2!=it)
            {
                // If interval is larger than histMinInterval, update the histogram
                if (interval>=histMinInterval)
                {
/*
                    // Histogram is updated with a gauss function centered at the found interval
                    int center  = (interval-histMinInterval)/histInterval;
                    int j_start = -min(gaussWidth, center);
                    int j_end   =  min(gaussWidth, (histSize-1)-center);

                    // Set hysterisisFactor. If the gauss is within histMaxIdx, use a large hysterisisFactor
                    float hysterisisFactor = 1.;
                    if (center>histMaxIdx-gaussWidth && center<histMaxIdx+gaussWidth)
                        hysterisisFactor = 1.2;

                    for (int j=j_start; j<j_end; j++)
                    {
                        hist[center+j] += exp((-0.5*j*j)/(0.5*gaussWidth))*hfc[(*it).i]*hfc[(*it2).i]*hysterisisFactor;
                        if (hist[center+j]>hist[histMaxIdx])
                            histMaxIdx = center+j;
                    }

                    // Determine histMaxCorr
                    if (histMaxIdx>1 && histMaxIdx<histSize-2)
                    {
                        float t  = hist[histMaxIdx];
                        float t1 = hist[histMaxIdx-1];
                        float t2 = hist[histMaxIdx+1];

                        if ((t1-2.0*t+t2) != 0.)
                            histMaxCorr = (0.5*(t1-t2))/(t1-2.*t+t2);
                        else
                            histMaxCorr = 0.;
                    }
                    else
                        histMaxCorr = 0.;

*/
                    // Histogram is updated with a gauss function centered at the found interval
                    CSAMPLE center  = (interval-histMinInterval)/histInterval;
                    CSAMPLE j_start = -min(gaussWidth, center);
                    CSAMPLE j_end   = min(gaussWidth, (histSize-1)-center);

                    // Set hysterisisFactor. If the gauss is within histMaxIdx, use a large hysterisisFactor
                    float hysterisisFactor = 1.;
                    if (center>(CSAMPLE)(histMaxIdx-gaussWidth) && center<(CSAMPLE)(histMaxIdx+gaussWidth))
                        hysterisisFactor = 1.2;

                    for (CSAMPLE j=j_start; j<j_end; j++)
                    {
                        int idx = round((center+j));
                        hist[idx] += exp((-0.5*j*j)/(0.5*(CSAMPLE)gaussWidth))*hfc[(*it).i]*hfc[(*it2).i]*hysterisisFactor;
                        if (hist[idx]>hist[histMaxIdx])
                        {
                             histMaxIdx = idx;

                             // Determine histMaxCorr
                             if (histMaxIdx>1 && histMaxIdx<histSize-2)
                             {
                                 float t  = hist[histMaxIdx];
                                 float t1 = hist[histMaxIdx-1];
                                 float t2 = hist[histMaxIdx+1];

                                 if ((t1-2.0*t+t2) != 0.)
                                     histMaxCorr = (0.5*(t1-t2))/(t1-2.*t+t2);
                                 else
                                     histMaxCorr = 0.;
                            }
                            else
                                histMaxCorr = 0.;
                        }
                    }


                }

                // Determine if (it2) is within histMaxIdx distance from current peak, and if it is 2.0
                // times larger than the current peak.
                if ((interval<((CSAMPLE)histMaxIdx*histInterval)+histMinInterval) && hfc[(*it2).i]>hfc[(*it).i])
                    maxPeakInHistMaxInterval = true;

                // Get next previous peak and calculate its distance (interval) to current peak
                if (it2==peaks.begin())
                    it2 = peaks.end();
                --it2;
                if ((*it2).i>(*it).i)
                    interval = (CSAMPLE)(((*it).i+frameNo)-(*it2).i)/(CSAMPLE)input->getRate();
                else
                    interval = (CSAMPLE)((*it).i-(*it2).i)/(CSAMPLE)input->getRate();
            }

            //
            // Is the current peak (it) a beat?
//            qDebug("max in interval %i",maxPeakInHistMaxInterval);
            
            // Interval in samples between current peak (it) and a previous beat mark
            int beatint;
            if (beatBufferLastIdx>(*it).i)
                beatint = ((*it).i+frameNo)-beatBufferLastIdx;
            else
                beatint = (*it).i-beatBufferLastIdx;

//            qDebug("int %i, min %i, max %i",beatint, histMaxIdx+(int)(histMinInterval/histInterval)-gaussWidth,
//                                                   histMaxIdx+(int)(histMinInterval/histInterval)+gaussWidth);

            if (
                beatint>(histMaxIdx+(int)(histMinInterval/histInterval)-gaussWidth) &&
                beatint<(histMaxIdx+(int)(histMinInterval/histInterval)+gaussWidth))
            {
                if (!maxPeakInHistMaxInterval)
                {

                    updateConfidence((*it).i, beatBufferLastIdx);
                    qDebug("set peak at %i, conf %f",(*it).i,confidence);
                    beatBuffer[(*it).i] = 1.;
                    beatBufferLastIdx = (*it).i;
                }
                //else
                //    qDebug("maxPeakInHistMaxInterval");
            }
            ++it;
        }
        else
        {
            //
            // No peak
            //

            if (histMaxIdx>-1)
            {
                // Check if it's time to set a beat, ie. the interval to last beat (beatint)
                // is greater than or equal to the current max histogram interval
                int idx = i%frameNo;
                int beatint;
                if (beatBufferLastIdx>idx)
                    beatint = (idx+frameNo)-beatBufferLastIdx;
                else
                    beatint = idx-beatBufferLastIdx;

//                qDebug("beatint %i, histint %i",beatint,histMaxIdx+(int)(histMinInterval/histInterval));                
                if (beatint >= histMaxIdx+(int)(histMinInterval/histInterval))
                {
                    // Try search for a beat a little into the future to see if it's there
                    // ****************

                    
//                    qDebug("conf %f",confidence);
                    if (confidence<=0.)
                    {
                        // Find max peak in histogram between from current index and back to max interval histogram
                        int from = (idx-(histMaxIdx+(int)(histMinInterval/histInterval))+frameNo)%frameNo;
                        if (from>idx)
                            from = max(from,frameFrom);
                        else
                            from = min(from,frameFrom);
                        int add = 0;
                        int to = idx-2;
                        if (to<0)
                            to+=frameNo;
                        if (from>to)
                            add = frameNo;
                        int maxidx = -1;
                        CSAMPLE maxval = 0.;
                        for (int i=from; i<to+add-2; ++i)
                        {
                            if (hfc[(i+1+frameNo)%frameNo]>maxval && hfc[(i+1+frameNo)%frameNo]>hfc[i%frameNo] && hfc[(i+1+frameNo)%frameNo]>hfc[(i+2+frameNo)%frameNo])
                            {
                                maxidx = (i+1+frameNo)%frameNo;
                                maxval = hfc[(i+1+frameNo)%frameNo];
                            }
                        }
//                        qDebug("maxidx: %i",maxidx);
                        
                        if (maxidx>-1)
                        {
                            // Check if a peak exists between maxidx and hist interval backwards in time
                            from = (maxidx-(histMaxIdx+(int)(histMinInterval/histInterval))+frameNo)%frameNo;
                            add = 0;
                            to = maxidx;
                            if (to<0)
                                to+=frameNo;
                            if (from>to)
                                add = frameNo;

                            bool valid = true;
                            for (int i=from; i<to+add; ++i)
                            {
                                if (hfc[(i+frameNo)%frameNo]>2.*hfc[maxidx]) // && hfc[(i+1+frameNo)%frameNo]>hfc[i%frameNo] && hfc[(i+1+frameNo)%frameNo]>hfc[(i+2+frameNo)%frameNo])
                                {
                                    valid = false;
                                }
                            }


                            if (valid)
                            {
                                beatBuffer[maxidx] = 2.;    
                                beatBufferLastIdx = maxidx;
                                updateConfidence(maxidx, beatBufferLastIdx);
                                qDebug("resync at %i, cur %i, conf %f",maxidx,idx,confidence);

/*
                                confidence = 0.;
                                QTextStream streamconf(&textconf);
                                streamconf << confidence << "\n";
                                textconf.flush();
*/
                            }
                        }
                    }
                    else
                    {

                        // If the confidence is high (>0) force a beat mark right away, otherwise, search
                        // for something which could be a valid peak
                        qDebug("force peak at %i, conf %f",idx,confidence);
                        updateConfidence(idx, beatBufferLastIdx);
                        beatBuffer[idx] = 3.;
                        beatBufferLastIdx = idx;
                    }
                    
                }
/*
                else if (beatint >= histMaxIdx+(int)(histMinInterval/histInterval)+gaussWidth)
                {
                }
*/
            }
        }
    }

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

    // Update bpmBuffer
    CSAMPLE bpm;
    if (histMaxIdx>-1)
        bpm = 60./((((CSAMPLE)histMaxIdx+histMaxCorr)*histInterval)+histMinInterval);
    else
        bpm = -1.;
    for (i=frameFrom; i<frameTo+frameAdd; ++i)
    {
        bpmBuffer[i%frameNo] = bpm;
        streambpm << bpm << "\n";
    }
    textbpm.flush();
    
    // Down-write histogram
    for (i=0; i<histSize; i++)
    {
        streamhist << hist[i] << " ";
        hist[i] *= histDownWrite;
    }
    streamhist << "\n";
    texthist.flush();

    // Write beat mark to text fil
    for (i=frameFrom; i<=frameTo+frameAdd; i++)
    {
        streambeat << beatBuffer[i%frameNo] << "\n";
        streamhfc << hfc[i%frameNo] << "\n";
    }
    texthfc.flush();
    textbeat.flush();

#ifdef __GNUPLOT__
    //
    // Plot Histogram
    //
    setLineType(gnuplot_hist,"lines");
    plotData(hist, histSize, gnuplot_hist, plotFloats);

    setLineType(gnuplot_hist,"points");
    float _maxidx = (float)histMaxIdx+histMaxCorr;
    replotxy(&_maxidx, &hist[histMaxIdx], 1, gnuplot_hist);

    //savePlot(gnuplot_hist, "hist.png", "png");
#endif

#ifdef __VISUALS__
    // Update vertex buffer by sending an event containing indexes of where to update.
    if (m_pVisualBuffer != 0)
        QApplication::postEvent(m_pVisualBuffer, new ReaderEvent(frameFrom, frameTo+frameAdd-frameFrom));
#endif

    return (void *)hist ;//&hist[idx];
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
    QTextStream streamconf(&textconf);

/*
    CSAMPLE min = 0.4f;
    if (confidence>=0.7)
        min = 0.71f;
        
    int i=curBeatIdx-1+frameNo;
    while (i<lastBeatIdx)
        i+=frameNo;
        
    CSAMPLE max = 0.00001f;
    for (; i%frameNo!=lastBeatIdx%frameNo; --i)
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
*/

    // Find max in HFC between lastBeatIdx and curBeatIdx-1
    CSAMPLE max = 0.00001f;
    int i1 = lastBeatIdx;
    int i2 = curBeatIdx-1;
    if (i1>i2)
        i2+=frameNo;        
    for (int i=i1; i<i2; ++i)
        if (hfc[i%frameNo]>max)
            max = hfc[i%frameNo];
    
    // Update confidence
    CSAMPLE tmp = hfc[curBeatIdx%frameNo]/max;
    if (tmp<=0.)
        tmp = 0.00001f;
    confidence = confidence*0.99+(0.01*(log(tmp)));


    streamconf << confidence << "\n";
    textconf.flush();


//    confidence=0.9;    
//    qDebSg("confidence :%f, beat %f, max %f, tmp %f",confidence,hfc[curBeatIdx%frameNo], max, tmp);
}



