/*
 * replaygain.cpp
 *
 *  Created on: 22/apr/2011
 *      Author: vittorio
 */
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "replaygain.h"

ReplayGain::ReplayGain(float inputSampleRate) :
        Plugin(inputSampleRate),
        samplefreq(inputSampleRate),
        BlockSize(0),
        numchannels(1) {
}

ReplayGain::~ReplayGain() {
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

    return "Analyzes input samples and gives the recommended dB change";
}

string
ReplayGain::getMaker() const
{
    return "Vittorio Colao (l0rdt@gmx.it). Totally derived from David Robinson and Glen Sawyer work.";
}

int
ReplayGain::getPluginVersion() const
{

    return 1;
}

string
ReplayGain::getCopyright() const
{

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
    return 0;
}

size_t
ReplayGain::getPreferredStepSize() const
{
    return 0;
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

    return list;
}


float
ReplayGain::getParameter(string identifier) const
{
    return 0;
}

void
ReplayGain::setParameter(string identifier, float value)
{

}

ReplayGain::ProgramList
ReplayGain::getPrograms() const
{
    ProgramList list;

    return list;
}

string
ReplayGain::getCurrentProgram() const
{
    return "";
}

void
ReplayGain::selectProgram(string name)
{
}

ReplayGain::OutputList
ReplayGain::getOutputDescriptors() const
{
    OutputList list;


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


        bool ok = this->ResetSampleFrequency(samplefreq);
        if (!ok) {
            return false;
        }

        linpre       = linprebuf + 10;
        rinpre       = rinprebuf + 10;
        lstep        = lstepbuf  + 10;
        rstep        = rstepbuf  + 10;
        lout         = loutbuf   + 10;
        rout         = routbuf   + 10;
        BlockSize    = blockSize;
        numchannels  = channels;
        return true;
}

void
ReplayGain::reset()
{
    float ok = this->ResetSampleFrequency(samplefreq);
    if(!ok){
        //cerr <<"Error in reset";
    }
    for ( int i = 0; i < (int)(sizeof(A)/sizeof(*A)); i++ ) {
        A[i]  = 0;
    }
    totsamp = 0;
    lsum    = rsum = 0.;
}

ReplayGain::FeatureSet
ReplayGain::process(const float *const *inputBuffers, Vamp::RealTime timestamp)
{

       FeatureSet fs;
       float*  left_samples;
       float*  right_samples;
       left_samples = new float[BlockSize];
       right_samples = new float[BlockSize];
       //float left_samples[BlockSize];
       //float right_samples[BlockSize];
       const float*  curleft;
       const float*  curright;
       long            batchsamples;
       long            cursamples;
       long            cursamplepos;
       int             i;


       cursamplepos = 0;
       batchsamples = BlockSize;
       if ( BlockSize == 0 )
               return fs;
       switch ( numchannels) {
       case  1: {
           for(int i=0; i<BlockSize; i++){
               left_samples[i] = 32767*inputBuffers[0][i];
               right_samples[i] = 32767*inputBuffers[0][i];
           }
//           memcpy(left_samples, inputBuffers[0], BlockSize * sizeof(float));
//           memcpy(right_samples, inputBuffers[0], BlockSize* sizeof(float));
       };
       case  2: {
           for(int i=0; i<BlockSize; i++){
                          left_samples[i] = 32767*inputBuffers[0][i];
                          right_samples[i] = 32767*inputBuffers[1][i];
                      }
//           memcpy(left_samples, inputBuffers[0], BlockSize * sizeof(float));
//           memcpy(right_samples, inputBuffers[1], BlockSize* sizeof(float));
       }break;
       default: return fs;
       }


       if ( BlockSize < 10 ) {
           memcpy ( linprebuf + 10, left_samples , BlockSize * sizeof(float) );
           memcpy ( rinprebuf + 10, right_samples, BlockSize * sizeof(float) );
       }
       else {
           memcpy ( linprebuf + 10, left_samples,  10   * sizeof(float) );
           memcpy ( rinprebuf + 10, right_samples, 10   * sizeof(float) );
       }

       while ( batchsamples > 0 ) {
           cursamples = batchsamples > (long)(sampleWindow-totsamp)  ?  (long)(sampleWindow - totsamp)  :  batchsamples;
           if ( cursamplepos < 10 ) {
               curleft  = linpre+cursamplepos;
               curright = rinpre+cursamplepos;
               if (cursamples > 10 - cursamplepos )
                   cursamples = 10 - cursamplepos;
           }
           else {
               curleft  = left_samples  + cursamplepos;
               curright = right_samples + cursamplepos;
           }

           this->filterYule( curleft , lstep + totsamp, cursamples, ABYule);
           this->filterYule( curright, rstep + totsamp, cursamples, ABYule);

           this->filterButter( lstep + totsamp, lout + totsamp, cursamples, ABButter);
           this->filterButter( rstep + totsamp, rout + totsamp, cursamples, ABButter);

           curleft = lout + totsamp;                   // Get the squared values
           curright = rout + totsamp;

           i = cursamples % 16;
           while (i--)
           {   lsum += pow((*curleft++),2);
               rsum += pow((*curright++),2);
           }
           i = cursamples / 16;
           while (i--)
           {   lsum += pow(curleft[0],2)
                     + pow(curleft[1],2)
                     + pow(curleft[2],2)
                     + pow(curleft[3],2)
                     + pow(curleft[4],2)
                     + pow(curleft[5],2)
                     + pow(curleft[6],2)
                     + pow(curleft[7],2)
                     + pow(curleft[8],2)
                     + pow(curleft[9],2)
                     + pow(curleft[10],2)
                     + pow(curleft[11],2)
                     + pow(curleft[12],2)
                     + pow(curleft[13],2)
                     + pow(curleft[14],2)
                     + pow(curleft[15],2);
               curleft += 16;
               rsum += pow(curright[0],2)
                     + pow(curright[1],2)
                     + pow(curright[2],2)
                     + pow(curright[3],2)
                     + pow(curright[4],2)
                     + pow(curright[5],2)
                     + pow(curright[6],2)
                     + pow(curright[7],2)
                     + pow(curright[8],2)
                     + pow(curright[9],2)
                     + pow(curright[10],2)
                     + pow(curright[11],2)
                     + pow(curright[12],2)
                     + pow(curright[13],2)
                     + pow(curright[14],2)
                     + pow(curright[15],2);
               curright += 16;
           }

           batchsamples -= cursamples;
           cursamplepos += cursamples;
           totsamp      += cursamples;
           if ( totsamp == sampleWindow ) {  // Get the Root Mean Square (RMS) for this set of samples
               double  val  = 1000. * log10 ( (lsum+rsum) / totsamp * 0.5 + 1.e-37 );
               int     ival = (int) val;

               if ( ival <                     0 ) ival = 0;
               if ( ival >= (int)(sizeof(A)/sizeof(*A)) ) ival = sizeof(A)/sizeof(*A) - 1;
               A [ival]++;
               lsum = rsum = 0.;
               memmove ( loutbuf , loutbuf  + totsamp, 10 * sizeof(float) );
               memmove ( routbuf , routbuf  + totsamp, 10 * sizeof(float) );
               memmove ( lstepbuf, lstepbuf + totsamp, 10 * sizeof(float) );
               memmove ( rstepbuf, rstepbuf + totsamp, 10 * sizeof(float) );
               totsamp = 0;
           }
           if ( totsamp > sampleWindow )   // somehow I really screwed up: Error in programming! Contact author about totsamp > sampleWindow
               return fs;
       }
       if (  BlockSize < 10 ) {
           memmove ( linprebuf,                           linprebuf +  BlockSize, (10- BlockSize) * sizeof(float) );
           memmove ( rinprebuf,                           rinprebuf +  BlockSize, (10- BlockSize) * sizeof(float) );
           memcpy  ( linprebuf + 10 -  BlockSize, left_samples,           BlockSize             * sizeof(float) );
           memcpy  ( rinprebuf + 10 -  BlockSize, right_samples,          BlockSize            * sizeof(float) );
       }
       else {
           memcpy  ( linprebuf, left_samples  + BlockSize - 10, 10 * sizeof(float) );
           memcpy  ( rinprebuf, right_samples + BlockSize - 10, 10 * sizeof(float) );
       }
       delete [] left_samples;
       delete [] right_samples;
       return fs;
}

ReplayGain::FeatureSet
ReplayGain::getRemainingFeatures()
{
float  retval;
    int    i;

    retval = this->analyzeResult( A, sizeof(A)/sizeof(*A) );

    for ( i = 0; i < (int)(sizeof(A)/sizeof(*A)); i++ ) {
        A[i]  = 0;
    }

    for ( i = 0; i < 10; i++ )
        linprebuf[i] = lstepbuf[i] = loutbuf[i] = rinprebuf[i] = rstepbuf[i] = routbuf[i] = 0.f;

    totsamp = 0;
    lsum    = rsum = 0.;
    FeatureSet fs;
        Feature gain;
        gain.hasTimestamp = false;
        gain.label = "volume change";
        //gain.values.clear();
        gain.values.push_back(retval);
        fs[0].push_back(gain);
    return fs;
}

//private functions

void
ReplayGain::filterYule (const float* input, float* output, size_t nSamples, const float* kernel){
    while (nSamples--) {
          *output =  1e-10  /* 1e-10 is a hack to avoid slowdown because of denormals */
            + input [0]  * kernel[0]
            - output[-1] * kernel[1]
            + input [-1] * kernel[2]
            - output[-2] * kernel[3]
            + input [-2] * kernel[4]
            - output[-3] * kernel[5]
            + input [-3] * kernel[6]
            - output[-4] * kernel[7]
            + input [-4] * kernel[8]
            - output[-5] * kernel[9]
            + input [-5] * kernel[10]
            - output[-6] * kernel[11]
            + input [-6] * kernel[12]
            - output[-7] * kernel[13]
            + input [-7] * kernel[14]
            - output[-8] * kernel[15]
            + input [-8] * kernel[16]
            - output[-9] * kernel[17]
            + input [-9] * kernel[18]
            - output[-10]* kernel[19]
            + input [-10]* kernel[20];
           ++output;
           ++input;
       }
}

void
ReplayGain::filterButter(const float* input, float* output, size_t nSamples, const float* kernel){


    while (nSamples--) {
        *output =
           input [0]  * kernel[0]
         - output[-1] * kernel[1]
         + input [-1] * kernel[2]
         - output[-2] * kernel[3]
         + input [-2] * kernel[4];
        ++output;
        ++input;
    }
}

bool
ReplayGain::ResetSampleFrequency(long samplefreq){
    int  i;

        // zero out initial values
        for ( i = 0; i < 10; i++ )
            linprebuf[i] = lstepbuf[i] = loutbuf[i] = rinprebuf[i] = rstepbuf[i] = routbuf[i] = 0.;

        switch ((int) (samplefreq)) {

    case 48000: {
        ABYule[0] = 0.03857599435200;
        ABYule[1] = -3.84664617118067;
        ABYule[2] = -0.02160367184185;
        ABYule[3] = 7.81501653005538;
        ABYule[4] = -0.00123395316851;
        ABYule[5] = -11.34170355132042;
        ABYule[6] = -0.00009291677959;
        ABYule[7] = 13.05504219327545;
        ABYule[8] = -0.01655260341619;
        ABYule[9] = -12.28759895145294;
        ABYule[10] = 0.02161526843274;
        ABYule[11] = 9.48293806319790;
        ABYule[12] = -0.02074045215285;
        ABYule[13] = -5.87257861775999;
        ABYule[14] = 0.00594298065125;
        ABYule[15] = 2.75465861874613;
        ABYule[16] = 0.00306428023191;
        ABYule[17] = -0.86984376593551;
        ABYule[18] = 0.00012025322027;
        ABYule[19] = 0.13919314567432;
        ABYule[20] = 0.00288463683916;

        ABButter[0] = 0.98621192462708;
        ABButter[1] = -1.97223372919527;
        ABButter[2] = -1.97242384925416;
        ABButter[3] = 0.97261396931306;
        ABButter[4] = 0.98621192462708;

    }
        ;
        break;
    case 44100: {

        ABYule[0] = 0.05418656406430;
        ABYule[1] = -3.47845948550071;
        ABYule[2] = -0.02911007808948;
        ABYule[3] = 6.36317777566148;
        ABYule[4] = -0.00848709379851;
        ABYule[5] = -8.54751527471874;
        ABYule[6] = -0.00851165645469;
        ABYule[7] = 9.47693607801280;
        ABYule[8] = -0.00834990904936;
        ABYule[9] = -8.81498681370155;
        ABYule[10] = 0.02245293253339;
        ABYule[11] = 6.85401540936998;
        ABYule[12] = -0.02596338512915;
        ABYule[13] = -4.39470996079559;
        ABYule[14] = 0.01624864962975;
        ABYule[15] = 2.19611684890774;
        ABYule[16] = -0.00240879051584;
        ABYule[17] = -0.75104302451432;
        ABYule[18] = 0.00674613682247;
        ABYule[19] = 0.13149317958808;
        ABYule[20] = -0.00187763777362;

        ABButter[0] = 0.98500175787242;
        ABButter[1] = -1.96977855582618;
        ABButter[2] = -1.97000351574484;
        ABButter[3] = 0.97022847566350;
        ABButter[4] = 0.98500175787242;

    }
        ;
        break;
    case 32000: {

        ABYule[0] = 0.15457299681924;
        ABYule[1] = -2.37898834973084;
        ABYule[2] = -0.09331049056315;
        ABYule[3] = 2.84868151156327;
        ABYule[4] = -0.06247880153653;
        ABYule[5] = -2.64577170229825;
        ABYule[6] = 0.02163541888798;
        ABYule[7] = 2.23697657451713;
        ABYule[8] = -0.05588393329856;
        ABYule[9] = -1.67148153367602;
        ABYule[10] = 0.04781476674921;
        ABYule[11] = 1.00595954808547;
        ABYule[12] = 0.00222312597743;
        ABYule[13] = -0.45953458054983;
        ABYule[14] = 0.03174092540049;
        ABYule[15] = 0.16378164858596;
        ABYule[16] = -0.01390589421898;
        ABYule[17] = -0.05032077717131;
        ABYule[18] = 0.00651420667831;
        ABYule[19] = 0.02347897407020;
        ABYule[20] = -0.00881362733839;

        ABButter[0] = 0.97938932735214;
        ABButter[1] = -1.95835380975398;
        ABButter[2] = -1.95877865470428;
        ABButter[3] = 0.95920349965459;
        ABButter[4] = 0.97938932735214;

    }
        ;
        break;
    case 24000: {

        ABYule[0] = 0.30296907319327;
        ABYule[1] = -1.61273165137247;
        ABYule[2] = -0.22613988682123;
        ABYule[3] = 1.07977492259970;
        ABYule[4] = -0.08587323730772;
        ABYule[5] = -0.25656257754070;
        ABYule[6] = 0.03282930172664;
        ABYule[7] = -0.16276719120440;
        ABYule[8] = -0.00915702933434;
        ABYule[9] = -0.22638893773906;
        ABYule[10] = -0.02364141202522;
        ABYule[11] = 0.39120800788284;
        ABYule[12] = -0.00584456039913;
        ABYule[13] = -0.22138138954925;
        ABYule[14] = 0.06276101321749;
        ABYule[15] = 0.04500235387352;
        ABYule[16] = -0.00000828086748;
        ABYule[17] = 0.02005851806501;
        ABYule[18] = 0.00205861885564;
        ABYule[19] = 0.00302439095741;
        ABYule[20] = -0.02950134983287;

        ABButter[0] = 0.97531843204928;
        ABButter[1] = -1.95002759149878;
        ABButter[2] = -1.95063686409857;
        ABButter[3] = 0.95124613669835;
        ABButter[4] = 0.97531843204928;

    }
        ;
        break;
    case 22050: {

        ABYule[0] = 0.33642304856132;
        ABYule[1] = -1.49858979367799;
        ABYule[2] = -0.25572241425570;
        ABYule[3] = 0.87350271418188;
        ABYule[4] = -0.11828570177555;
        ABYule[5] = 0.12205022308084;
        ABYule[6] = 0.11921148675203;
        ABYule[7] = -0.80774944671438;
        ABYule[8] = -0.07834489609479;
        ABYule[9] = 0.47854794562326;
        ABYule[10] = -0.00469977914380;
        ABYule[11] = -0.12453458140019;
        ABYule[12] = -0.00589500224440;
        ABYule[13] = -0.04067510197014;
        ABYule[14] = 0.05724228140351;
        ABYule[15] = 0.08333755284107;
        ABYule[16] = 0.00832043980773;
        ABYule[17] = -0.04237348025746;
        ABYule[18] = -0.01635381384540;
        ABYule[19] = 0.02977207319925;
        ABYule[20] = -0.01760176568150;

        ABButter[0] = 0.97316523498161;
        ABButter[1] = -1.94561023566527;
        ABButter[2] = -1.94633046996323;
        ABButter[3] = 0.94705070426118;
        ABButter[4] = 0.97316523498161;
    }
        ;
        break;

    case 16000: {

        ABYule[0] = 0.44915256608450;
        ABYule[1] = -0.62820619233671;
        ABYule[2] = -0.14351757464547;
        ABYule[3] = 0.29661783706366;
        ABYule[4] = -0.22784394429749;
        ABYule[5] = -0.37256372942400;
        ABYule[6] = -0.01419140100551;
        ABYule[7] = 0.00213767857124;
        ABYule[8] = 0.04078262797139;
        ABYule[9] = -0.42029820170918;
        ABYule[10] = -0.12398163381748;
        ABYule[11] = 0.22199650564824;
        ABYule[12] = 0.04097565135648;
        ABYule[13] = 0.00613424350682;
        ABYule[14] = 0.10478503600251;
        ABYule[15] = 0.06747620744683;
        ABYule[16] = -0.01863887810927;
        ABYule[17] = 0.05784820375801;
        ABYule[18] = -0.03193428438915;
        ABYule[19] = 0.03222754072173;
        ABYule[20] = 0.00541907748707;

        ABButter[0] = 0.96454515552826;
        ABButter[1] = -1.92783286977036;
        ABButter[2] = -1.92909031105652;
        ABButter[3] = 0.93034775234268;
        ABButter[4] = 0.96454515552826;

    }
        ;
        break;
    case 12000: {

        ABYule[0] = 0.56619470757641;
        ABYule[1] = -1.04800335126349;
        ABYule[2] = -0.75464456939302;
        ABYule[3] = 0.29156311971249;
        ABYule[4] = 0.16242137742230;
        ABYule[5] = -0.26806001042947;
        ABYule[6] = 0.16744243493672;
        ABYule[7] = 0.00819999645858;
        ABYule[8] = -0.18901604199609;
        ABYule[9] = 0.45054734505008;
        ABYule[10] = 0.30931782841830;
        ABYule[11] = -0.33032403314006;
        ABYule[12] = -0.27562961986224;
        ABYule[13] = 0.06739368333110;
        ABYule[14] = 0.00647310677246;
        ABYule[15] = -0.04784254229033;
        ABYule[16] = 0.08647503780351;
        ABYule[17] = 0.01639907836189;
        ABYule[18] = -0.03788984554840;
        ABYule[19] = 0.01807364323573;
        ABYule[20] = -0.00588215443421;

        ABButter[0] = 0.96009142950541;
        ABButter[1] = -1.91858953033784;
        ABButter[2] = -1.92018285901082;
        ABButter[3] = 0.92177618768381;
        ABButter[4] = 0.96009142950541;

    }
        ;
        break;
    case 11025: {

        ABYule[0] = 0.58100494960553;
        ABYule[1] = -0.51035327095184;
        ABYule[2] = -0.53174909058578;
        ABYule[3] = -0.31863563325245;
        ABYule[4] = -0.14289799034253;
        ABYule[5] = -0.20256413484477;
        ABYule[6] = 0.17520704835522;
        ABYule[7] = 0.14728154134330;
        ABYule[8] = 0.02377945217615;
        ABYule[9] = 0.38952639978999;
        ABYule[10] = 0.15558449135573;
        ABYule[11] = -0.23313271880868;
        ABYule[12] = -0.25344790059353;
        ABYule[13] = -0.05246019024463;
        ABYule[14] = 0.01628462406333;
        ABYule[15] = -0.02505961724053;
        ABYule[16] = 0.06920467763959;
        ABYule[17] = 0.02442357316099;
        ABYule[18] = -0.03721611395801;
        ABYule[19] = 0.01818801111503;
        ABYule[20] = -0.00749618797172;

        ABButter[0] = 0.95856916599601;
        ABButter[1] = -1.91542108074780;
        ABButter[2] = -1.91713833199203;
        ABButter[3] = 0.91885558323625;
        ABButter[4] = 0.95856916599601;

    }
        ;
        break;
    case 8000: {

        ABYule[0] = 0.53648789255105;
        ABYule[1] = -0.25049871956020;
        ABYule[2] = -0.42163034350696;
        ABYule[3] = -0.43193942311114;
        ABYule[4] = -0.00275953611929;
        ABYule[5] = -0.03424681017675;
        ABYule[6] = 0.04267842219415;
        ABYule[7] = -0.04678328784242;
        ABYule[8] = -0.10214864179676;
        ABYule[9] = 0.26408300200955;
        ABYule[10] = 0.14590772289388;
        ABYule[11] = 0.15113130533216;
        ABYule[12] = -0.02459864859345;
        ABYule[13] = -0.17556493366449;
        ABYule[14] = -0.11202315195388;
        ABYule[15] = -0.18823009262115;
        ABYule[16] = -0.04060034127000;
        ABYule[17] = 0.05477720428674;
        ABYule[18] = 0.04788665548180;
        ABYule[19] = 0.04704409688120;
        ABYule[20] = -0.02217936801134;

        ABButter[0] = 0.94597685600279;
        ABButter[1] = -1.88903307939452;
        ABButter[2] = -1.89195371200558;
        ABButter[3] = 0.89487434461664;
        ABButter[4] = 0.94597685600279;
            }; break;
            default:    return false;
        }

            sampleWindow = (int) ceil (samplefreq * 0.050);

            lsum         = 0.;
            rsum         = 0.;
            totsamp      = 0;

            memset ( A, 0, sizeof(A) );

            return true;
}

float
ReplayGain::analyzeResult ( unsigned int* Array, size_t len ){

    unsigned int  elems;
    int   upper;
    size_t    i;

    elems = 0;
    for ( i = 0; i < len; i++ )
        elems += Array[i];

    if ( elems == 0 )
        return 0;

    upper = (signed int) ceil (elems * (1. - 0.95));
    for ( i = len; i-- > 0; ) {
        if ( (upper -= Array[i]) <= 0 )
            break;
    }

    return (float) ((float)64.82 - (float)i / (float)100);
}
