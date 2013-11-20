/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    QM Vamp Plugin Set

    Centre for Digital Music, Queen Mary, University of London.

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#include "BarBeatTrack.h"

#include "../dsp/DetectionFunction.h"
#include "../dsp/PeakPicking.h"
#include "../dsp/TempoTrackV2.h"
#include "../dsp/DownBeat.h"
#include "../dsp/MathUtilities.h"
#include "../dsp/GetKeyMode.h"

using std::string;
using std::vector;
using std::cerr;
using std::endl;

/*
 * The Autoconf manual (section 5.5.2, "Particular Function Checks")
 *recommends the following check for source files that use the alloca()
 *function
 */
#ifdef HAVE_ALLOCA_H
    # include <alloca.h>
    #elif defined __GNUC__
    # define alloca __builtin_alloca
    #elif defined _AIX
    # define alloca __alloca
    #elif defined _MSC_VER
    # include <malloc.h>
    # define alloca _alloca
    #else
    # include <stddef.h>
    # ifdef  __cplusplus
    extern "C"
    # endif
    void *alloca (size_t);
 #endif

float BarBeatTracker::m_stepSecs = 0.01161; // 512 samples at 44100

class BarBeatTrackerData
{
public:
    BarBeatTrackerData(float rate, const DFConfig &config) : dfConfig(config) {
	df = new DetectionFunction(config);
        // decimation factor aims at resampling to c. 3KHz; must be power of 2
        int factor = MathUtilities::nextPowerOfTwo(rate / 3000);
//        std::cerr << "BarBeatTrackerData: factor = " << factor << std::endl;
        downBeat = new DownBeat(rate, factor, config.stepSize);
    }
    ~BarBeatTrackerData() {
	delete df;
        delete downBeat;
    }
    void reset() {
	delete df;
	df = new DetectionFunction(dfConfig);
	dfOutput.clear();
        downBeat->resetAudioBuffer();
        origin = Vamp::RealTime::zeroTime;
    }

    DFConfig dfConfig;
    DetectionFunction *df;
    DownBeat *downBeat;
    vector<double> dfOutput;
    Vamp::RealTime origin;
};
    

BarBeatTracker::BarBeatTracker(float inputSampleRate) :
    Vamp::Plugin(inputSampleRate),
    m_d(0),
    m_bpb(4)
{
}

BarBeatTracker::~BarBeatTracker()
{
    delete m_d;
}

string
BarBeatTracker::getIdentifier() const
{
    return "qm-barbeattracker";
}

string
BarBeatTracker::getName() const
{
    return "Bar and Beat Tracker";
}

string
BarBeatTracker::getDescription() const
{
    return "Estimate bar and beat locations";
}

string
BarBeatTracker::getMaker() const
{
    return "Queen Mary, University of London";
}

int
BarBeatTracker::getPluginVersion() const
{
    return 2;
}

string
BarBeatTracker::getCopyright() const
{
    return "Plugin by Matthew Davies, Christian Landone and Chris Cannam.  Copyright (c) 2006-2009 QMUL - All Rights Reserved";
}

BarBeatTracker::ParameterList
BarBeatTracker::getParameterDescriptors() const
{
    ParameterList list;

    ParameterDescriptor desc;

    desc.identifier = "bpb";
    desc.name = "Beats per Bar";
    desc.description = "The number of beats in each bar";
    desc.minValue = 2;
    desc.maxValue = 16;
    desc.defaultValue = 4;
    desc.isQuantized = true;
    desc.quantizeStep = 1;
    list.push_back(desc);

    return list;
}

float
BarBeatTracker::getParameter(std::string name) const
{
    if (name == "bpb") return m_bpb;
    return 0.0;
}

void
BarBeatTracker::setParameter(std::string name, float value)
{
    if (name == "bpb") m_bpb = lrintf(value);
}

bool
BarBeatTracker::initialise(size_t channels, size_t stepSize, size_t blockSize)
{
    if (m_d) {
	delete m_d;
	m_d = 0;
    }

    if (channels < getMinChannelCount() ||
	channels > getMaxChannelCount()) {
        std::cerr << "BarBeatTracker::initialise: Unsupported channel count: "
                  << channels << std::endl;
        return false;
    }

    if (stepSize != getPreferredStepSize()) {
        std::cerr << "ERROR: BarBeatTracker::initialise: Unsupported step size for this sample rate: "
                  << stepSize << " (wanted " << (getPreferredStepSize()) << ")" << std::endl;
        return false;
    }

    if (blockSize != getPreferredBlockSize()) {
        std::cerr << "WARNING: BarBeatTracker::initialise: Sub-optimal block size for this sample rate: "
                  << blockSize << " (wanted " << getPreferredBlockSize() << ")" << std::endl;
//        return false;
    }

    DFConfig dfConfig;
    dfConfig.DFType = DF_COMPLEXSD;
    dfConfig.stepSize = stepSize;
    dfConfig.frameLength = blockSize;
    dfConfig.dbRise = 3;
    dfConfig.adaptiveWhitening = false;
    dfConfig.whiteningRelaxCoeff = -1;
    dfConfig.whiteningFloor = -1;
    
    m_d = new BarBeatTrackerData(m_inputSampleRate, dfConfig);
    m_d->downBeat->setBeatsPerBar(m_bpb);
    return true;
}

void
BarBeatTracker::reset()
{
    if (m_d) m_d->reset();
}

size_t
BarBeatTracker::getPreferredStepSize() const
{
    size_t step = size_t(m_inputSampleRate * m_stepSecs + 0.0001);
    if (step < 1) step = 1;
//    std::cerr << "BarBeatTracker::getPreferredStepSize: input sample rate is " << m_inputSampleRate << ", step size is " << step << std::endl;
    return step;
}

size_t
BarBeatTracker::getPreferredBlockSize() const
{
    size_t theoretical = getPreferredStepSize() * 2;

    // I think this is not necessarily going to be a power of two, and
    // the host might have a problem with that, but I'm not sure we
    // can do much about it here
    return theoretical;
}

BarBeatTracker::OutputList
BarBeatTracker::getOutputDescriptors() const
{
    OutputList list;

    OutputDescriptor beat;
    beat.identifier = "beats";
    beat.name = "Beats";
    beat.description = "Beat locations labelled with metrical position";
    beat.unit = "";
    beat.hasFixedBinCount = true;
    beat.binCount = 0;
    beat.sampleType = OutputDescriptor::VariableSampleRate;
    beat.sampleRate = 1.0 / m_stepSecs;

    OutputDescriptor bars;
    bars.identifier = "bars";
    bars.name = "Bars";
    bars.description = "Bar locations";
    bars.unit = "";
    bars.hasFixedBinCount = true;
    bars.binCount = 0;
    bars.sampleType = OutputDescriptor::VariableSampleRate;
    bars.sampleRate = 1.0 / m_stepSecs;

    OutputDescriptor beatcounts;
    beatcounts.identifier = "beatcounts";
    beatcounts.name = "Beat Count";
    beatcounts.description = "Beat counter function";
    beatcounts.unit = "";
    beatcounts.hasFixedBinCount = true;
    beatcounts.binCount = 1;
    beatcounts.sampleType = OutputDescriptor::VariableSampleRate;
    beatcounts.sampleRate = 1.0 / m_stepSecs;

    OutputDescriptor beatsd;
    beatsd.identifier = "beatsd";
    beatsd.name = "Beat Spectral Difference";
    beatsd.description = "Beat spectral difference function used for bar-line detection";
    beatsd.unit = "";
    beatsd.hasFixedBinCount = true;
    beatsd.binCount = 1;
    beatsd.sampleType = OutputDescriptor::VariableSampleRate;
    beatsd.sampleRate = 1.0 / m_stepSecs;

    list.push_back(beat);
    list.push_back(bars);
    list.push_back(beatcounts);
    list.push_back(beatsd);

    return list;
}

BarBeatTracker::FeatureSet
BarBeatTracker::process(const float *const *inputBuffers,
                        Vamp::RealTime timestamp)
{
    if (!m_d) {
	cerr << "ERROR: BarBeatTracker::process: "
	     << "BarBeatTracker has not been initialised"
	     << endl;
	return FeatureSet();
    }

    // We use time domain input, because DownBeat requires it -- so we
    // use the time-domain version of DetectionFunction::process which
    // does its own FFT.  It requires doubles as input, so we need to
    // make a temporary copy

    // We only support a single input channel

    const int fl = m_d->dfConfig.frameLength;
#ifndef __GNUC__
    double *dfinput = (double *)alloca(fl * sizeof(double));
#else
    double dfinput[fl];
#endif
    for (int i = 0; i < fl; ++i) dfinput[i] = inputBuffers[0][i];

    double output = m_d->df->process(dfinput);

    if (m_d->dfOutput.empty()) m_d->origin = timestamp;

//    std::cerr << "df[" << m_d->dfOutput.size() << "] is " << output << std::endl;
    m_d->dfOutput.push_back(output);

    // Downsample and store the incoming audio block.
    // We have an overlap on the incoming audio stream (step size is
    // half block size) -- this function is configured to take only a
    // step size's worth, so effectively ignoring the overlap.  Note
    // however that this means we omit the last blocksize - stepsize
    // samples completely for the purposes of barline detection
    // (hopefully not a problem)
    m_d->downBeat->pushAudioBlock(inputBuffers[0]);

    return FeatureSet();
}

BarBeatTracker::FeatureSet
BarBeatTracker::getRemainingFeatures()
{
    if (!m_d) {
	cerr << "ERROR: BarBeatTracker::getRemainingFeatures: "
	     << "BarBeatTracker has not been initialised"
	     << endl;
	return FeatureSet();
    }

    return barBeatTrack();
}

BarBeatTracker::FeatureSet
BarBeatTracker::barBeatTrack()
{
    vector<double> df;
    vector<double> beatPeriod;
    vector<double> tempi;

    for (size_t i = 2; i < m_d->dfOutput.size(); ++i) { // discard first two elts
        df.push_back(m_d->dfOutput[i]);
        beatPeriod.push_back(0.0);
    }
    if (df.empty()) return FeatureSet();

    TempoTrackV2 tt(m_inputSampleRate, m_d->dfConfig.stepSize);
    tt.calculateBeatPeriod(df, beatPeriod, tempi);

    vector<double> beats;
    tt.calculateBeats(df, beatPeriod, beats);

    vector<int> downbeats;
    size_t downLength = 0;
    const float *downsampled = m_d->downBeat->getBufferedAudio(downLength);
    m_d->downBeat->findDownBeats(downsampled, downLength, beats, downbeats);

    vector<double> beatsd;
    m_d->downBeat->getBeatSD(beatsd);

//    std::cerr << "BarBeatTracker: found downbeats at: ";
//    for (int i = 0; i < downbeats.size(); ++i) std::cerr << downbeats[i] << " " << std::endl;
                                 
    FeatureSet returnFeatures;

    char label[20];

    int dbi = 0;
    int beat = 0;
    int bar = 0;

    if (!downbeats.empty()) {
        // get the right number for the first beat; this will be
        // incremented before use (at top of the following loop)
        int firstDown = downbeats[0];
        beat = m_bpb - firstDown - 1;
        if (beat == m_bpb) beat = 0;
    }

    for (size_t i = 0; i < beats.size(); ++i) {

	size_t frame = beats[i] * m_d->dfConfig.stepSize;

        if (dbi < downbeats.size() && i == downbeats[dbi]) {
            beat = 0;
            ++bar;
            ++dbi;
        } else {
            ++beat;
        }

        // outputs are:
        //
        // 0 -> beats
        // 1 -> bars
        // 2 -> beat counter function
        
	Feature feature;
	feature.hasTimestamp = true;
	feature.timestamp = m_d->origin + Vamp::RealTime::frame2RealTime
	    (frame, lrintf(m_inputSampleRate));

        sprintf(label, "%d", beat + 1);
        feature.label = label;
	returnFeatures[0].push_back(feature); // labelled beats

        feature.values.push_back(beat + 1);
        returnFeatures[2].push_back(feature); // beat function

        if (i > 0 && i <= beatsd.size()) {
            feature.values.clear();
            feature.values.push_back(beatsd[i-1]);
            feature.label = "";
            returnFeatures[3].push_back(feature); // beat spectral difference
        }

        if (beat == 0) {
            feature.values.clear();
            sprintf(label, "%d", bar);
            feature.label = label;
            returnFeatures[1].push_back(feature); // bars
        }
    }

    return returnFeatures;
}

