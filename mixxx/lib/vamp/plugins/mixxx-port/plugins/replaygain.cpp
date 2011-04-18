

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "replaygain.h"

using std::string;
using std::vector;
using std::cerr;
using std::endl;

ReplayGain::ReplayGain(float inputSampleRate):
        Vamp::Plugin(inputSampleRate),
        m_iStepControl(0)
   {

    m_dSampleFreq=(int)inputSampleRate;
    if(m_dSampleFreq)
        m_iStepControl=1;
}

ReplayGain::~ReplayGain()
{
}

string
ReplayGain::getIdentifier() const
{
    return "replaygain";
}

string
ReplayGain::getName() const
{
    return "ReplayGain Vamp Plugin";
}

string
ReplayGain::getDescription() const
{
    // Return something helpful here!
    return "Analyzes input samples and give the recommended dB change";
}

string
ReplayGain::getMaker() const
{
    // Your name here
    return "Vittorio Colao (l0rdt@gmx.it). Totally derived from David Robinson and Glen Sawyer work.";
}

int
ReplayGain::getPluginVersion() const
{
    // Increment this each time you release a version that behaves
    // differently from the previous one
    return 1;
}

string
ReplayGain::getCopyright() const
{
    // This function is not ideally named.  It does not necessarily
    // need to say who made the plugin -- getMaker does that -- but it
    // should indicate the terms under which it is distributed.  For
    // example, "Copyright (year). All Rights Reserved", or "GPL"
    return "GPL";
}

ReplayGain::InputDomain
ReplayGain::getInputDomain() const
{
    return TimeDomain;
}

size_t
ReplayGain::getPreferredBlockSize() const
{
    return 4096; // 0 means "I can handle any block size"
}

size_t 
ReplayGain::getPreferredStepSize() const
{
    return 0; // 0 means "anything sensible"; in practice this
              // means the same as the block size for TimeDomain
              // plugins, or half of it for FrequencyDomain plugins
}

size_t
ReplayGain::getMinChannelCount() const
{
    return 1;
}

size_t
ReplayGain::getMaxChannelCount() const
{
    return 2;
}

ReplayGain::ParameterList
ReplayGain::getParameterDescriptors() const
{
    ParameterList list;

    // If the plugin has no adjustable parameters, return an empty
    // list here (and there's no need to provide implementations of
    // getParameter and setParameter in that case either).

    // Note that it is your responsibility to make sure the parameters
    // start off having their default values (e.g. in the constructor
    // above).  The host needs to know the default value so it can do
    // things like provide a "reset to default" function, but it will
    // not explicitly set your parameters to their defaults for you if
    // they have not changed in the mean time.

//   ParameterDescriptor d;
//  d.identifier = "parameter";
//  d.name = "Some Parameter";
//  d.description = "";
//  d.unit = "";
//  d.minValue = -100;
//  d.maxValue = 100;
//  d.defaultValue = 0;
//  d.isQuantized = false;
//   list.push_back(d);

    return list;
}


float ReplayGain::getParameter(string identifier) const
{
    if (identifier == "parameter") {
        return 5; // return the ACTUAL current value of your parameter here!
    }
    return 0;
}

void
ReplayGain::setParameter(string identifier, float value)
{
    if (identifier == "parameter") {
        // set the actual value of your parameter
    }
}

ReplayGain::ProgramList
ReplayGain::getPrograms() const
{
    ProgramList list;

    // If you have no programs, return an empty list (or simply don't
    // implement this function or getCurrentProgram/selectProgram)

    return list;
}

string
ReplayGain::getCurrentProgram() const
{
    return ""; // no programs
}

void
ReplayGain::selectProgram(string name)
{
}

ReplayGain::OutputList
ReplayGain::getOutputDescriptors() const
{
    OutputList list;

    // See OutputDescriptor documentation for the possibilities here.
    // Every plugin must have at least one output.

    OutputDescriptor d;
    d.identifier = "replaygain";
    d.name = "ReplayGain Value";
    d.description = "Suggested ReplayGain Value";
    d.unit = "dB";
    d.hasFixedBinCount = true;
    d.binCount = 1;
    d.hasKnownExtents = false;
    d.isQuantized = false;
    d.sampleType = OutputDescriptor::OneSamplePerStep;
    d.hasDuration = false;
    list.push_back(d);

    return list;
}

bool
ReplayGain::initialise(size_t channels, size_t stepSize, size_t blockSize)
{
    if (channels < getMinChannelCount() ||
	channels > getMaxChannelCount()) return false;
    m_iBlockSize = blockSize;
    m_ichannels=channels;
    //if(m_iStepControl)
    //    return false;
    if(this->InitGainAnalysis((long) m_dSampleFreq ))
    {
        return true;
    }
    else
    {
        std::cerr<<"Error here";
        return false;
    }
}

void
ReplayGain::reset()
{
    // Clear buffers, reset stored values, etc
}

ReplayGain::FeatureSet
ReplayGain::process(const float *const *inputBuffers, Vamp::RealTime timestamp)
{
    //int numsamples = 0;
    //numsamples = (int)(sizeof(inputBuffers[0])/sizeof(inputBuffers[0][0]));
    m_iStepControl=this->AnalyzeSamples(inputBuffers[0],inputBuffers[m_ichannels-1],m_iBlockSize,m_ichannels);
    return FeatureSet();
}

ReplayGain::FeatureSet
ReplayGain::getRemainingFeatures()
{
    Float_t result = this->GetTitleGain();
    FeatureSet fs;
    Feature gain;
    gain.hasTimestamp = false;
    gain.label = "volume change";
    //gain.values.clear();
    gain.values.push_back(result);

    fs[0].push_back(gain);
    return fs;
}


//protected

/* When calling this procedure, make sure that ip[-order] and op[-order] point to real data! */

void
ReplayGain::filter ( const Float_t* input, Float_t* output, size_t nSamples, const Float_t* a, const Float_t* b, size_t order )
{
    double  y;
    size_t  i;
    size_t  k;

    for ( i = 0; i < nSamples; i++ ) {
        y = input[i] * b[0];
        for ( k = 1; k <= order; k++ )
            y += input[i-k] * b[k] - output[i-k] * a[k];
        output[i] = (Float_t)y;
    }
}

int
ReplayGain::ResetSampleFrequency ( long samplefreq ) {
    int  i;

    /* zero out initial values */
    for ( i = 0; i < MAX_ORDER; i++ )
        linprebuf[i] = lstepbuf[i] = loutbuf[i] = rinprebuf[i] = rstepbuf[i] = routbuf[i] = 0.;

    switch ( (int)(samplefreq) ) {
        case 48000: freqindex = 0; break;
        case 44100: freqindex = 1; break;
        case 32000: freqindex = 2; break;
        case 24000: freqindex = 3; break;
        case 22050: freqindex = 4; break;
        case 16000: freqindex = 5; break;
        case 12000: freqindex = 6; break;
        case 11025: freqindex = 7; break;
        case  8000: freqindex = 8; break;
        default:    return INIT_GAIN_ANALYSIS_ERROR;
    }

    sampleWindow = (int) ceil (samplefreq * RMS_WINDOW_TIME);

    lsum         = 0.;
    rsum         = 0.;
    totsamp      = 0;

    memset ( A, 0, sizeof(A) );

    return INIT_GAIN_ANALYSIS_OK;
}

int
ReplayGain::InitGainAnalysis ( long samplefreq )
{
    if (this->ResetSampleFrequency(samplefreq) != INIT_GAIN_ANALYSIS_OK) {
        return INIT_GAIN_ANALYSIS_ERROR;
    }

    linpre       = linprebuf + MAX_ORDER;
    rinpre       = rinprebuf + MAX_ORDER;
    lstep        = lstepbuf  + MAX_ORDER;
    rstep        = rstepbuf  + MAX_ORDER;
    lout         = loutbuf   + MAX_ORDER;
    rout         = routbuf   + MAX_ORDER;

    //memset ( B, 0, sizeof(B) );

    return INIT_GAIN_ANALYSIS_OK;
}

/* returns GAIN_ANALYSIS_OK if successful, GAIN_ANALYSIS_ERROR if not */

int
ReplayGain::AnalyzeSamples ( const Float_t* left_samples, const Float_t* right_samples, size_t num_samples, int num_channels )
{
    const Float_t*  curleft;
    const Float_t*  curright;
    long            batchsamples;
    long            cursamples;
    long            cursamplepos;
    int             i;

    if ( num_samples == 0 )
        return GAIN_ANALYSIS_OK;

    cursamplepos = 0;
    batchsamples = num_samples;

    switch ( num_channels) {
    case  1: right_samples = left_samples;
    case  2: break;
    default: return GAIN_ANALYSIS_ERROR;
    }

    if ( num_samples < MAX_ORDER ) {
        memcpy ( linprebuf + MAX_ORDER, left_samples , num_samples * sizeof(Float_t) );
        memcpy ( rinprebuf + MAX_ORDER, right_samples, num_samples * sizeof(Float_t) );
    }
    else {
        memcpy ( linprebuf + MAX_ORDER, left_samples,  MAX_ORDER   * sizeof(Float_t) );
        memcpy ( rinprebuf + MAX_ORDER, right_samples, MAX_ORDER   * sizeof(Float_t) );
    }

    while ( batchsamples > 0 ) {
        cursamples = batchsamples > (long)(sampleWindow-totsamp)  ?  (long)(sampleWindow - totsamp)  :  batchsamples;
        if ( cursamplepos < MAX_ORDER ) {
            curleft  = linpre+cursamplepos;
            curright = rinpre+cursamplepos;
            if (cursamples > MAX_ORDER - cursamplepos )
                cursamples = MAX_ORDER - cursamplepos;
        }
        else {
            curleft  = left_samples  + cursamplepos;
            curright = right_samples + cursamplepos;
        }

        filter ( curleft , lstep + totsamp, cursamples, AYule[freqindex], BYule[freqindex], YULE_ORDER );
        filter ( curright, rstep + totsamp, cursamples, AYule[freqindex], BYule[freqindex], YULE_ORDER );

        filter ( lstep + totsamp, lout + totsamp, cursamples, AButter[freqindex], BButter[freqindex], BUTTER_ORDER );
        filter ( rstep + totsamp, rout + totsamp, cursamples, AButter[freqindex], BButter[freqindex], BUTTER_ORDER );

        for ( i = 0; i < cursamples; i++ ) {             /* Get the squared values */
            lsum += lout [totsamp+i] * lout [totsamp+i];
            rsum += rout [totsamp+i] * rout [totsamp+i];
        }

        batchsamples -= cursamples;
        cursamplepos += cursamples;
        totsamp      += cursamples;
        if ( totsamp == sampleWindow ) {  /* Get the Root Mean Square (RMS) for this set of samples */
            double  val  = STEPS_per_dB * 10. * log10 ( (lsum+rsum) / totsamp * 0.5 + 1.e-37 );
            int     ival = (int) val;
            if ( ival <                     0 ) ival = 0;
            if ( ival >= (int)(sizeof(A)/sizeof(*A)) ) ival = (int)(sizeof(A)/sizeof(*A)) - 1;
            A [ival]++;
            lsum = rsum = 0.;
            memmove ( loutbuf , loutbuf  + totsamp, MAX_ORDER * sizeof(Float_t) );
            memmove ( routbuf , routbuf  + totsamp, MAX_ORDER * sizeof(Float_t) );
            memmove ( lstepbuf, lstepbuf + totsamp, MAX_ORDER * sizeof(Float_t) );
            memmove ( rstepbuf, rstepbuf + totsamp, MAX_ORDER * sizeof(Float_t) );
            totsamp = 0;
        }
        if ( totsamp > sampleWindow )   /* somehow I really screwed up: Error in programming! Contact author about totsamp > sampleWindow */
            return GAIN_ANALYSIS_ERROR;
    }
    if ( num_samples < MAX_ORDER ) {
        memmove ( linprebuf,                           linprebuf + num_samples, (MAX_ORDER-num_samples) * sizeof(Float_t) );
        memmove ( rinprebuf,                           rinprebuf + num_samples, (MAX_ORDER-num_samples) * sizeof(Float_t) );
        memcpy  ( linprebuf + MAX_ORDER - num_samples, left_samples,          num_samples             * sizeof(Float_t) );
        memcpy  ( rinprebuf + MAX_ORDER - num_samples, right_samples,         num_samples             * sizeof(Float_t) );
    }
    else {
        memcpy  ( linprebuf, left_samples  + num_samples - MAX_ORDER, MAX_ORDER * sizeof(Float_t) );
        memcpy  ( rinprebuf, right_samples + num_samples - MAX_ORDER, MAX_ORDER * sizeof(Float_t) );
    }

    return GAIN_ANALYSIS_OK;
}


Float_t
ReplayGain::analyzeResult ( Uint32_t* Array, size_t len )
{
    Uint32_t  elems;
    Int32_t   upper;
    size_t    i;

    elems = 0;
    for ( i = 0; i < len; i++ )
        elems += Array[i];
    if ( elems == 0 )
        return GAIN_NOT_ENOUGH_SAMPLES;

    upper = (Int32_t) ceil (elems * (1. - RMS_PERCENTILE));
    for ( i = len; i-- > 0; ) {
        if ( (upper -= Array[i]) <= 0 )
            break;
    }

   return (Float_t) ((Float_t)PINK_REF - (Float_t)i / (Float_t)STEPS_per_dB);
   // return (Float_t)  ((Float_t)i / (Float_t)STEPS_per_dB);
}

Float_t
ReplayGain::GetTitleGain ( void )
{
    Float_t  retval;
    unsigned int    i;

    retval = this->analyzeResult( A, sizeof(A)/sizeof(*A) );

    for ( i = 0; i < sizeof(A)/sizeof(*A); i++ ) {
        //B[i] += A[i];
        A[i]  = 0;
    }

    for ( i = 0; i < MAX_ORDER; i++ )
        linprebuf[i] = lstepbuf[i] = loutbuf[i] = rinprebuf[i] = rstepbuf[i] = routbuf[i] = 0.f;

    totsamp = 0;
    lsum    = rsum = 0.;
    return retval;
}
