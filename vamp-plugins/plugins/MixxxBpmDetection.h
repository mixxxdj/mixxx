#ifndef _MIXXXBPMDETECTION_H_
#define _MIXXXBPMDETECTION_H_

#include <vamp-sdk/Plugin.h>
#include "BPMDetect.h"

class MixxxBpmDetection : public Vamp::Plugin {
  public:
    MixxxBpmDetection(float inputSampleRate);
    virtual ~MixxxBpmDetection();

    std::string getIdentifier() const;
    std::string getName() const;
    std::string getDescription() const;
    std::string getMaker() const;
    int getPluginVersion() const;
    std::string getCopyright() const;

    InputDomain getInputDomain() const;
    size_t getPreferredBlockSize() const;
    size_t getPreferredStepSize() const;
    size_t getMinChannelCount() const;
    size_t getMaxChannelCount() const;

    ParameterList getParameterDescriptors() const;
    float getParameter(std::string identifier) const;
    void setParameter(std::string identifier, float value);

    ProgramList getPrograms() const;
    std::string getCurrentProgram() const;
    void selectProgram(std::string name);

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
