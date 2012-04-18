/*
 *  ReplayGainAnalysis - analyzes input samples and give the recommended dB change
 *  Copyright (C) 2001 David Robinson and Glen Sawyer
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  concept and filter values by David Robinson (David@Robinson.org)
 *    -- blame him if you think the idea is flawed
 *  coding by Glen Sawyer (glensawyer@hotmail.com) 442 N 700 E, Provo, UT 84606 USA
 *    -- blame him if you think this runs too slowly, or the coding is otherwise flawed
 *  minor cosmetic tweaks to integrate with FLAC by Josh Coalson
 *
 *  For an explanation of the concepts and the basic algorithms involved, go to:
 *    http://www.replaygain.org/
 *
 *    -- Vamp plugin by Vittorio Colao (l0rdt@gmx.it)
 */


#ifndef REPLAYGAIN_H_
#define REPLAYGAIN_H_

#include <stddef.h>
#include <vamp-sdk/Plugin.h>

using std::string;

class ReplayGain : public Vamp::Plugin {
  public:
    ReplayGain (float inputSampleRate);
    virtual ~ReplayGain();

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

    void filterYule (const float* input, float* output, size_t nSamples, const float* kernel);
    void filterButter (const float* input, float* output, size_t nSamples, const float* kernel);
    bool ResetSampleFrequency ( long samplefreq );
    float analyzeResult ( unsigned int* Array, size_t len );

    int             samplefreq;
    int             BlockSize;
    int             numchannels;
    float           linprebuf [20];
    float*          linpre;                                          // left input samples, with pre-buffer
    float           lstepbuf  [2410];
    float*          lstep;                                           // left "first step" (i.e. post first filter) samples
    float           loutbuf   [2410];
    float*          lout;                                            // left "out" (i.e. post second filter) samples
    float           rinprebuf [20];
    float*          rinpre;                                          // right input samples ...
    float           rstepbuf  [2410];
    float*          rstep;
    float           routbuf   [2410];
    float*          rout;
    long            sampleWindow;                                    // number of samples required to reach number of milliseconds required for RMS window
    long            totsamp;
    double          lsum;
    double          rsum;
    int             freqindex;
    int             first;
    unsigned int    A [12000];
    float           ABYule [21];
    float           ABButter [5];
};

#endif /* REPLAYGAIN_H_ */
