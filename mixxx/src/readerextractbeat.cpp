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
#include "peaklist.h"
#include "probabilityvector.h"
#include "trackinfoobject.h"

#ifdef __GNUPLOT__
    // For sleep:
    #include <unistd.h>
#endif

ReaderExtractBeat::ReaderExtractBeat(ReaderExtract *input, int frameSize, int frameStep, int _histSize) : ReaderExtract(input, "marks")
{
    frameNo = input->getBufferSize(); ///frameStep;
    framePerChunk = frameNo/READCHUNK_NO;
    framePerFrameSize = frameSize/frameStep;
    
    // Initialize beat and bpm buffer
    beatBuffer = new float[getBufferSize()];
    beatCorr = new float[getBufferSize()];
    bpmBuffer = new CSAMPLE[getBufferSize()];
    for (int i=0; i<getBufferSize(); i++)
    {
        beatBuffer[i] = 0.;
        beatCorr[i] = 0.;
        bpmBuffer[i] = 0.;
    }
    beatBufferLastIdx = 0;


    // Initialize beat probability vector
    bpv = new ProbabilityVector(60.f/histMaxBPM, 60.f/histMinBPM, _histSize);

    // Get pointer to HFC buffer
    hfc = (CSAMPLE *)input->getBasePtr();

    // Construct hfc peak list
    peaks = new PeakList(frameNo, hfc);

    // Confidence is calculated for each new beat mark
    confidence = -1.;

#ifdef __GNUPLOT__
    // Initialize gnuplot interface
    gnuplot_hfc = openPlot("HFC");
//    gnuplot_bpm  = openPlot("BPM");
#endif

    m_pTrack = 0;
}

ReaderExtractBeat::~ReaderExtractBeat()
{
    closeSource();
    delete bpv;
    delete [] beatBuffer;
    delete [] bpmBuffer;
}

void ReaderExtractBeat::newSource(TrackInfoObject *pTrack)
{
    closeSource();

    // Reset the beat estimation
    reset();
    bpv->reset();

    m_pTrack = pTrack;

    // Initialization of the BPV with the BPM value from the TrackInfoObject
    bpv->setBpm(m_pTrack->getBpm(), m_pTrack->getBpmConfidence());
    for (int i=0; i<getBufferSize(); i++)
        bpmBuffer[i] = m_pTrack->getBpm();

#ifdef FILEOUTPUT
    QString qFilename = m_pTrack->Location();
    textbpm.close();
    textbpm.setName(QString(qFilename).append(".bpm"));
    textbpm.open(IO_WriteOnly);

    textbeat.close();
    textbeat.setName(QString(qFilename).append(".beat"));
    textbeat.open(IO_WriteOnly);

    textconf.close();
    textconf.setName(QString(qFilename).append(".conf"));
    textconf.open(IO_WriteOnly);

    texthfc.close();
    texthfc.setName(QString(qFilename).append(".hfc"));
    texthfc.open(IO_WriteOnly);

    bpv->newsource(qFilename);
#endif
}

void ReaderExtractBeat::closeSource()
{
    // Update TrackInfoObject with new BPM value
    if (m_pTrack && bpv->getBestBpmConfidence()>m_pTrack->getBpmConfidence())
    {
        m_pTrack->setBpmConfidence(bpv->getBestBpmConfidence());
        m_pTrack->setBpm(bpv->getBestBpmValue());
    }
}

void ReaderExtractBeat::reset()
{
    peaks->clear();
    for (int i=0; i<getBufferSize(); i++)
    {
        beatBuffer[i] = 0.;
        beatCorr[i] = 0.;
    }
    confidence = -1.;

    // Update vertex buffer by sending an event containing indexes of where to update.
    if (m_pVisualBuffer != 0)
        QApplication::postEvent(m_pVisualBuffer, new ReaderEvent(0, getBufferSize()));
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

void *ReaderExtractBeat::processChunk(const int _idx, const int start_idx, const int _end_idx, bool backwards)
{
#ifdef FILEOUTPUT
    QTextStream streambpm(&textbpm);
    QTextStream streambeat(&textbeat);
    QTextStream streamhfc(&texthfc);
#endif

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
    frameTo = (frameTo+1)%frameNo;

    int frameAdd = 0;
    if (frameFrom>frameTo)
        frameAdd = frameNo;

    // updateFrom is used when updating the visual subsystem. The updateFrom can be before frameFrom
    // if a beat is needed to be set back in time (before frameFrom)
    int updateFrom = frameFrom;

//    idx = idx%READCHUNK_NO;
//    end_idx = end_idx%READCHUNK_NO;


    // Delete beat markings in beat buffer covered by chunk idx
    int i;
    //qDebug("Deleting beat marks %i-%i (bufsize: %i)",frameFrom,frameTo+frameAdd,getBufferSize());
    for (i=frameFrom; i<frameTo+frameAdd; i++)
    {
        beatBuffer[i%frameNo] = 0.;
        beatCorr[i%frameNo] = 0.;
    }

    // Update peak list
    peaks->update(frameFrom, frameTo+frameAdd-frameFrom);

    //peaks->print();

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
    PeakList::iterator itt = peaks->begin();
    int j=0;
    while (itt!=peaks->end())
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
    for (i=0; i<getBufferSize(); i++)
    {
        if (beatBuffer[i]==2)
            py[i] = hfc[i];
        else
            py[i] = 0;
    }
    replotData(py, getBufferSize(), gnuplot_hfc, plotFloats);
    for (i=0; i<getBufferSize(); i++)
    {
        if (beatBuffer[i]==3)
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

    // For each sample in the current chunk...
    PeakList::iterator it = peaks->getFirstInRange(frameFrom, frameTo+frameAdd-frameFrom);
    i= frameFrom;

    while (i<frameTo+frameAdd)
    {
        // Is this sample a peak?
        if ((i%frameNo)==(*it).i)
        {
            //
            // Peak
            //

            //
            // Consider distance to previous peaks (it2) in the range between histMinBPM and histMaxBPM
            PeakList::iterator it2 = it;
            if (it2==peaks->begin())
                it2 = peaks->end();
            --it2;

            // Interval in seconds between current peak (it) and a previous peak (it2)
            float interval = peaks->getDistance(it2,it)/(float)input->getRate();

            // Interval in seconds between current peak (it) and a previous beat mark
            float beatint = peaks->getDistance(beatBufferLastIdx, it)/(float)input->getRate();

            // This variable is set to true, if there exists a peak which is 1.5 times larger than the current
            // peak from this peak and histMaxIdx back in time.
            bool maxPeakInHistMaxInterval = false;

            //
            // For each previous peak in maximum beat range, update the bpv
            PeakList::iterator it3 = it2;
            while(interval>0. && interval<=60.f/histMinBPM && it2!=it)
            {
//                qDebug("cur(%i,%p)=%f, prev(%i,%p)=%f, interval %f",(*it).i, it, hfc[(*it).i],(*it2).i, it2, hfc[(*it2).i], interval);

                // Update beat probability vector
                bpv->add(interval, hfc[(*it).i]*hfc[(*it2).i]);

                // Determine if (it2) is within maximum beat distance from current peak, and if it is
                // larger than the current peak.
                if (interval<beatint && hfc[(*it2).i]>hfc[(*it).i])
                {
                    maxPeakInHistMaxInterval = true;
//                    qDebug("max in interval");
                }

                // Get next previous peak and calculate its distance (interval) to current peak
                if (it2==peaks->begin())
                    it2 = peaks->end();
                --it2;

                // Workaround for possible bug in QT or gcc. When there seems to be only one element in the
                // list, there may be a difference between it and it2!!!
                if (it2==it3)
                    break;

                // Update interval
                interval = peaks->getDistance(it2,it)/(float)input->getRate();
            }

            //
            // Is the current peak (it) a beat?
//            qDebug("max in interval %i",maxPeakInHistMaxInterval);


//            qDebug("peak beatint %f, interval %f",beatint, bpv->getCurrMaxInterval());

//            qDebug("int %f, min %f, max %f",beatint, bpv->getCurrMaxInterval()-kfBeatRange, bpv->getCurrMaxInterval()+kfBeatRange);

            if (!backwards && beatint>bpv->getCurrMaxInterval()-kfBeatRange && beatint<bpv->getCurrMaxInterval()+kfBeatRange)
            {
                if (!maxPeakInHistMaxInterval)
                {

                    updateConfidence((*it).i, beatBufferLastIdx);

                    //qDebug("set beat at %i, conf %f, bpv %f, int %f",(*it).i,confidence, bpv->getCurrMaxInterval(), beatint*input->getRate());
                    markBeat((*it).i);
                    beatCorr[(*it).i] = (*it).corr;
                    beatBufferLastIdx = (*it).i;
                }
//                else
//                    qDebug("maxPeakInHistMaxInterval");
            }
            ++it;
            if (it==peaks->end())
                it = peaks->begin();
        }
        else if (!backwards)
        {
            //
            // No peak
            //

            if (bpv->getCurrMaxInterval()>0.)
            {
                int idx = i%frameNo;

                // interval to last marked beat in seconds
                float beatint;
                if (beatBufferLastIdx>idx)
                    beatint = (float)((idx+frameNo)-beatBufferLastIdx);
                else
                    beatint = (float)(idx-beatBufferLastIdx);
                beatint /= input->getRate();

//                qDebug("no peak beatint %f, interval %f",beatint, bpv->getCurrMaxInterval());


//                qDebug("beatint %i",beatint);
//                qDebug("idx %i, int beat: %i, hist %i", idx, beatint, histMaxIdx+(int)(histMinInterval/histInterval));
//                qDebug("beatint %i, histint %i",beatint,histMaxIdx+(int)(histMinInterval/histInterval));


                // If distance to last marked beat is larger than the current beat interval + kfBeatRange,
                // consider marking a beat even though no peak is around...
                if (beatint > bpv->getCurrMaxInterval()+kfBeatRange)
                {
                    bool beat = false;

                    if (confidence<kfBeatConfThreshold)
                    {
                        //
                        // Confidence is low, thus we try to resync to find a valid beat mark

                        // Search from current sample and hist interval back in time for the largest peak
                        int from = (idx-(int)(bpv->getCurrMaxInterval()*input->getRate())+frameNo)%frameNo;
                        PeakList::iterator itmax = peaks->getMaxInRange(from, bpv->getCurrMaxInterval()*input->getRate());

                        //qDebug("curr block %i-%i, search from %i-%i, max at %i",frameFrom,frameTo+frameAdd,from,idx, (*itmax).i);

                        // If a peak, itmax, was found, ensure that there is no larger peak between itmax and the current
                        // bpv interval back in time
                        if (itmax != peaks->end())
                        {
                            int oldfrom = from;

                            // Find maximum peak between itmax and hist interval back in time
                            int from = ((*itmax).i-1-(int)(bpv->getCurrMaxInterval()*input->getRate())+frameNo)%frameNo;
                            PeakList::iterator itmax2 = peaks->getMaxInRange(from, bpv->getCurrMaxInterval()*input->getRate());
                            //qDebug("max found %i, from %i-%i, len %i",(*itmax).i,oldfrom,from, (int)(bpv->getCurrMaxInterval()*input->getRate()));

//                            if (itmax2!=peaks->end())
//                                qDebug("max1(%i) %f, max2(%i) %f",(*itmax).i,hfc[(*itmax).i],(*itmax2).i,hfc[(*itmax2).i]);
//                            else
//                                qDebug("no max2");

                            // If it exists ensure that it's less than itmax
                            if (itmax2==peaks->end() || hfc[(*itmax2).i] < hfc[(*itmax).i])
                            {
                                confidence = 0.;
                                markBeat((*itmax).i);
                                beatBufferLastIdx = (*itmax).i;
                                beatCorr[(*itmax).i] = (*itmax).corr;

                                //qDebug("resync at %i, cur %i, conf %f",(*itmax).i,idx,confidence);

                                if ((*itmax).i<updateFrom)
                                    updateFrom = (*itmax).i;

                                // Delete everything from resync point to frameTo+frameAdd
                                for (int j=(*itmax).i+1; j<frameTo+frameAdd; ++j)
                                    beatBuffer[j%frameNo] = 0.;

#ifdef FILEOUTPUT
                                QTextStream streamconf(&textconf);
                                streamconf << confidence << "\n";
                                textconf.flush();
#endif

                                beat = true;
                            }
                        }
                    }

                    if (!beat)
                    {
                        // The confidence is either high, or the resync failed. Thus a beat mark
                        // is forced near the predicted position
                        int beatidx = (beatBufferLastIdx + (int)(bpv->getCurrMaxInterval()*input->getRate()))%frameNo;
                        for (int i=beatidx; i<beatidx+kfBeatRangeForce*input->getRate(); ++i)
                        {
                            if (hfc[i%frameNo]>hfc[(i-1+frameNo)%frameNo] && hfc[i%frameNo]>hfc[(i+1)%frameNo])
                            {
                                beatidx = i;
                                break;
                            }
                        }
                        updateConfidence(beatidx, beatBufferLastIdx);

                        float interval;
                        if (beatBufferLastIdx>beatidx)
                            interval = (float)(beatidx+frameNo-beatBufferLastIdx);
                        else
                            interval = (float)(beatidx-beatBufferLastIdx);
                        //qDebug("force peak at %i, beatint %f, conf %f, bpv %f, interval %f",beatidx,beatint,confidence,bpv->getCurrMaxInterval(), interval);

                        markBeat(beatidx);
                        beatBufferLastIdx = beatidx;

                        if (beatidx<updateFrom)
                            updateFrom = beatidx;
                    }
                }
            }
        }
        else
            // Going backwards set confidence very low
            confidence = -1.;


        ++i;
    }

    // Update bpmBuffer
    float bpm = bpv->getCurrMaxInterval();
    if (bpm>0.)
        bpm = 60./bpm;
    for (i=frameFrom; i<frameTo+frameAdd; ++i)
    {
        bpmBuffer[i%frameNo] = bpm;
    }

    // Down-write histogram
    bpv->downWrite(kfHistDownWrite);

#ifdef FILEOUTPUT
    // Write beat mark and hfc to text file for a frame back in time
    int writeFrom = (frameFrom-(framePerChunk*2)+frameNo)%frameNo;
    int writeTo = (frameTo+frameAdd-(framePerChunk*2)+frameNo)%frameNo;
    int writeAdd = 0;
    if (writeTo<writeFrom)
        writeAdd = frameNo;

    for (i=writeFrom; i<writeTo+writeAdd; i++)
    {
        streambeat << beatBuffer[i%frameNo] << " " << beatCorr[i%frameNo] << "\n";
        streamhfc << hfc[i%frameNo] << "\n";
        streambpm << bpm << "\n";
    }
    textbpm.flush();
    texthfc.flush();
    textbeat.flush();
#endif

    // Update vertex buffer by sending an event containing indexes of where to update.
    if (m_pVisualBuffer != 0)
        QApplication::postEvent(m_pVisualBuffer, new ReaderEvent(updateFrom, frameTo+frameAdd-updateFrom));

    //qDebug("from %i-%i, update from %i",frameFrom,frameTo,updateFrom);

/*
    // Print beatBuffer array
    for (i=0; i<frameNo; i++)
    {
        if (beatBuffer[i]==1.)
            std::cout << "*";
		else if (beatBuffer[i]==3.)
		    std::cout << "F";
		else if (beatBuffer[i]==2.)
		    std::cout << "R";
		    std::cout << "-";
    }
    std::cout << "\n";
*/
    return 0;
}

void ReaderExtractBeat::updateConfidence(int curBeatIdx, int lastBeatIdx)
{
    // Find max in HFC between lastBeatIdx and curBeatIdx-1
    CSAMPLE max = 0.00001f;
    int i1 = lastBeatIdx+2;
    int i2 = curBeatIdx-1;
    if (i1>i2)
        i2+=frameNo;
    for (int i=i1; i<i2; ++i)
        if (hfc[i%frameNo]>max && hfc[(i-1+frameNo)%frameNo]<hfc[i%frameNo] && hfc[(i+1)%frameNo]<hfc[i%frameNo])
            max = hfc[i%frameNo];

//    qDebug("hfc last %f, cur %f, max %f",hfc[lastBeatIdx], hfc[curBeatIdx], max);
    // Update confidence
    CSAMPLE tmp = hfc[curBeatIdx%frameNo]/max;
    if (tmp<=0.)
        tmp = 0.00001f;
//    qDebug("conf %f",log(tmp));
    confidence = confidence*kfBeatConfFilter+((1.-kfBeatConfFilter)*(log(tmp)));


#ifdef FILEOUTPUT
    QTextStream streamconf(&textconf);
    streamconf << confidence << "\n";
    textconf.flush();
#endif
}

void ReaderExtractBeat::markBeat(int i)
{
    // Color is defined from confidence (between -0.2 and 0.3)
    float v = (0.2+max(-0.2,min(confidence,0.3)))/0.5;

    if (v==0.)
        beatBuffer[i] = 0.0001f;
    else
        beatBuffer[i] = v;
}


