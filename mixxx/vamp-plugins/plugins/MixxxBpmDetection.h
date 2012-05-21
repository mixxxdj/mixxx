#ifndef _MIXXXBPMDETECTION_H_
#define _MIXXXBPMDETECTION_H_

#include <vamp-sdk/Plugin.h>
#include "BPMDetect.h"

using std::string;

class MixxxBpmDetection : public Vamp::Plugin {
  public:
    MixxxBpmDetection(float inputSampleRate);
    virtual ~MixxxBpmDetection();

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
    float correctBPM(float BPM, float min, float max, bool aboveRange);

    soundtouch::BPMDetect *m_pDetector;
    int m_iSampleRate,
        m_iBlockSize;
    float m_fPhase, m_fNumCycles, m_fMinBpm, m_fMaxBpm;
    bool m_bProcessEntireSong, m_bAllowAboveRange;
};

#endif
