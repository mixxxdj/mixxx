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


#include <TempogramPlugin.h

using Vamp::FFT;
using Vamp::RealTime;
using namespace std;

TempogramPlugin::TempogramPlugin(float inputSampleRate) :
    Plugin(inputSampleRate),
    m_inputBlockSize(0), //host parameter
    m_inputStepSize(0), //host parameter
    m_noveltyCurveMinDB(-74), //parameter
    m_noveltyCurveMinV(0), //set in initialise()
    m_noveltyCurveCompressionConstant(1000), //parameter
    m_tempogramLog2WindowLength(10), //parameter
    m_tempogramWindowLength(0), //set in initialise()
    m_tempogramLog2FftLength(m_tempogramLog2WindowLength), //parameter
    m_tempogramFftLength(0), //set in initialise()
    m_tempogramLog2HopSize(6), //parameter
    m_tempogramHopSize(0), //set in initialise()
    m_tempogramMinBPM(30), //parameter
    m_tempogramMaxBPM(480), //parameter
    m_tempogramMinBin(0), //set in initialise()
    m_tempogramMaxBin(0), //set in initialise()
    m_tempogramMinLag(0), //set in initialise()
    m_tempogramMaxLag(0), //set in initialise()
    m_cyclicTempogramMinBPM(30), //reset in initialise()
    m_cyclicTempogramNumberOfOctaves(0), //set in initialise()
    m_cyclicTempogramOctaveDivider(30), //parameter
    m_cyclicTempogramReferenceBPM(60) //parameter

    // Also be sure to set your plugin parameters (presumably stored
    // in member variables) to their default values here -- the host
    // will not do that for you
{
}

TempogramPlugin::~TempogramPlugin()
{
    //delete stuff
}

string
TempogramPlugin::getIdentifier() const
{
    return "tempogram";
}

string
TempogramPlugin::getName() const
{
    return "Tempogram";
}

string
TempogramPlugin::getDescription() const
{
    return "Tempogram and Cyclic Tempogram as described by Peter Grosche and Meinard Müller";
}

string
TempogramPlugin::getMaker() const
{
    return "Carl Bussey";
}

int
TempogramPlugin::getPluginVersion() const
{
    return 1;
}

string
TempogramPlugin::getCopyright() const
{
    return "Copyright 2014 Queen Mary University of London. GPL licence.";
}

TempogramPlugin::InputDomain
TempogramPlugin::getInputDomain() const
{
    return FrequencyDomain;
}

size_t
TempogramPlugin::getPreferredBlockSize() const
{
    return 2048; // 0 means "I can handle any block size"
}

size_t 
TempogramPlugin::getPreferredStepSize() const
{
    return 1024; // 0 means "anything sensible"; in practice this
              // means the same as the block size for TimeDomain
              // plugins, or half of it for FrequencyDomain plugins
}

size_t
TempogramPlugin::getMinChannelCount() const
{
    return 1;
}

size_t
TempogramPlugin::getMaxChannelCount() const
{
    return 1;
}

TempogramPlugin::ParameterList
TempogramPlugin::getParameterDescriptors() const
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

    ParameterDescriptor d1;
    d1.identifier = "C";
    d1.name = "Novelty Curve Spectrogram Compression Constant";
    d1.description = "Spectrogram compression constant, C, used when retrieving the novelty curve from the audio.";
    d1.unit = "";
    d1.minValue = 2;
    d1.maxValue = 10000;
    d1.defaultValue = 1000;
    d1.isQuantized = false;
    list.push_back(d1);
    
    ParameterDescriptor d2;
    d2.identifier = "minDB";
    d2.name = "Novelty Curve Minimum DB";
    d2.description = "Spectrogram minimum DB used when removing unwanted peaks in the Spectrogram when retrieving the novelty curve from the audio.";
    d2.unit = "";
    d2.minValue = -100;
    d2.maxValue = -50;
    d2.defaultValue = -74;
    d2.isQuantized = false;
    list.push_back(d2);

    ParameterDescriptor d3;
    d3.identifier = "log2TN";
    d3.name = "Tempogram Window Length";
    d3.description = "FFT window length when analysing the novelty curve and extracting the tempogram time-frequency function.";
    d3.unit = "";
    d3.minValue = 7;
    d3.maxValue = 12;
    d3.defaultValue = 10;
    d3.isQuantized = true;
    d3.quantizeStep = 1;
    for (int i = d3.minValue; i <= d3.maxValue; i++){
        d3.valueNames.push_back(floatToString(pow((float)2,(float)i)));
    }
    list.push_back(d3);
    
    ParameterDescriptor d4;
    d4.identifier = "log2HopSize";
    d4.name = "Tempogram Hopsize";
    d4.description = "FFT hopsize when analysing the novelty curve and extracting the tempogram time-frequency function.";
    d4.unit = "";
    d4.minValue = 6;
    d4.maxValue = 12;
    d4.defaultValue = 6;
    d4.isQuantized = true;
    d4.quantizeStep = 1;
    for (int i = d4.minValue; i <= d4.maxValue; i++){
        d4.valueNames.push_back(floatToString(pow((float)2,(float)i)));
    }
    list.push_back(d4);
    
    ParameterDescriptor d5;
    d5.identifier = "log2FftLength";
    d5.name = "Tempogram FFT Length";
    d5.description = "FFT length when analysing the novelty curve and extracting the tempogram time-frequency function. This parameter determines the amount of zero padding.";
    d5.unit = "";
    d5.minValue = 6;
    d5.maxValue = 12;
    d5.defaultValue = 10;
    d5.isQuantized = true;
    d5.quantizeStep = 1;
    for (int i = d5.minValue; i <= d5.maxValue; i++){
        d5.valueNames.push_back(floatToString(pow((float)2,(float)i)));
    }
    list.push_back(d5);
    
    ParameterDescriptor d6;
    d6.identifier = "minBPM";
    d6.name = "(Cyclic) Tempogram Minimum BPM";
    d6.description = "The minimum BPM of the tempogram output bins.";
    d6.unit = "";
    d6.minValue = 0;
    d6.maxValue = 2000;
    d6.defaultValue = 30;
    d6.isQuantized = true;
    d6.quantizeStep = 5;
    list.push_back(d6);
    
    ParameterDescriptor d7;
    d7.identifier = "maxBPM";
    d7.name = "(Cyclic) Tempogram Maximum BPM";
    d7.description = "The maximum BPM of the tempogram output bins.";
    d7.unit = "";
    d7.minValue = 30;
    d7.maxValue = 2000;
    d7.defaultValue = 480;
    d7.isQuantized = true;
    d7.quantizeStep = 5;
    list.push_back(d7);
    
    ParameterDescriptor d8;
    d8.identifier = "octDiv";
    d8.name = "Cyclic Tempogram Octave Divider";
    d8.description = "The number bins within each octave.";
    d8.unit = "";
    d8.minValue = 5;
    d8.maxValue = 60;
    d8.defaultValue = 30;
    d8.isQuantized = true;
    d8.quantizeStep = 1;
    list.push_back(d8);
    
    ParameterDescriptor d9;
    d9.identifier = "refBPM";
    d9.name = "Cyclic Tempogram Reference Tempo";
    d9.description = "The reference tempo used when calculating the Cyclic Tempogram parameter \'s\'.";
    d9.unit = "";
    d9.minValue = 30;
    d9.maxValue = 120;
    d9.defaultValue = 60;
    d9.isQuantized = true;
    d9.quantizeStep = 1;
    list.push_back(d9);

    return list;
}

float
TempogramPlugin::getParameter(string identifier) const
{
    if (identifier == "C") {
        return m_noveltyCurveCompressionConstant; // return the ACTUAL current value of your parameter here!
    }
    else if (identifier == "minDB"){
        return m_noveltyCurveMinDB;
    }
    else if (identifier == "log2TN"){
        return m_tempogramLog2WindowLength;
    }
    else if (identifier == "log2HopSize"){
        return m_tempogramLog2HopSize;
    }
    else if (identifier == "log2FftLength"){
        return m_tempogramLog2FftLength;
    }
    else if (identifier == "minBPM") {
        return m_tempogramMinBPM;
    }
    else if (identifier == "maxBPM"){
        return m_tempogramMaxBPM;
    }
    else if (identifier == "octDiv"){
        return m_cyclicTempogramOctaveDivider;
    }
    else if (identifier == "refBPM"){
        return m_cyclicTempogramReferenceBPM;
    }
    
    return 0;
}

void
TempogramPlugin::setParameter(string identifier, float value) 
{
    
    if (identifier == "C") {
        m_noveltyCurveCompressionConstant = value; // set the actual value of your parameter
    }
    else if (identifier == "minDB"){
        m_noveltyCurveMinDB = value;
    }
    else if (identifier == "log2TN") {
        m_tempogramLog2WindowLength = value;
    }
    else if (identifier == "log2HopSize"){
        m_tempogramLog2HopSize = value;
    }
    else if (identifier == "log2FftLength"){
        m_tempogramLog2FftLength = value;
    }
    else if (identifier == "minBPM") {
        m_tempogramMinBPM = value;
    }
    else if (identifier == "maxBPM"){
        m_tempogramMaxBPM = value;
    }
    else if (identifier == "octDiv"){
        m_cyclicTempogramOctaveDivider = value;
    }
    else if (identifier == "refBPM"){
        m_cyclicTempogramReferenceBPM = value;
    }
    
}

TempogramPlugin::ProgramList
TempogramPlugin::getPrograms() const
{
    ProgramList list;

    // If you have no programs, return an empty list (or simply don't
    // implement this function or getCurrentProgram/selectProgram)

    return list;
}

string
TempogramPlugin::getCurrentProgram() const
{
    return ""; // no programs
}

void
TempogramPlugin::selectProgram(string name)
{
}

TempogramPlugin::OutputList
TempogramPlugin::getOutputDescriptors() const
{
    OutputList list;

    // See OutputDescriptor documentation for the possibilities here.
    // Every plugin must have at least one output.
    
    float d_sampleRate;
    float tempogramInputSampleRate = (float)m_inputSampleRate/m_inputStepSize;
    OutputDescriptor d1;
    d1.identifier = "cyclicTempogram";
    d1.name = "Cyclic Tempogram";
    d1.description = "Cyclic tempogram calculated by \"octave folding\" the DFT tempogram";
    d1.unit = "";
    d1.hasFixedBinCount = true;
    d1.binCount = m_cyclicTempogramOctaveDivider > 0 && !isnan(m_cyclicTempogramOctaveDivider) ? m_cyclicTempogramOctaveDivider : 0;
    d1.hasKnownExtents = false;
    d1.isQuantized = false;
    d1.sampleType = OutputDescriptor::FixedSampleRate;
    d_sampleRate = tempogramInputSampleRate/m_tempogramHopSize;
    d1.sampleRate = d_sampleRate > 0.0 && !isnan(d_sampleRate) ? d_sampleRate : 0;
    vector< vector <unsigned int> > logBins = calculateTempogramNearestNeighbourLogBins();
    if (!logBins.empty()){
        float scale = pow(2,ceil(log2(60/binToBPM(logBins[0][0]))));
        for(int i = 0; i < m_cyclicTempogramOctaveDivider; i++){
            float s = scale*binToBPM(logBins[0][i])/m_cyclicTempogramReferenceBPM;
            d1.binNames.push_back(floatToString(s));
            //cerr << m_cyclicTempogramOctaveDivider  << " " << s << endl;
        }
    }
    d1.hasDuration = false;
    list.push_back(d1);
    
    OutputDescriptor d2;
    d2.identifier = "tempogramDFT";
    d2.name = "Tempogram via DFT";
    d2.description = "Tempogram calculated using Discrete Fourier Transform method";
    d2.unit = ""; // unit of bin contents, not of "bin label", so not bpm
    d2.hasFixedBinCount = true;
    d2.binCount = m_tempogramMaxBin - m_tempogramMinBin + 1;
    d2.hasKnownExtents = false;
    d2.isQuantized = false;
    d2.sampleType = OutputDescriptor::FixedSampleRate;
    d_sampleRate = tempogramInputSampleRate/m_tempogramHopSize;
    d2.sampleRate = d_sampleRate > 0.0 && !isnan(d_sampleRate) ? d_sampleRate : 0.0;
    for(int i = m_tempogramMinBin; i <= (int)m_tempogramMaxBin; i++){
        float w = ((float)i/m_tempogramFftLength)*(tempogramInputSampleRate);
        d2.binNames.push_back(floatToString(w*60));
    }
    d2.hasDuration = false;
    list.push_back(d2);
    
    OutputDescriptor d3;
    d3.identifier = "tempogramACT";
    d3.name = "Tempogram via ACT";
    d3.description = "Tempogram calculated using autocorrelation method";
    d3.unit = ""; // unit of bin contents, not of "bin label", so not bpm
    d3.hasFixedBinCount = true;
    d3.binCount = m_tempogramMaxLag - m_tempogramMinLag + 1;
    d3.hasKnownExtents = false;
    d3.isQuantized = false;
    d3.sampleType = OutputDescriptor::FixedSampleRate;
    d_sampleRate = tempogramInputSampleRate/m_tempogramHopSize;
    d3.sampleRate = d_sampleRate > 0.0 && !isnan(d_sampleRate) ? d_sampleRate : 0.0;
    for(int lag = m_tempogramMaxLag; lag >= (int)m_tempogramMinLag; lag--){
        d3.binNames.push_back(floatToString(60/(m_inputStepSize*(lag/m_inputSampleRate))));
    }
    d3.hasDuration = false;
    list.push_back(d3);
    
    OutputDescriptor d4;
    d4.identifier = "nc";
    d4.name = "Novelty Curve";
    d4.description = "Novelty curve underlying the tempogram calculations";
    d4.unit = "";
    d4.hasFixedBinCount = true;
    d4.binCount = 1;
    d4.hasKnownExtents = false;
    d4.isQuantized = false;
    d4.sampleType = OutputDescriptor::FixedSampleRate;
    d_sampleRate = tempogramInputSampleRate;
    d4.sampleRate = d_sampleRate > 0 && !isnan(d_sampleRate) ? d_sampleRate : 0;
    d4.hasDuration = false;
    list.push_back(d4);
    
    return list;
}

bool
TempogramPlugin::initialise(size_t channels, size_t stepSize, size_t blockSize)
{
    if (channels < getMinChannelCount() ||
	channels > getMaxChannelCount()) return false;
    
    // Real initialisation work goes here!
    m_inputBlockSize = blockSize;
    m_inputStepSize = stepSize;
    
    //m_spectrogram = Spectrogram(m_inputBlockSize/2 + 1);
    if (!handleParameterValues()) return false;
    //cout << m_cyclicTempogramOctaveDivider << endl;
    
    return true;
}

void
TempogramPlugin::reset()
{
    // Clear buffers, reset stored values, etc
    m_spectrogram.clear();
    handleParameterValues();
}

TempogramPlugin::FeatureSet
TempogramPlugin::process(const float *const *inputBuffers, Vamp::RealTime timestamp)
{
    int n = m_inputBlockSize/2 + 1;
    const float *in = inputBuffers[0];

    //calculate magnitude of FrequencyDomain input
    vector<float> fftCoefficients;
    for (int i = 0; i < n; i++){
        float magnitude = sqrt(in[2*i] * in[2*i] + in[2*i + 1] * in[2*i + 1]);
        magnitude = magnitude > m_noveltyCurveMinV ? magnitude : m_noveltyCurveMinV;
        fftCoefficients.push_back(magnitude);
    }
    m_spectrogram.push_back(fftCoefficients);
    //m_spectrogram.push_back(fftCoefficients);
    
    return FeatureSet();
}

TempogramPlugin::FeatureSet
TempogramPlugin::getRemainingFeatures()
{
    
    float * hannWindow = new float[m_tempogramWindowLength];
    for (int i = 0; i < (int)m_tempogramWindowLength; i++){
        hannWindow[i] = 0.0;
    }
    
    FeatureSet featureSet;
    
    //initialise novelty curve processor
    int numberOfBlocks = m_spectrogram.size();

    NoveltyCurveProcessor nc(m_inputSampleRate, m_inputBlockSize, m_noveltyCurveCompressionConstant);
    vector<float> noveltyCurve = nc.spectrogramToNoveltyCurve(m_spectrogram); //calculate novelty curvefrom magnitude data
    
    //push novelty curve data to featureset 1 and set timestamps
    for (int i = 0; i < numberOfBlocks; i++){
        Feature noveltyCurveFeature;
        noveltyCurveFeature.values.push_back(noveltyCurve[i]);
        noveltyCurveFeature.hasTimestamp = false;
        featureSet[3].push_back(noveltyCurveFeature);
        assert(!isnan(noveltyCurveFeature.values.back()));
    }
    
    //window function for spectrogram
    WindowFunction::hanning(hannWindow, m_tempogramWindowLength);
    
    //initialise spectrogram processor
    SpectrogramProcessor spectrogramProcessor(m_tempogramWindowLength, m_tempogramFftLength, m_tempogramHopSize);
    //compute spectrogram from novelty curve data (i.e., tempogram)
    Tempogram tempogramDFT = spectrogramProcessor.process(&noveltyCurve[0], numberOfBlocks, hannWindow);
    delete []hannWindow;
    hannWindow = 0;
    
    int tempogramLength = tempogramDFT.size();
    
    //push tempogram data to featureset 0 and set timestamps.
    for (int block = 0; block < tempogramLength; block++){
        Feature tempogramDFTFeature;
        
        assert(tempogramDFT[block].size() == (m_tempogramFftLength/2 + 1));
        for(int k = m_tempogramMinBin; k <= (int)m_tempogramMaxBin; k++){
            tempogramDFTFeature.values.push_back(tempogramDFT[block][k]);
        }
        tempogramDFTFeature.hasTimestamp = false;
        featureSet[1].push_back(tempogramDFTFeature);
    }
    
    AutocorrelationProcessor autocorrelationProcessor(m_tempogramWindowLength, m_tempogramHopSize);
    Tempogram tempogramACT = autocorrelationProcessor.process(&noveltyCurve[0], numberOfBlocks);
    
    for (int block = 0; block < tempogramLength; block++){
        Feature tempogramACTFeature;

        for(int k = m_tempogramMaxLag; k >= (int)m_tempogramMinLag; k--){
            tempogramACTFeature.values.push_back(tempogramACT[block][k]);
        }
        tempogramACTFeature.hasTimestamp = false;
        featureSet[2].push_back(tempogramACTFeature);
    }
    
    //Calculate cyclic tempogram
    vector< vector<unsigned int> > logBins = calculateTempogramNearestNeighbourLogBins();
    
    //assert((int)logBins.size() == m_cyclicTempogramOctaveDivider*m_cyclicTempogramNumberOfOctaves);
    for (int block = 0; block < tempogramLength; block++){
        Feature cyclicTempogramFeature;
        
        for (int i = 0; i < m_cyclicTempogramOctaveDivider; i++){
            float sum = 0;
            
            for (int j = 0; j < m_cyclicTempogramNumberOfOctaves; j++){
                sum += tempogramDFT[block][logBins[j][i]];
            }
            cyclicTempogramFeature.values.push_back(sum/m_cyclicTempogramNumberOfOctaves);
            assert(!isnan(cyclicTempogramFeature.values.back()));
        }

        cyclicTempogramFeature.hasTimestamp = false;
        featureSet[0].push_back(cyclicTempogramFeature);
    }
    
    return featureSet;
}

vector< vector<unsigned int> > TempogramPlugin::calculateTempogramNearestNeighbourLogBins() const
{
    vector< vector<unsigned int> > logBins;
    
    for (int octave = 0; octave < (int)m_cyclicTempogramNumberOfOctaves; octave++){
        vector<unsigned int> octaveBins;

        for (int bin = 0; bin < (int)m_cyclicTempogramOctaveDivider; bin++){
            float bpm = m_cyclicTempogramMinBPM*pow(2.0f, octave+(float)bin/m_cyclicTempogramOctaveDivider);
            octaveBins.push_back(bpmToBin(bpm));
        }
        logBins.push_back(octaveBins);
    }
    
    return logBins;
}

unsigned int TempogramPlugin::bpmToBin(const float &bpm) const
{
    float w = (float)bpm/60;
    float sampleRate = m_inputSampleRate/m_inputStepSize;
    int bin = floor((float)m_tempogramFftLength*w/sampleRate + 0.5);
    
    if(bin < 0) bin = 0;
    else if(bin > m_tempogramFftLength/2.0f) bin = m_tempogramFftLength/2.0f;
    
    return bin;
}

float TempogramPlugin::binToBPM(const int &bin) const
{
    float sampleRate = m_inputSampleRate/m_inputStepSize;
    
    return (bin*sampleRate/m_tempogramFftLength)*60;
}

bool TempogramPlugin::handleParameterValues(){
    
    if (m_tempogramLog2HopSize <= 0) {
	cerr << "Tempogram log2 hop size " << m_tempogramLog2HopSize
	     << " <= 0, failing initialise" << endl;
	return false;
    }
    if (m_tempogramLog2FftLength <= 0) {
	cerr << "Tempogram log2 fft length " << m_tempogramLog2FftLength
	     << " <= 0, failing initialise" << endl;
	return false;
    }
    
    if (m_tempogramMinBPM < 1) {
	m_tempogramMinBPM = 1;
    }
    if (m_tempogramMinBPM >= m_tempogramMaxBPM){
        m_tempogramMinBPM = 30;
        m_tempogramMaxBPM = 480;
    }
    
    m_noveltyCurveMinV = pow(10,(float)m_noveltyCurveMinDB/20);
    
    m_tempogramWindowLength = pow(2,m_tempogramLog2WindowLength);
    m_tempogramHopSize = pow(2,m_tempogramLog2HopSize);
    m_tempogramFftLength = pow(2,m_tempogramLog2FftLength);
    
    if (m_tempogramFftLength < m_tempogramWindowLength){
        m_tempogramFftLength = m_tempogramWindowLength;
    }
    
    float tempogramInputSampleRate = (float)m_inputSampleRate/m_inputStepSize;
    m_tempogramMinBin = (max((int)floor(((m_tempogramMinBPM/60)/tempogramInputSampleRate)*m_tempogramFftLength), 0));
    m_tempogramMaxBin = (min((int)ceil(((m_tempogramMaxBPM/60)/tempogramInputSampleRate)*m_tempogramFftLength), (int)(m_tempogramFftLength/2)));

    if (m_tempogramMaxBin < m_tempogramMinBin) {
	cerr << "At audio sample rate " << m_inputSampleRate
	     << ", tempogram sample rate " << tempogramInputSampleRate
	     << " with bpm range " << m_tempogramMinBPM << " -> "
	     << m_tempogramMaxBPM << ", min bin = " << m_tempogramMinBin 
	     << " > max bin " << m_tempogramMaxBin
	     << ": can't proceed, failing initialise" << endl;
	return false;
    }
    
    m_tempogramMinLag = max((int)ceil((60/(m_inputStepSize * m_tempogramMaxBPM))*m_inputSampleRate), 0);
    m_tempogramMaxLag = min((int)floor((60/(m_inputStepSize * m_tempogramMinBPM))*m_inputSampleRate), (int)m_tempogramWindowLength-1);

    if (m_tempogramMaxLag < m_tempogramMinLag) {
	cerr << "At audio sample rate " << m_inputSampleRate
	     << ", tempogram sample rate " << tempogramInputSampleRate
	     << ", window length " << m_tempogramWindowLength
	     << " with bpm range " << m_tempogramMinBPM << " -> "
	     << m_tempogramMaxBPM << ", min lag = " << m_tempogramMinLag 
	     << " > max lag " << m_tempogramMaxLag
	     << ": can't proceed, failing initialise" << endl;
	return false;
    }
    
    m_cyclicTempogramMinBPM = max(binToBPM(m_tempogramMinBin), m_tempogramMinBPM);
    float cyclicTempogramMaxBPM = min(binToBPM(m_tempogramMaxBin), m_tempogramMaxBPM);

    m_cyclicTempogramNumberOfOctaves = floor(log2(cyclicTempogramMaxBPM/m_cyclicTempogramMinBPM));

    if (m_cyclicTempogramNumberOfOctaves < 1) {
	cerr << "At audio sample rate " << m_inputSampleRate
	     << ", tempogram sample rate " << tempogramInputSampleRate
	     << " with bpm range " << m_tempogramMinBPM << " -> "
	     << m_tempogramMaxBPM << ", cyclic tempogram min bpm = "
	     << m_cyclicTempogramMinBPM << " and max bpm = "
	     << cyclicTempogramMaxBPM << " giving number of octaves = "
	     << m_cyclicTempogramNumberOfOctaves
	     << ": can't proceed, failing initialise" << endl;
	return false;
    }
    
    return true;
}

string TempogramPlugin::floatToString(float value) const
{
    ostringstream ss;
    
    if(!(ss << value)) throw runtime_error("TempogramPlugin::floatToString(): invalid conversion from float to string");
    return ss.str();
}
*/