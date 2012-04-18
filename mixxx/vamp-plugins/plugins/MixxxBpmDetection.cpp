#include "MixxxBpmDetection.h"

MixxxBpmDetection::MixxxBpmDetection(float inputSampleRate):
        Vamp::Plugin(inputSampleRate),
        m_pDetector(NULL),
        m_iSampleRate(inputSampleRate),
        m_iBlockSize(4096),
        m_fNumCycles(0),
        m_fMinBpm(50),
        m_fMaxBpm(150),
        m_bAllowAboveRange(0) {
    m_fPhase=0;
}

MixxxBpmDetection::~MixxxBpmDetection() {
}

string MixxxBpmDetection::getIdentifier() const {
    return "mixxxbpmdetection";
}

string MixxxBpmDetection::getName() const {
    return "SoundTouch BPM Detector (Legacy)";
}

string MixxxBpmDetection::getDescription() const {
    // Return something helpful here!
    return "Port of Mixxx BPM Analyser";
}

string MixxxBpmDetection::getMaker() const {
    return "Olli Parviainen";
}

int MixxxBpmDetection::getPluginVersion() const {
    // Increment this each time you release a version that behaves
    // differently from the previous one
    return 1;
}

string MixxxBpmDetection::getCopyright() const {
    // This function is not ideally named.  It does not necessarily
    // need to say who made the plugin -- getMaker does that -- but it
    // should indicate the terms under which it is distributed.  For
    // example, "Copyright (year). All Rights Reserved", or "GPL"
    return "GPL";
}

MixxxBpmDetection::InputDomain
MixxxBpmDetection::getInputDomain() const {
    return TimeDomain;
}

size_t MixxxBpmDetection::getPreferredBlockSize() const {
    return 4096; // 0 means "I can handle any block size"
}

size_t MixxxBpmDetection::getPreferredStepSize() const {
    return 0; // 0 means "anything sensible"; in practice this
    // means the same as the block size for TimeDomain
    // plugins, or half of it for FrequencyDomain plugins
}

size_t MixxxBpmDetection::getMinChannelCount() const {
    return 1;
}

size_t MixxxBpmDetection::getMaxChannelCount() const {
    return 1;
}

MixxxBpmDetection::ParameterList MixxxBpmDetection::getParameterDescriptors() const {
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

    ParameterDescriptor d;
    d.identifier = "minbpm";
    d.name = "BPM min";
    d.description = "Minimum detected BPM";
    d.unit = "bpm";
    d.minValue = 0;
    d.maxValue = 220;
    d.defaultValue = 50;
    d.isQuantized = true;
    d.quantizeStep = 1.0f;
    list.push_back(d);

    d.identifier = "maxbpm";
    d.name = "BPM max";
    d.description = "Maximum BPM";
    d.unit = "bpm";
    d.minValue = 0;
    d.maxValue = 220;
    d.defaultValue = 150;
    d.isQuantized = true;
    d.quantizeStep = 1.0f;
    list.push_back(d);

    d.identifier = "bpmaboverange";
    d.name = "BPM above range";
    d.description = "Allow BPM above range";
    d.unit = "";
    d.minValue = 0;
    d.maxValue = 1.0f;
    d.defaultValue = 0;
    d.isQuantized = true;
    d.quantizeStep = 1.0f;
    list.push_back(d);

    d.identifier = "phase";
    d.name = "phase";
    d.description = "Phase ( expressed as a fraction of beat length )";
    d.unit = "";
    d.minValue = 0;
    d.maxValue = 1.0f;
    d.defaultValue = 0;
    d.isQuantized = false;
    list.push_back(d);

    return list;
}

float MixxxBpmDetection::getParameter(string identifier) const {
    if (identifier == "minbpm") {
        return m_fMinBpm; // return the ACTUAL current value of your parameter here!
    }
    if (identifier == "maxbpm") {
        return m_fMaxBpm;
    }
    if (identifier == "bpmaboverange") {
        return m_bAllowAboveRange ? 1.0 : 0.0;
    }

    if (identifier == "phase") {
        return m_fPhase;
    }
    return 0;
}

void MixxxBpmDetection::setParameter(string identifier, float value) {
    if (identifier == "minbpm") {
        m_fMinBpm = value; // return the ACTUAL current value of your parameter here!
    }
    if (identifier == "maxbpm") {
        m_fMaxBpm = value;
    }
    if (identifier == "bpmaboverange") {
        m_bAllowAboveRange = (value > 0.5);
    }
    if (identifier == "phase") {
        m_fPhase = value;
    }
}

MixxxBpmDetection::ProgramList MixxxBpmDetection::getPrograms() const {
    ProgramList list;

    // If you have no programs, return an empty list (or simply don't
    // implement this function or getCurrentProgram/selectProgram)

    return list;
}

string MixxxBpmDetection::getCurrentProgram() const {
    return ""; // no programs
}

void MixxxBpmDetection::selectProgram(string name) {
}

MixxxBpmDetection::OutputList MixxxBpmDetection::getOutputDescriptors() const {
    OutputList list;

    // See OutputDescriptor documentation for the possibilities here.
    // Every plugin must have at least one output.

    OutputDescriptor d;
    d.identifier = "Beat";
    d.name = "Beat location";
    d.description = "Estimated Beat location";
    d.unit = "";
    d.hasFixedBinCount = true;
    d.binCount = 0;
    d.sampleType = OutputDescriptor::FixedSampleRate;
    d.hasDuration = false;
    list.push_back(d);

    return list;
}

bool MixxxBpmDetection::initialise(size_t channels, size_t stepSize, size_t blockSize) {
    if (channels < getMinChannelCount() ||
            channels > getMaxChannelCount()) return false;

    m_pDetector = new soundtouch::BPMDetect(channels, m_iSampleRate);
    m_iBlockSize = blockSize;
    return true;
}

void MixxxBpmDetection::reset() {
    m_fNumCycles = 0;
    // Clear buffers, reset stored values, etc
}

MixxxBpmDetection::FeatureSet MixxxBpmDetection::process(const float *const *inputBuffers, Vamp::RealTime timestamp) {
    if(m_pDetector != NULL) {
        m_pDetector->inputSamples(inputBuffers[0], m_iBlockSize);
    }
    m_fNumCycles++;
    return FeatureSet();
}

MixxxBpmDetection::FeatureSet MixxxBpmDetection::getRemainingFeatures() {
    FeatureSet returnfs;
    if(m_pDetector != NULL) {
        float bpm = m_pDetector->getBpm();
        if(bpm != 0) {
            // Shift it by 2's until it is in the desired range
            float newbpm = correctBPM(bpm, m_fMinBpm, m_fMaxBpm,
                                      m_bAllowAboveRange);
            float beatlen = (60.0 * m_iSampleRate / newbpm);
            float beatpos = m_fPhase * beatlen;
            while (beatpos<m_iBlockSize*m_fNumCycles){
                Feature f;
                f.hasTimestamp = true;
                f.timestamp = Vamp::RealTime::frame2RealTime
                        (beatpos, m_iSampleRate);
                f.label = "Beat";
                returnfs[0].push_back(f);
                beatpos += beatlen;
            }
        }
        delete m_pDetector;
    }
    return returnfs;
}

float MixxxBpmDetection::correctBPM(float BPM, float min, float max, bool aboveRange) {
    //qDebug() << "BPM range is" << min << "to" << max;
    //if ( BPM == 0 ) return BPM;

    if (!aboveRange) {
        if( BPM*2 < max ) BPM *= 2;
        while ( BPM > max ) BPM /= 2;
    }
    while ( BPM < min ) BPM *= 2;

    return BPM;
}
