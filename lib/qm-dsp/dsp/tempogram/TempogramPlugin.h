/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
   Vamp Tempogram Plugin
   Carl Bussey, Centre for Digital Music, Queen Mary University of London
   Copyright 2014 Queen Mary University of London.
    
   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.  See the file
   COPYING included with this distribution for more information.
*/

//* Should I use initialiseForGRF()? I generally think it's nicer to initialise stuff before processing. It just means that for some reason if somebody needs to process quickly (and have preparation time before) it's a bit easier on the load.
//* I've taken this approach with NoveltyCurve, Spectrogram and FIRFilter too. Is this a good approach?
//* The names "cleanUpForGRF()" and "initialise...()" are horrible...
//* The "m_..." variable name thing (I've been quite inconsitent with that)
//* Using size_t and not unsigned int?
//* In Tempogram.h, should the protected methods be private?
//* NoveltyCurve::NoveltyCurve() calls initialise(). May be overdetermined with amount of info? i.e., constructor takes parameters fftLength, numberOfBlocks... these are dimensions of vector< vector<float> >spectrogram.
//* When to use function() const?
//* spectrogram continues for too long? see tempogram output
//* should WindowFunction::hanning be static? Justification: no initialisation needed (i.e., no need for a constructor!).


// Remember to use a different guard symbol in each header!
#ifndef _TEMPOGRAM_H_
#define _TEMPOGRAM_H_

#include <vamp-sdk/Plugin.h>
#include "FIRFilter.h"
#include "WindowFunction.h"
#include "NoveltyCurveProcessor.h"
#include "SpectrogramProcessor.h"
#include "AutocorrelationProcessor.h"
#include <vamp-sdk/FFT.h>

#include <cmath>
#include <fstream>
#include <cassert>
#include <string>
#include <sstream>
#include <stdexcept>

using std::string;
using std::vector;

typedef Spectrogram Tempogram;

class TempogramPlugin : public Vamp::Plugin
{
public:
    TempogramPlugin(float inputSampleRate);
    virtual ~TempogramPlugin();

    string getIdentifier() const;
    string getName() const;
    string getDescription() const;
    string getMaker() const;
    int getPluginVersion() const;
    string getCopyright() const;

    InputDomain getInputDomain() const;
    size_t getPreferredBlockSize() const;
    size_t getPreferredStepSize() const;
    size_t getMinChannelCount() const;
    size_t getMaxChannelCount() const;

    ParameterList getParameterDescriptors() const;
    float getParameter(string identifier) const;
    void setParameter(string identifier, float value);

    ProgramList getPrograms() const;
    string getCurrentProgram() const;
    void selectProgram(string name);

    OutputList getOutputDescriptors() const;
    
    bool initialise(size_t channels, size_t stepSize, size_t blockSize);
    void reset();

    FeatureSet process(const float *const *inputBuffers,
                       Vamp::RealTime timestamp);

    FeatureSet getRemainingFeatures();

protected:
    // plugin-specific data and methods go here
    size_t m_inputBlockSize;
    size_t m_inputStepSize;
    Spectrogram m_spectrogram; //spectrogram data
    
    //Novelty Curve specific parameters
    float m_noveltyCurveMinDB;
    float m_noveltyCurveMinV;
    float m_noveltyCurveCompressionConstant;
    
    //Tempogram specific parameters
    float m_tempogramLog2WindowLength;
    size_t m_tempogramWindowLength;
    float m_tempogramLog2FftLength;
    size_t m_tempogramFftLength;
    float m_tempogramLog2HopSize;
    size_t m_tempogramHopSize;
    
    float m_tempogramMinBPM; // tempogram output bin range min
    float m_tempogramMaxBPM; // tempogram output bin range max
    unsigned int m_tempogramMinBin;
    unsigned int m_tempogramMaxBin;
    unsigned int m_tempogramMinLag;
    unsigned int m_tempogramMaxLag;
    
    //Cyclic tempogram parameters
    float m_cyclicTempogramMinBPM;
    int m_cyclicTempogramNumberOfOctaves;
    int m_cyclicTempogramOctaveDivider;
    float m_cyclicTempogramReferenceBPM;

    string floatToString(float value) const;
    vector< vector<unsigned int> > calculateTempogramNearestNeighbourLogBins() const;
    unsigned int bpmToBin(const float &bpm) const;
    float binToBPM (const int &bin) const;
    bool handleParameterValues();
};


#endif
